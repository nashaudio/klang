
#include <klang.h>
using namespace klang::optimised;

struct IIR : Effect {
	signal last = 0;

	// Initialise plugin (called once at startup)
	IIR() {
		controls = { 
			Dial("Filter", 0.0, 1.0, 0.5),
		};
	}
	
	signal filter(signal dry){
		param f = cube(controls[0]);
		
		signal filtered = (1 - f) * last + f * dry;
		last = filtered;
		
		return filtered;
	}

	// Apply processing (called once per sample)
	void process() {
		filter(in) >> out;
	}
};