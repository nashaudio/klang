#include <klang.h>
using namespace klang::basic;

float square(float x){
	return x*x;
}

struct MyEffect : Effect {

	Sine lfo;

	// Initialise plugin (called once at startup)
	MyEffect() {
		//square >> graph;
	
		controls = { 
			Dial("Gain", 0.0, 1.0, 0.5),
		};

		presets = { 
			{ "Copied Preset", { 0.500 } },
		};
	}

	// Prepare for processing (called once per buffer)
	void prepare() {
		lfo.set(6);
		char* ptr = nullptr;
		//lfo.set(*ptr);		
	}

	// Apply processing (called once per sample)
	void process() {
		//0.5 >> debug;
		param gain = controls[0];
		in * gain >> out;
	}
};