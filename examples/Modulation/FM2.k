
#include <klang.h>
using namespace klang::optimised;

struct FM2 : Synth {

	struct FM2Note : public Note {
		Sine carrier;
		Sine modulator1, modulator2;
		ADSR adsr;
		param f0;

		event on(Pitch pitch, Amplitude velocity) {
			f0 = pitch -> Frequency;
			carrier.reset();
			modulator1.reset();
			modulator2.reset();
			adsr(0.5, 0, 1, 0.25);
		}

		event off(Amplitude velocity) {
			adsr.release();
		}

		void process() {
			param mod_rate = controls[0] * f0;
			param mod_depth1 = controls[1] * f0;
			param mod_depth2 = controls[2] * f0;
			
			signal mod1 = modulator1(mod_rate) * mod_depth1;
			signal mod2 = modulator2(f0 + mod1) * mod_depth2;
			
			carrier(f0 + mod2) * adsr >> out;
			if (adsr.finished())
				stop();
		}
	};

	// Initialise plugin (called once at startup)
	FM2() {
		controls = {
			Dial("Rate", 0.01, 3, 0.5),
			Dial("Depth 1", 0, 10, 0.5),
			Dial("Depth 2", 0, 10, 0.5),
		};
		
		presets = { 
			{ "Violin", { 3.000, 10.000, 6.791 } },
			{ "Cello", { 1.490, 7.076, 1.523 } },
			{ "Oboe", { 3.000, 0.755, 10.000 } },
			{ "Harmonica", { 2.500, 4.900, 8.443 } },
		};

		notes.add<FM2Note>(32);
	}
};