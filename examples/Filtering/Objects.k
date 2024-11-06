
#include <klang.h>
using namespace klang::optimised;

struct Objects : Effect {
	LPF filter;
	Noise noise;

	// Initialise plugin (called once at startup)
	Objects() {
		controls = { 
			Dial("Cutoff", 10, 1000, 1000),
			Dial("Resonance", 0.1, 10, root2),
		};
	}

	// Prepare for processing (called once per buffer)
	void prepare() {
		filter(controls[0], controls[1]);
	}

	// Apply processing (called once per sample)
	void process() {
		noise >> filter >> out;
	}
};