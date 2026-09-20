
#include <klang.h>
using namespace klang::optimised;

struct ModalResonator : Synth {

	struct ModalResonatorNote : public Note {
		Modal modal[3];
		ADSR adsr;
		DCF dc;

		event on(Pitch pitch, Amplitude velocity) {
			const param f = pitch -> Frequency;
			modal[0](f * 1, 4);
			modal[1](f * 1.99, 2);
			modal[2](f * 2.01, 2);
			1 >> modal[0];
			1 >> modal[1];
			1 >> modal[2];
			adsr(0.001, 0, 1, 0.5);
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
		notes.add<ModalResonatorNote>(32);
	}
};