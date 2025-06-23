
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
		
		presets = { { "Bathroom",    { 0.276, 0.057, 3835.062 } },
					{ "Lively Room", { 0.309, 0.060, 806.557 } },
					{ "Air Vent",    { 0.496, 0.089, 2807.678 } },
					{ "Parking Lot", { 0.216, 0.220, 1367.304 } },
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
		param times[3] = { 0.2, 0.324, 0.777 };
		param gains[3] = { 1.0, 0.6, 0.2 };
		signal mix = 0;
		mix += gains[0] * controls[0] * feedback((times[0] * controls[1]) * fs);
		mix += gains[1] * controls[0] * feedback((times[1] * controls[1]) * fs);
		mix += gains[2] * controls[0] * feedback((times[2] * controls[1]) * fs);
		return mix >> filter;
	}
	
	// Apply processing (called once per sample)
	void process() {
		early() + late() >> out >> feedback;
	}
};