
#include <klang.h>
using namespace klang::optimised;

struct Patterns : Effect {

	Delay<192000> delay;

	// Initialise plugin (called once at startup)
	Patterns() {
		controls = { 
			Menu("Pattern", "1", "2", "3"),
		};
	}

	// Prepare for processing (called once per buffer)
	void prepare() {
		
	}

	// Apply processing (called once per sample)
	void process() {
		in >> delay;
		
		param times[3][3] = {
			{ 0.5f, 1.0f, 1.5f },	{ 0.25f,0.5f, 1.0f },	{ 0.5f, .75f, 1.0f },
		};
		param gains[3][3] = {
			{ .75f, 0.5f, .25f },	{ .25f, 0.5f, .75f },	{ .25f, 0.5f, .25f },
		};
		
		int pattern = controls[0];
		
		signal mix = in;
		for(int d=0; d<3; d++)
			mix += delay(times[pattern][d] * fs) * gains[pattern][d];
		
		mix >> out;
	}
};