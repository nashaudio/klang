#include <klang.h>
using namespace klang::optimised;

struct ModDelay : Effect {
	Delay<192000> delay;
	Sine lfo;

	ModDelay() {
		controls = { 
			{ "Mod", Dial("Rate"),
			         Dial("Depth"),
			         Dial("Feedback", 0, 0.9) }
		};
	}

	void process() {
		param rate = controls[0] * 9 + 1;
		param depth = controls[1] * 0.01;
		param gain = controls[2];
		
		signal mod = lfo(rate) * depth + depth;   // generate mod signal
		signal feedback = gain * delay(mod * fs); // generate feedback signal
		in + feedback >> out;                     // output the modulated delay
		delay << out;                             // feedback output to the dela
	}
};