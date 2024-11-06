
#include <klang.h>
using namespace klang::optimised;

struct WahWah : Effect {
	LPF lpf;
	Sine lfo;
	
	// Initialise plugin (called once at startup)
	WahWah() {
		controls = { 
			Dial("f", 10, 10000, 1000),
			Dial("Q", 0.1, 10, 1),
			Dial("Rate", 4, 10, 6),
		};
	}

	// Apply processing (called once per sample)
	void process() {
		param f = controls[0];
		param Q = controls[1];
		param rate = controls[2];
		
		signal mod = lfo(rate) * 0.5 + 0.5f;
		
		in >> lpf(sqr(mod) * f, Q) >> out;
	}
};