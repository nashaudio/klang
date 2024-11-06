
#include <klang.h>
using namespace klang::optimised;

struct Gain : Effect {

	// Initialise plugin (called once at startup)
	Gain() {
		controls = { 
			Dial("Gain", 0.0, 1.0, 0.5),
		};
	}

	// Apply processing (called once per sample)
	void process() {
		param gain = controls[0];
		in * gain >> out;
	}
};