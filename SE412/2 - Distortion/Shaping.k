
#include <klang.h>
using namespace klang::optimised;

float hardclip(float x){
	if(x > 1)
		return 1;
	else if(x < -1)
		return -1;
	else
		return x;
}

float softclip(float x, float c){
	return tanh(c * x) / tanh(c);
}

struct Shaping : Effect {

	// Initialise plugin (called once at startup)
	Shaping() {
		controls = { 
			Dial("Distort", 0.001, 5.6, 0.001),
		};
	}

	// Prepare for processing (called once per buffer)
	void prepare() {
		
	}

	// Apply processing (called once per sample)
	void process() {
		param distort = controls[0];
		
		hardclip >> graph;
		Function f(softclip);
		f.with(distort) >> graph;
			
		in >> f(distort) >> out;
	}
};