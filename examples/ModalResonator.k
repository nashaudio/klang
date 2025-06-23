
#include <klang.h>
using namespace klang::optimised;

struct DCFilter {
    float R = 0.995f; // Set pole close to 1 for deep low cut
    float y1 = 0.0f, x1 = 0.0f;

    float process(float x) {
        float y = x - x1 + R * y1;
        x1 = x;
        y1 = y;
        return y;
    }
};

struct ModalResonator : Synth {

	struct ModalResonatorNote : public Note {
		Modal modal[3];
		ADSR adsr;
		
		HPF dc;

		event on(Pitch pitch, Amplitude velocity) {
			const param f = pitch -> Frequency;
			modal[0](f * 1, 4);
			modal[1](f * 1.99, 2);
			modal[2](f * 2.01, 2);
			1 >> modal[0];
			1 >> modal[1];
			1 >> modal[2];
			adsr(0.001, 0, 1, 0.5);
			dc(50);
		}

		event off(Amplitude velocity) {
			adsr.release();
		}

		void process() {
			(modal[0] + modal[1] + modal[2]) * adsr >> dc >> out;
			if (adsr.finished())
				stop();
		}
	};

	// Initialise plugin (called once at startup)
	ModalResonator() {
		controls = {
			// UI controls and parameters
		};

		notes.add<ModalResonatorNote>(32);
	}
};