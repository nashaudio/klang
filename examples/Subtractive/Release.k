
#include <klang.h>
using namespace klang::optimised;

struct Release : Synth {

	struct ReleaseNote : public Note {
		Sine osc;
		Envelope env;

		event on(Pitch pitch, Amplitude velocity) {
			osc(pitch -> Frequency, 0);
			
			param A = controls[0];		
			param D = controls[1];		
			param S = controls[2];		
			env = { { 0, 0 }, { A, 1 }, { A + D, S } };
			env.setLoop(2, 2);
		}
		
		event off(Amplitude velocity) {
			param R = controls[3];
			env.release(R);
		}

		void process() {
			osc * env++ >> out;
			if(env.finished())
				stop();
		}
	};

	Release() {
		controls = {
			Dial("Attack", 0, 1, 0.002),
			Dial("Decay",  0, 1, 0.1),
			Dial("Sustain",0, 1, 0.05),
			Dial("Release",0, 1, 1.0),
		};

		notes.add<ReleaseNote>(32);
	}
};