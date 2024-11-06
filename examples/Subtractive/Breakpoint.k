
#include <klang.h>
using namespace klang::optimised;

struct Breakpoint : Synth {

	struct BreakpointNote : public Note {
		Sine osc;
		Envelope env;

		event on(Pitch pitch, Amplitude velocity) {
			osc(pitch -> Frequency, 0);
			
			param attack = controls[0];		
			param decay = controls[0];		
			env = { { 0, 0 }, { attack, 1 }, { attack + decay, 0 } };
		}

		void process() {
			osc * env++ >> out;
		}
	};

	Breakpoint() {
		controls = {
			Dial("Attack", 0.05, 1.0, 0.05),
			Dial("Decay", 0.1, 1.0, 0.1) 
		};

		notes.add<BreakpointNote>(32);
	}
};