
#include <klang.h>
using namespace klang::optimised;

struct Clipping : Effect {

	// Initialise plugin (called once at startup)
	Clipping() {
		controls = { 
			Dial("Overdrive", 1.0, 25.0, 1.0),
		};
	}

	// Prepare for processing (called once per buffer)
	void prepare() {
		
	}

	// Apply processing (called once per sample)
	void process() {
		param gain = controls[0];
		
		in *= gain;
		
		if(in > 1)
			in = 1;
		else if(in < -1)
			in = -1;
		
		in >> out;
	}
};