
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

struct Functions : Effect {

	// Initialise plugin (called once at startup)
	Functions() {
		controls = { 
			Dial("Overdrive", 1.0, 25.0, 1.0),
		};
		
		hardclip >> graph(-2,2,-2,2);
	}

	// Apply processing (called once per sample)
	void process() {
		param gain = controls[0];
		
		hardclip(in * gain) >> out;
	}
};