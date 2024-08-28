
#include <klang.h>
using namespace klang::optimised;

struct Tremolo : Effect {
	Sine lfo;

	// Initialise plugin (called once at startup)
	Tremolo() {
		controls = { 
			Dial("Mod Rate", 1.0, 10.0, 6.0),
			Dial("Mod Depth", 0.0, 0.5, 0.5),
		};
	}

	// Prepare for processing (called once per buffer)
	void prepare() {
		
	}

	// Apply processing (called once per sample)
	void process() {
		param rate = controls[0];
		param depth = controls[1];
		
		signal mod = lfo(rate) * depth + (1 - depth);
		mod >> debug;
		in * mod >> out;
	}
};