
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

	// Prepare for processing (called once per buffer)
	void prepare() {
		
	}

	// Apply processing (called once per sample)
	void process() {
		delay.set(controls[0] * fs);
		param gain = controls[1];
		
		in + (in >> delay) * gain >> out;
	}
};