
#include <klang.h>
using namespace klang::optimised;

struct PingPong : Stereo::Effect {

	Stereo::Delay<192000> delay;

	// Initialise plugin (called once at startup)
	PingPong() {
		controls = { 
			Dial("Delay L", 0.0, 1.0, 0.25),
			Dial("Feedback L", 0.0, 1.0, 0.5),
			Dial("Delay R", 0.0, 1.0, 0.5),
			Dial("Feedback R", 0.0, 1.0, 0.5),
		};
	}

	// Prepare for processing (called once per buffer)
	void prepare() {

	}

	// Apply processing (called once per sample)
	void process() {
		stereo::signal time = { controls[0], controls[1] };
		stereo::signal gain = { controls[1], controls[3] };
		
		stereo::signal feedback = delay(time * fs) * gain;
			
		in.l + feedback.r >> out.l;
		in.r + feedback.l >> out.r;
	
		delay << out;
	}
};