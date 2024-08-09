
#include <klang.h>
using namespace klang::basic;

struct Chorus : Stereo::Effect {

	// Initialise plugin (called once at startup)
	Chorus() : left(controls), right(controls) {
		controls = { 
			Dial("Gain", 0.0, 1.0, 1.0),
			Dial("Rate", 0.01, 2, 0.1),
			Dial("Rate %", 0, 0.5, 0.5),
			Dial("Depth", 0, 6, 5.5),
			Dial("Depth %", 0, 0.5, 0.5),
			Dial("Delay", 0, 1000, 6),
		};
		
		presets = {
			{ "Warm", { 1.000, 0.1, 0.500, 5.500, 0.500, 1.500 } },
		};
		
		// minor variations for stereo effect
		left.rnd_f =  0.990025043f; //random(0.99, 1.01);
		left.rnd_ad = 1.001271720f; //random(0.99, 1.01);
		left.rnd_ao = 0.993866086f; //random(0.99, 1.01);
		right.rnd_f =  1.00617480f; //random(0.99, 1.01);
		right.rnd_ad = 1.00170016f; //random(0.99, 1.01);
		right.rnd_ao = .999597489f; //random(0.99, 1.01);
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
			(osc * depth + offset) >> out;
		}
	};
	
	struct Channel : Mono::Modifier {
		Channel(Controls& controls) : controls(controls) { }
	
		Controls& controls;
		Delay<192000> delay;
		Modulator mod[5];
		
		Delay<192000> echo;
		
		param rnd_f, rnd_ad, rnd_ao;
		
		void prepare() {
			const param f =    controls[1] * rnd_f; 
			const param f_d =  controls[2];
			const param ad =   controls[3] * rnd_ad;
			const param ad_d = controls[4];
			const param ao =   controls[5] * rnd_ao;
			const float ms = fs / 1000.f;
					
			mod[0].set(f - f * f_d * 2.f, (ad - ad * ad_d * 2.f) * ms, (ao - ao * 1.0f) * ms);
			mod[1].set(f - f * f_d,       (ad - ad * ad_d)       * ms, (ao - ao * 0.5f) * ms);
			mod[2].set(f, 			       ad                    * ms,  ao              * ms);
			mod[3].set(f + f * f_d,       (ad + ad * ad_d)       * ms, (ao + ao * 0.5f) * ms);
			mod[4].set(f + f * f_d * 2.f, (ad + ad * ad_d * 2.f) * ms, (ao + ao * 1.0f) * ms);	
			
			echo.set(0.5 * fs);
		}
		
		void process() {
			const param gain = controls[0];
			
			in >> delay;			
//			in >> echo;
			
			in + gain * ( delay(mod[0]) 
			            + delay(mod[1]) 
			            + delay(mod[2]) 
			            + delay(mod[3]) 
			            + delay(mod[4]) ) >> out;

//			in + gain * echo(0.5 * fs) >> out;
		}
	};
	
	Channel left, right;
	
	void prepare(){
		left.prepare();
		right.prepare();
	}
	
	void process(){
		in.l >> left >> out.l;
		in.r >> right >> out.r;
	}
	
};