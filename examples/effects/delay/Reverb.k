
#include <klang.h>
using namespace klang::optimised;

struct Reverb : Effect {

	Delay<192000> feedforward;
	Delay<192000> feedback;
	
	LPF filter;
	
	param times[8] = { 2.078, 5.154, 5.947, 7.544, 8.878, 10.422, 13.938, 17.140 };
	param gains[8] = {  .609,  .262, -.360, -.470,  .290,  -.423,   .100,   .200 };

	// Initialise plugin (called once at startup)
	Reverb() {
		controls = { 
			Dial("Resonance", 0, 0.5, 0.4),
			Dial("Room Size", 0, 0.4, 0.1),
			Dial("Brightness", 500, 5000, 1500),
		};
	}

	// Prepare for processing (called once per buffer)
	void prepare() {
		filter.set(controls[2]);
	}
	
	signal early() {
		in >> feedforward;
		signal mix = in;
		for(int d=0; d<8; d++)
			mix += feedforward(times[d] * fs/1000) * gains[d];
		return mix;
	}
	
	signal late() {
		return controls[0] * feedback((controls[1]) * fs) >> filter;
	}
	
	// Apply processing (called once per sample)
	void process() {
		early() + late() >> out >> feedback;
	}
};