
#include <klang.h>
using namespace klang::optimised;

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
			{ "Warm", { 1.000, 0.1, 0.250, 5.691, 0.500, 1.500 } },
		};
		
		left.rnd_f = random(0.99, 1.01);
		left.rnd_ad = random(0.99, 1.01);
		left.rnd_ao = random(0.99, 1.01);
		
		right.rnd_f = random(0.99, 1.01);
		right.rnd_ad = random(0.99, 1.01);
		right.rnd_ao = random(0.99, 1.01);
	}

	struct Modulator : Oscillator {
		Triangle osc;
		param depth, offset;
		
		void set(param f, param depth, param offset){
			osc.set(f);
			Modulator::depth = depth;
			Modulator::offset = depth + offset;
		}
		
		void process() {
			osc * depth + offset >> out;
		}
	};
	
	struct Channel : Mono::Modifier {
		Channel(Controls& controls) : controls(controls) { }
	
		Controls& controls;
		Delay<192000> delay;
		Modulator mod[5];
		
		param rnd_f;
		param rnd_ad;
		param rnd_ao;
		
		// Apply processing (called once per sample)
		void process() {
			const param f = rnd_f * controls[1];
			const param f_d = controls[2];
			const param ad = rnd_ad * controls[3].smooth();
			const param ad_d = controls[4];
			const param ao = rnd_ao * controls[5].smooth();

			delay.set(0);
			
			mod[0].set(f - f * 2 * f_d, (ad - ad * 2.f * ad_d) * fs / 1000.f, (ao - 2.f * ao * 0.5f) * fs / 1000.f);
			mod[1].set(f - f * f_d,     (ad - ad * ad_d)       * fs / 1000.f, (ao - ao * 0.5f)       * fs / 1000.f);
			mod[2].set(f, 			     ad                    * fs / 1000.f,  ao                    * fs / 1000.f);
			mod[3].set(f + f * f_d,     (ad + ad * ad_d)       * fs / 1000.f, (ao + ao * 0.5f)       * fs / 1000.f);
			mod[4].set(f + f * 2 * f_d, (ad + ad * 2.f * ad_d) * fs / 1000.f, (ao + 2.f * ao * 0.5f) * fs / 1000.f);
	
			const param gain = controls[0];
			
			in >> delay;	
			in + gain * delay(mod[0]) 
			   + gain * delay(mod[1]) 
			   + gain * delay(mod[2]) 
			   + gain * delay(mod[3]) 
			   + gain * delay(mod[4]) >> out;
		}
	};
	
	Channel left, right;
	
//	void prepare(){
//		left.prepare();
//		right.prepare();
//	}
	
	void process(){
		in.l >> left >> out.l;
		in.r >> right >> out.r;
	}
	
};