
#include <klang.h>
using namespace klang::optimised;

signal cubic(signal in){ return in * in; }

struct FM : Synth {

	struct FMNote : public Note {
		Sine carrier;
		Sine modulator;
		ADSR adsr;
		Envelope env;
		param fc;

		event on(Pitch pitch, Amplitude velocity) {
			fc = pitch -> Frequency;
			carrier.reset();
			modulator.reset();
			adsr(0.1, 0, 1, 0.25);
			env = { { 0, 0.1 }, { 3, 1} };
		}

		event off(Amplitude velocity) {
			adsr.release();
		}

		void process() {
			param mod_rate = controls[0] * fc;
			param mod_depth = (env) * controls[1] * mod_rate;

			signal mod = modulator(mod_rate) * mod_depth + fc;
			carrier(mod) * adsr >> out;

			if (adsr.finished())
				stop();
		}
	};

	// Initialise plugin (called once at startup)
	FM() {
		controls = {
			Dial("Rate", 0.01, 10, 0.5),
			Dial("Depth", 0, 10, 0.5),
		};

		notes.add<FMNote>(32);
	}
};