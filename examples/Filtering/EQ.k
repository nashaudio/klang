
#include <klang.h>
using namespace klang::optimised;

struct EQ : Effect {

	LPF lpf;
	HPF hpf;

	// Initialise plugin (called once at startup)
	EQ() {
		controls = { 
			Dial("Low", 0.0, 1.0, 0.5),
			Dial("Mid", 0.0, 1.0, 0.5),
			Dial("High", 0.0, 1.0, 0.5),
		};
	}

	// Prepare for processing (called once per buffer)
	void prepare() {
		lpf.set(200);
		hpf.set(4000);
	}

	// Apply processing (called once per sample)
	void process() {
		signal lows = in >> lpf;
		signal highs = in >> hpf;
		signal mids = in - (lows + highs);
		
		lows * sqr(controls[0]) + mids * sqr(controls[1]) + highs * sqr(controls[2]) >> out;
	}
};