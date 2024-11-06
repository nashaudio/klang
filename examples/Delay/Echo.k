
#include <klang.h>
using namespace klang::optimised;

struct Echo : Effect {
	Delay<192000> delay;

	// Initialise plugin (called once at startup)
	Echo() {
		controls = { 
			Dial("Delay", 0.0, 1.0, 0.5),
			Dial("Gain", 0.0, 1.0, 0.5),
		};
	}

	// Apply processing (called once per sample)
	void process() {
		param time = controls[0] * fs;
		param gain = controls[1];
		
		in >> delay;
		in + delay(time) * gain >> out;
	}
};