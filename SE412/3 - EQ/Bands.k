
#include <klang.h>
using namespace klang::optimised;

struct Bands : Effect {

	BPF filter1;
	BPF filter2;

	// Initialise plugin (called once at startup)
	Bands() {
		controls = { 
			Dial("Band1 - f", 10, 1000, 100),
			Dial("Band1 - Q", 0.1, 10, root2),
			Dial("Band2 - f", 10, 1000, 500),
			Dial("Band2 - Q", 0.1, 10, root2),
		};
	}

	// Prepare for processing (called once per buffer)
	void prepare() {
		
	}

	// Apply processing (called once per sample)
	void process() {
		param gain = controls[0];
		
		signal band1 = in >> filter1(controls[0], controls[1]);
		signal band2 = in >> filter2(controls[2], controls[3]);
		
		band1 + band2 >> out;
	}
};