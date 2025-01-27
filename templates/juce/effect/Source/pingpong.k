
#include <klang.h>
using namespace klang::optimised;

struct PingPong : public Stereo::Effect {
	Stereo::Delay<192000> delay;

	// Initialise plugin (called once at startup)
	PingPong() {
		controls = {
		 	{ "Left",
				Dial("Delay", 0.0, 1.0, 0.5 ),
				Dial("Feedback", 0.0, 1.0, 0.5 )
			},
			{ "Right",
				Dial("Delay", 0.0, 1.0, 0.5 ),
				Dial("Feedback", 0.0, 1.0, 0.5 ),
			}
		};
	}

	// Apply processing (called once per sample)
	void process() {
		stereo::signal time = { controls[0].smooth(), controls[2].smooth() };
		stereo::signal gain = { controls[1], controls[3] };
		
		stereo::signal feedback = (out >> delay)(time * fs) * gain;
		
		in.l + feedback.r >> out.l;
		in.r + feedback.l >> out.r;
	}
};