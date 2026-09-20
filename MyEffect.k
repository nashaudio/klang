
#include <klang.h>
using namespace klang::optimised;

struct MyEffect : Effect {

	// Initialise plugin (called once at startup)
	MyEffect() {
		controls = { 
			Dial("Gain", 0.0, 1.0, 0.5),
		};
	}

	// Prepare for processing (called once per buffer)
	void prepare() {
		
	}

	// Apply processing (called once per sample)
	void process() {
		param gain = controls[0];
		in * gain >> out;
	}
};