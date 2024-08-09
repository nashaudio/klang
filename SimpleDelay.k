
#include <klang.h>
using namespace klang::basic;

struct SimpleDelay : Effect {

	// Initialise plugin (called once at startup)
	SimpleDelay() {
		controls = { 
			Dial("Gain", 0.0, 1.0, 1.0),
			Dial("Rate", 0.01, 2, 0.1),
			Dial("Rate %", 0, 0.5, 0.5),
			Dial("Depth", 0, 6, 5.5),
			Dial("Depth %", 0, 0.5, 0.5),
			Dial("Delay", 0, 1000, 6),
		};
		
	}

	// simple LFO (0.0-1.0)
	struct Modulator : Oscillator {
		Triangle osc;
		param depth, offset;
		
		void set(param f, param depth, param offset){
			osc.set(f);
			Modulator::depth = depth;
			Modulator::offset = depth + offset;
		}
		
		void process() {
			(osc * depth + offset) >> out;// >> debug;
		}
	} mod;
	
	void prepare(){
		const param f =    controls[1]; 
		const param f_d =  controls[2];
		const param ad =   controls[3];
		const param ad_d = controls[4];
		const param ao =   controls[5];
		const float ms = fs / 1000.f;
		
		mod.set(f, ad * ms, ao * ms);
	}
	
	void process(){
		mod >> debug;
		in >> out;
	}
	
};