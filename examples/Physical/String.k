
#include <klang.h>
using namespace klang::optimised;

struct String : Synth {

	struct StringNote : public Note {
		Envelope env;
		Noise noise;
		Delay<19200> ff_delay;
		LPF ff_filter;
		
		Delay<19200> delay;
		LPF filter;
		APF apf;
		
		ADSR adsr;
		DCF dc;

		event on(Pitch pitch, Amplitude velocity) {
			const param f = pitch -> Frequency;
			
			const param fc = controls[1] * fs.nyquist * pitch / 96;
			filter.set(fc, 0.5);
			apf.set(fc, 0.99);
			
			ff_filter.set(controls[3] * fs.nyquist);
					
			const param t = fs.f / f - filter.delay(f) - apf.delay(f);
			delay.set(t);
			
			ff_delay.set(t * controls[2] * controls[2]);
			env = { { 0.000, 0.0 }, { 0.001, 1.0 }, { 0.003, -1 }, { 0.004, 0.0 } };
			
			adsr(0, 0, 1, 2.0);
		}

		event off(Amplitude velocity) {
			adsr.release();
		}

		void process() {
			signal excitation = env * (controls[4] * noise + 1 - controls[4])>> ff_filter;
			excitation -= (excitation >> ff_delay);
		
			signal feedback = (delay.lagrange(delay.time) >> filter >> apf) * controls[0];
			
			(excitation + feedback) * adsr >> out;
			
			delay << out;
			if (adsr.finished())
				stop();
				
			out >> dc >> out;
		}
	};

	// Initialise plugin (called once at startup)
	String() {
		controls = {
			Dial("Resonance", 0.9, 0.999, 0.997),
			Dial("Dampening", 0, 1, 0.9),
			Dial("Finger Position", 0, 1, 0.5),
			Dial("Pluck Material", 0, 1, 0.9),
			Dial("Noise")
		};

		notes.add<StringNote>(32);
	}
};