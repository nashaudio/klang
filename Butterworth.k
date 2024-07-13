
#include <klang.h>
using namespace klang::optimised;

struct Butterworth : Synth {

	struct ButterworthNote : public Note {
		Sine osc;
		ADSR adsr;

		event on(Pitch pitch, Amplitude velocity) {
			const param f = pitch > Type::Frequency;
			osc(f, 0);
			adsr(0.001, 0, 1, 0.25);
		}

		event off(Amplitude velocity) {
			adsr.release();
		}

		void process() {
			osc* adsr++ >> out;
			if (adsr.finished())
				stop();
		}
	};

	// Initialise plugin (called once at startup)
	Butterworth() {
		controls = {
			// UI controls and parameters
		};

		notes.add<ButterworthNote>(32);
	}
};