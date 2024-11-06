
#include <klang.h>
using namespace klang::optimised;

struct Mute : Effect {

	// Initialise plugin (called once at startup)
	Mute() {
		controls = { 
			Toggle("Mute"),
		};
	}

	// Prepare for processing (called once per buffer)
	void prepare() {
		
	}

	// Apply processing (called once per sample)
	void process() {
		param gain;
		param mute = controls[0];
		
		if(mute)
			gain = 0.f;
		else
			gain = 1.f;
	
		in * gain >> out;
	}
};