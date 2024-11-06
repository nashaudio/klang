
#include <klang.h>
using namespace klang::optimised;

struct Feedback : Effect {
	Delay<192000> delay;

	// Initialise plugin (called once at startup)
	Feedback() {
		controls = { 
			Dial("Delay", 0.0, 1.0, 0.5),
			Dial("Gain", 0.0, 1.0, 0.5),
		};
	}

	// Apply processing (called once per sample)
	void process() {
		param time = controls[0] * fs;
		param gain = controls[1];
		
		in + delay(time) * gain >> out;
		
		delay << out; // feedback delay
	}
};