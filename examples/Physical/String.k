
#include <klang.h>
using namespace klang::optimised;

struct String : Synth {

	struct StringNote : public Note {
		Envelope excitation;
		Delay<19200> delay;
		IIR filter;
		ADSR adsr;

		event on(Pitch pitch, Amplitude velocity) {
			const param f = pitch -> Frequency;
			
			delay.set(fs.f / f - 2);
			excitation = { { 0.000, 0.0 }, { 0.001, 1.0 }, { 0.003, -1.0 }, { 0.004, 0.0 } };
			filter.set(controls[1]);
			
			adsr(0, 0, 1, 2.0);
		}

		event off(Amplitude velocity) {
			adsr.release();
		}

		void process() {
			signal feedback = (delay >> filter) * controls[0];
			
			(excitation + feedback) * adsr >> out;
			
			delay << out;
			
			if (adsr.finished())
				stop();
		}
	};

	// Initialise plugin (called once at startup)
	String() {
		controls = {
			Dial("Resonance", 0, 1, 0.999),
			Dial("Dampening", 0, 1, 0.9),
		};

		notes.add<StringNote>(32);
	}
};