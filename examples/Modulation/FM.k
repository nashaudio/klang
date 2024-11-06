
#include <klang.h>
using namespace klang::optimised;

struct FM : Synth {

	struct FMNote : public Note {
		Sine carrier;
		Sine modulator;
		ADSR adsr;
		param f0;

		event on(Pitch pitch, Amplitude velocity) {
			f0 = pitch -> Frequency;
			carrier.reset();
			modulator.reset();
			adsr(0.001, 0, 1, 0.25);
		}

		event off(Amplitude velocity) {
			adsr.release();
		}

		void process() {
			param mod_rate = controls[0] * f0;
			param mod_depth = controls[1] * f0;
			
			signal mod = modulator(mod_rate) * mod_depth;
			carrier(f0 + mod) * adsr >> out;

			if (adsr.finished())
				stop();
		}
	};

	// Initialise plugin (called once at startup)
	FM() {
		controls = {
			Dial("Rate", 0.01, 3, 0.5),
			Dial("Depth", 0, 10, 0.5),
		};

		notes.add<FMNote>(32);
	}
};