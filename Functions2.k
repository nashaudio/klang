
#include <klang.h>
using namespace klang::optimised;

struct Functions2 : Effect {

	struct HardClipping : Function<float> {
		void process() {
			if(in > 1)
				out = 1;
			else if(in < -1)
				out = -1;
			else
				out = in;
		}	
	} hardclip;

	// Initialise plugin (called once at startup)
	Functions2() {
		controls = { 
			Dial("Overdrive", 1.0, 25.0, 1.0),
		};
		
		hardclip >> graph(-2,2,-2,2);
	}

	// Prepare for processing (called once per buffer)
	void prepare() {
		
	}

	// Apply processing (called once per sample)
	void process() {
		param gain = controls[0];
		
		in * gain >> hardclip >> out;
	}
};