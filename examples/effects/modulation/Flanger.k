
#include <klang.h>
using namespace klang::optimised;

struct Flanger : Effect {

	Delay<192000> delay;
	Triangle lfo;

	// Initialise plugin (called once at startup)
	Flanger() {
		controls = { 
			Dial("Mod Rate", 0.1, 1.0, 0.75),
			Dial("Mod Depth", 0.1, 5.0, 1.5),
		};
	}

	// Apply processing (called once per sample)
	void process() {
		param rate = controls[0];
		param depth = controls[1] / 1000.f;
		
		signal mod = lfo(rate) * depth + depth;
		in + (in >> delay)(mod * fs) >> out;
	}
};