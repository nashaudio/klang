
#include <klang.h>
using namespace klang::optimised;

struct Pan : Stereo::Effect {

	// Initialise plugin (called once at startup)
	Pan() {
		controls = { 
			Dial("Pan", 0.0, 1.0, 0.5),
		};
	}

	// Apply processing (called once per sample)
	void process() {
		param x = controls[0];
		
		in.l * (1 - x) >> out.l;
		in.r * (x)     >> out.r;
	}
};