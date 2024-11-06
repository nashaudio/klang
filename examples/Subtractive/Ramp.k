
#include <klang.h>
using namespace klang::optimised;

struct Ramp : Synth {

	struct RampNote : public Note {
		Sine osc;
		Envelope env;

		event on(Pitch pitch, Amplitude velocity) {
			osc(pitch -> Frequency, 0);
			
			param time = controls[0];		
			env = { { 0, 1 }, { time, 0 } };
		}
		
		void process() {
			osc * env++ >> out;
		}
	};

	Ramp() {
		controls = {
			Dial("Time", 0.1, 1.0, 0.1) 
		};

		notes.add<RampNote>(32);
	}
};