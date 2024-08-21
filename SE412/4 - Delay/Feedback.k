
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

	// Prepare for processing (called once per buffer)
	void prepare() {
		delay.set(controls[0] * fs);
	}

	// Apply processing (called once per sample)
	void process() {
		
		param gain = controls[1];
		//in >> delay;
	
		(in + delay * gain) >> out;
		
		delay << out; // feedback delay
	}
};