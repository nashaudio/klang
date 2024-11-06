
#include <klang.h>
using namespace klang::optimised;

struct RM : Effect {
	Sine lfo;

	// Initialise plugin (called once at startup)
	RM() {
		controls = { 
			Dial("Mod Rate", 1.0, 1000.0, 6.0),
			Dial("Mod Depth", 0.0, 0.5, 0.5),
		};
	}

	// Apply processing (called once per sample)
	void process() {
		param rate = controls[0];
		param depth = controls[1];
		
		signal mod = lfo(rate);
		mod >> debug;
		in * mod >> out;
	}
};