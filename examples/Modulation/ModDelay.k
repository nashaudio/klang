
#include <klang.h>
using namespace klang::optimised;

struct ModDelay : Effect {

	Delay<192000> delay;
	Sine lfo;

	// Initialise plugin (called once at startup)
	ModDelay() {
		controls = { 
			Dial("Mod Rate", 1, 10, 6),
			Dial("Mod Depth", 0.0, 1.0, 0.2),
		};
	}

	// Apply processing (called once per sample)
	void process() {
		param rate = controls[0];
		param depth = cube(controls[1].smooth()) / 10.f;
		
		signal mod = lfo(rate) * depth + depth;
		mod * 10.f >> debug;
		(in >> delay)(mod * fs) >> out;
	}
};