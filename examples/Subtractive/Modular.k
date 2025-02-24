#include <klang.h>
using namespace klang::optimised;

struct Distortion : Modifier {
	param gain = 1;
	
	void set(param drive){
		gain = drive;
	}
	
	void process() {
		if(gain > 1){
			in *= gain;
			if(in > 1)
				in = 1;
			else if(in < -1) 
				in = -1;
		}
		in >> out;
	}
};

signal invert(signal x){ return 1 - x; }
signal nonlinear(signal x){ return pow(10, 2 * (x - 1)); }
signal squared(signal x){ return x * x; }

struct OSC : Generator {
	Saw osc1;
	Saw osc2;
	Square osc3;
	param gain[3] = { 0 };
	
	void set(param f, param detune, param gain1, param gain2, param gain3){
		osc1(f * (1 - detune), 0);
		osc2(f, 0);
		osc3(f * (1 + detune), 0);
		gain[0] = squared(gain1) / 3;
		gain[1] = squared(gain2) / 3;
		gain[2] = squared(gain3) / 3;
	}
	
	void process() {
		osc1 * gain[0] + osc2 * gain[1] + osc3 * gain[2] >> out;
	}
};

struct FLT : Modifier {
	LPF lpf;
	HPF hpf;
	BPF bpf;
	int filter = 0;
	param f = 0, Q = 1;
	Distortion drive;
	
	void set(param type){
		filter = (int)type;
		lpf.reset(); 
		hpf.reset();
		bpf.reset();
	}
	
	void set(param frequency, param resonance){
		f = frequency;
		Q = resonance;
	}
	
	void process() {
		switch(filter){
		case 1: 
			in >> hpf(f,Q) >> drive >> out; break;
		case 2: 
			in >> bpf(f,Q) >> drive >> out; break;
		default:
			in >> lpf(f,Q) >> drive >> out; break;
		}
	}
};

struct AMP : Modifier {
	ADSR env;
	param gain = 1;
	
	void process() {
		in * env * gain >> out;
	}
};

struct MOD : Generator {
	ADSR env;
	param amount;
	
	void release() {
		if(env.R > 0.01)
			env.release();
	}
	
	void process() {
		if(amount <= 0)
			nonlinear(invert(env * -amount)) >> out;
		else
			(1 + nonlinear(env * amount) * 9) >> out;
	}
};

struct LFO : Generator {
	Sine osc;
	param amount;
	param depth;
	
	void set(param f, param depth){
		osc.set(f);
		LFO::depth = depth;
	}
	
	void set(param f, param depth, param phase){
		osc.set(f, phase);
		LFO::depth = depth;
	}
	
	void process() {
		signal lfo = pow(2, (signal)osc) * 0.5;
		if(amount <= 0)
			nonlinear(invert(lfo * -amount * depth)) >> out;
		else
			(1 + nonlinear(lfo * amount) * depth * 2) >> out;
	}
};

struct Modular : Synth {

	struct ModularNote : public Note {
		OSC osc;
		FLT flt;
		AMP amp;
		MOD mod;
		LFO lfo;

		event on(Pitch pitch, Amplitude velocity) {
			const param detune = controls[3] * 0.01f;
			osc.set(pitch -> Frequency, detune, controls[0], controls[1], controls[2]);
			
			amp.env(controls[4], controls[5], controls[6], controls[7]);
			mod.env(controls[8], controls[9], controls[10], controls[11]);

			flt.set(controls[12]);
			flt.drive(controls[15]);
			lfo.amount = controls[16];
			mod.amount = controls[17];		
			
			lfo.set(controls[18], controls[19], 0);
		}

		event off(Amplitude velocity) {
			amp.env.release();
			mod.release();
		}

		void process() {
			signal f = min(20000, mod * lfo * controls[13]);
			signal Q = controls[14];
		
			osc >> flt(f,Q) >> amp >> out;
			
			if (amp.env.finished())
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
						Dial("LFO", -1, 1, 0, { 185, 173, 25, 30 } ),
						Dial("MOD", -1, 1, 0, { 220, 173, 25, 30 } ) 
						},
			
			{ "LFO", { 269, 131 },
				Dial("Rate", .1, 10, 1, { 7, 0, 40, 60 } ),
				Dial("Depth", 0, 1, 0, { 58, 0, 40, 60 } ),
			},
		};
		
		presets = {
			{ "Saint-Saens en Seine", { 1.000, 1.000, 1.000, 0.895, 0.491, 0.707, 0.984, 0.730, 0.349, 0.860, 0.790, 0.350, 0, 2617.376, 1.240, 1.000, 0.370, 0.280, 4.466, 0.650 } },
			{ "Popcorn une fois", { 0.315, 1.000, 1.000, 0.720, 0.000, 0.465, 0.160, 0.510, 0.171, 0.000, 1.000, 0.000, 0, 5316.583, 3.088, 1.000, -0.400, -0.720, 4.966, 0.350 } },
		};

		notes.add<ModularNote>(32);
	}
};
