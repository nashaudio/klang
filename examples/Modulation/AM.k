
#include <klang.h>
using namespace klang::optimised;

struct AM : Synth {

	struct AMNote : public Note {
		Sine carrier;
		Sine modulator;
		ADSR adsr;

		event on(Pitch pitch, Amplitude velocity) {
			carrier(pitch -> Frequency, 0);
			adsr(0, .1, 0.1, 0.25);
		}

		event off(Amplitude velocity) {
			adsr.release();
		}

		void process() {
			param mod_rate = controls[0] * carrier.frequency;
			param mod_depth = controls[1];
			
			signal mod = modulator(mod_rate) * mod_depth + (1 - mod_depth);
			
			carrier * mod * adsr >> out;
			if (adsr.finished())
				stop();
		}
	};

	// Initialise plugin (called once at startup)
	AM() {
		controls = {
			Dial("Rate", 0.01, 3, 0.5),
			Dial("Depth", 0, 1, 0.5),
		};

		notes.add<AMNote>(32);
	}
};