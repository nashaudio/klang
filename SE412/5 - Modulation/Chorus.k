
#include <klang.h>
using namespace klang::optimised;

struct Chorus : Effect {

	Delay<192000> delay;
	Sine lfo[3];

	// Initialise plugin (called once at startup)
	Chorus() {
		controls = { 
			Dial("Mod Rate", 1, 10, 6),
			Dial("Mod Depth", 0.0, 1.0, 0.1),
		};
	}
	
	signal mod(int m){
		const param rates[3] = { 2.5,3,3.5 };
		const param depths[3] = { 0.45, 0.5, 0.55 };
		return (lfo[m](rates[m]) * depths[m] + depths[m]) * fs / 1000.f;
	}

	// Apply processing (called once per sample)
	void process() {
		in >> delay;
		
		0.5f * (in + delay(mod(0)) + delay(mod(1)) + delay(mod(2))) >> out;
	}
};