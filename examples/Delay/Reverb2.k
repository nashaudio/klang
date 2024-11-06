
#include <klang.h>
using namespace klang::optimised;

struct Reverb2 : Stereo::Effect {

	Delay<192000> feedforward[2];
	Delay<192000> feedback[2];
	
	LPF filter[2];
	
	param times[2][8] = { 2.078, 5.154, 5.947, 7.544, 8.878, 10.422, 13.938, 17.140 };
	param gains[8] = {  .609,  .262, -.360, -.470,  .290,  -.423,   .100,   .200 };

	// Initialise plugin (called once at startup)
	Reverb2() {
		controls = { 
			Dial("Resonance", 0, 0.5, 0.4),
			Dial("Room Size", 0, 0.4, 0.2),
			Dial("Brightness", 500, 5000, 2250),
		};
		
		for(int t2=0; t2<8; t2++)
			times[1][t2] = times[0][t2] * (1.f - 0.2f * gains[t2]);
	}

	// Prepare for processing (called once per buffer)
	void prepare() {
		filter[0].set(controls[2]);
		filter[1].set(controls[2]);
	}
	
	signal early(int c) {
		in[c] >> feedforward[c];
		signal mix = in[c];
		for(int d=0; d<8; d++){
			mix += feedforward[c](times[c][d] * fs/1000) * gains[d];
		}
		return mix;
	}
	
	signal late(int c) {
		return controls[0] * feedback[c]((controls[1] + 0.01232 * c) * fs) >> filter[c];
	}
	
	// Apply processing (called once per sample)
	void process() {
		
		early(0) + late(0) >> out.l >> feedback[0];
		early(1) + late(1) >> out.r >> feedback[1];
	}
};