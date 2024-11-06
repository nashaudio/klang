
#include <klang.h>
using namespace klang::optimised;

float hardclip(float in){
	if(in > 1.f)
		return 1.f;
	else if(in < -.8f)
		return -.8f;
	else
		return in;
}

struct Modular : Synth {

	struct ModularNote : public Note {
		Saw osc1;
		Saw osc2;
		Saw osc3;
		
		LPF lpf;
		HPF hpf;
		BPF bpf;
		
		ADSR adsr, mod;
		Sine lfo;

		event on(Pitch pitch, Amplitude velocity) {
			const param f = pitch -> Frequency;
			const param detune = controls[3] * 0.01f;
			osc1(f * (1 - detune), 0);
			osc2(f, 0);
			osc3(f * (1 + detune), 0);
			
			lpf.reset(); hpf.reset(); bpf.reset();
			
			adsr(controls[4], controls[5], controls[6], controls[7]);
			mod(controls[8], controls[9], controls[10], controls[11]);
			lfo.set(controls[18], 0);
		}

		event off(Amplitude velocity) {
			adsr.release();
			mod.release();
		}

		void process() {
			signal oscs = osc1 * controls[0] + osc2 * controls[1] + osc3 * controls[2];
			
			param modDepth = controls[13] * 0.5f;		
			param f = controls[13] + (mod++ * controls[17] + lfo(controls[18]) * controls[16] * controls[19]) * modDepth;
			param Q = controls[14];
			
			lfo.out * controls[16] >> debug;
			
			switch((int)controls[12]){
			case 0: oscs >> lpf(f, Q) >> out; break;
			case 1: oscs >> hpf(f, Q) >> out; break;
			case 2: oscs >> bpf(f, Q) >> out; break;
			}
			
			out = hardclip(out * controls[15]);
		
			out *= adsr * 0.2;
			if (adsr.finished())
				stop();
		}
	};

	// Initialise plugin (called once at startup)
	Modular() {
		controls = {
			{ "OSC",	Slider("1", 0, 1, 1, { 28, 52, 40, 60 } ), 
						Slider("2", 0, 1, 1, { 73, 52, 40, 60 } ),
						Slider("3", 0, 1, 1, { 118, 52, 40, 60 } ), 
						Dial("Detune", 0, 1, 0, { 163, 52, 40, 60 } ), },			
				
			{ "AMP",	Slider("A", 0, 1, 0.5, { 230, 52, 10, 60 } ), 
						Slider("D", 0, 1, 0.5, { 245, 52, 10, 60 } ), 
						Slider("S", 0, 1, 1.0, { 260, 52, 10, 60 } ), 
						Slider("R", 0, 1, 0.5, { 275, 52, 10, 60 } ), },
						
			{ "MOD",	Slider("A", 0, 1, 0.5, { 312, 52, 10, 60 } ), 
						Slider("D", 0, 1, 0.5, { 327, 52, 10, 60 } ), 
						Slider("S", 0, 1, 1.0, { 342, 52, 10, 60 } ), 
						Slider("R", 0, 1, 0.5, { 357, 52, 10, 60 } ), },

			{ "Filter", Menu("", { 178, 208, 71, 20 }, "Low-pass", "High-pass", "Band-pass" ),
						Dial("f", 100, 10000, 10000, { 28, 168, 40, 60 } ),
						Dial("Q", 0.1, 10, 1, { 78, 168, 40, 60 } ),
						Dial("Drive", 1, 24, 1, { 128, 168, 40, 60 } ),
						Dial("LFO", -.9, .9, 0, { 185, 173, 25, 30 } ),
						Dial("MOD", -.9, .9, 0, { 220, 173, 25, 30 } ) 
						},
			
			{ "LFO", { 269, 131 },
				Dial("Rate", .1, 10, 1, { 7, 0, 40, 60 } ),
				Dial("Depth", 0, 1, 0, { 58, 0, 40, 60 } ),
			},
		};

		notes.add<ModularNote>(32);
	}
};