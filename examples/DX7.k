
#include <klang.h>

using namespace klang::optimised;

enum { Ratio, Fixed }; // OSC_MODE (+ SYNC?!)

constexpr int NA = 0;

namespace DX {

//////////////////////// PATCH DATA ////////////////////////

struct Patch {
	int ALGORITHM;
	
	struct Osc { int MODE; float FREQ; int DETUNE; };
	struct Env { int RATE, LEVEL; };
	
	struct Op {
		Osc OSC = { Ratio, 1.f, 0 };
		Env EG[4] = { 0 };
		int BREAKPOINT;
		int LEFTCURVE;
		int LEFTDEPTH;
		int RIGHTCURVE;
		int RIGHTDEPTH;
		int RATESCALING;
		int OUTPUTLEVEL = 0;
		int KEYVELOCITY;
		// TODO: other OP stuff
	} OP[6];
	// TODO: GLOBAL stuff		
};

constexpr Patch HARPSICH_1 = { 5,
{ {	/* OP1 */ { Ratio, 4.000, 0  }, { { 95,99 }, { 28,90 }, { 27,0  }, { 47,0  } }, 49,0,0,0,0, 3, 89, 2, },
  {	/* OP2 */ { Ratio, 0.500, 0  }, { { 95,99 }, { 72,97 }, { 71,91 }, { 99,98 } }, 49,0,0,0,0, 1, 99, 0, }, 
  {	/* OP3 */ { Ratio, 1.000, -1 }, { { 95,99 }, { 28,90 }, { 27,0  }, { 47,0  } }, 49,0,0,0,0, 3, 85, 2, },
  {	/* OP4 */ { Ratio, 3.000, 0  }, { { 95,99 }, { 72,97 }, { 71,91 }, { 99,98 } }, 64,0,0,0,46,1, 99, 0, }, 
  {	/* OP5 */ { Ratio, 4.000, -1 }, { { 95,99 }, { 28,90 }, { 27,0  }, { 47,0  } }, 49,0,0,0,0, 3, 83, 3, }, 
  {	/* OP6 */ { Ratio, 6.000, 0  }, { { 95,99 }, { 72,97 }, { 71,91 }, { 99,98 } }, 64,0,0,0,55,1, 87, 0, }, },
};

// TUB BELLS
constexpr Patch TUB_BELLS = { 5,
{ {	/* OP1 */ { Ratio, 1.000, 2  }, { { 95,99 }, { 33,0 }, { 71,32 }, { 25,0 } }, 0,0,0,0,0,2, 95, 0, },
  {	/* OP2 */ { Ratio, 3.500, 3  }, { { 98,99 }, { 12,0 }, { 71,32 }, { 28,0 } }, 0,0,0,0,0,2, 78, 0, }, 
  {	/* OP3 */ { Ratio, 1.000, -5 }, { { 95,99 }, { 33,0 }, { 71,32 }, { 25,0 } }, 0,0,0,0,0,2, 99, 0, },
  {	/* OP4 */ { Ratio, 3.500, -2 }, { { 98,99 }, { 12,0 }, { 71,32 }, { 28,0 } }, 0,0,0,0,0,2, 75, 0, }, 
  {	/* OP5 */ { Ratio, 323.6, 0  }, { { 76,0 },  { 78,0 }, { 71, 0 }, { 70,0 } }, 0,0,0,0,0,2, 99, 5, }, 
  {	/* OP6 */ { Ratio, 2.000, -7 }, { { 98,0 },  { 91,0 }, {  0, 0 }, { 28,0 } }, 0,0,0,0,0,2, 85, 0, }, },
};

constexpr Patch E_PIANO_1 = { 5,
{ {	/* OP1 */ { Ratio, 1.000, 3  }, { { 96,99 }, { 25,75 }, { 25,0 }, { 67,0 } },  0,0,0,0, 0,3, 99, 2, },
  {	/* OP2 */ { Ratio, 14.00, 0  }, { { 95,99 }, { 50,75 }, { 35,0 }, { 78,0 } },  0,0,0,0, 0,3, 58, 7, }, 
  {	/* OP3 */ { Ratio, 1.000, 0  }, { { 95,99 }, { 20,95 }, { 20,0 }, { 50,0 } },  0,0,0,0, 0,3, 99, 2, },
  {	/* OP4 */ { Ratio, 1.000, 0  }, { { 95,99 }, { 29,95 }, { 20,0 }, { 50,0 } },  0,0,0,0, 0,3, 89, 6, }, 
  {	/* OP5 */ { Ratio, 1.000, -7 }, { { 95,99 }, { 20,95 }, { 20,0 }, { 50,0 } },  0,0,0,0, 0,3, 99, 0, }, 
  {	/* OP6 */ { Ratio, 1.000, -7 }, { { 95,99 }, { 29,95 }, { 20,0 }, { 50,0 } }, 41,0,0,0,19,3, 79, 6, }, },
};

constexpr Patch E_ORGAN_1 = { 32,
{ {	/* OP1 */ { Ratio, 0.500, -2 }, { { 99,99 }, { 80,99 }, { 22,99 }, { 90,0 } }, 0, 0,0,0,0, 0, 94, 0, },
  {	/* OP2 */ { Ratio, 1.010, -6 }, { { 99,99 }, { 20,99 }, { 22,97 }, { 90,0 } }, 0, 0,0,0,10,0, 94, 0, }, 
  {	/* OP3 */ { Ratio, 1.500, 4  }, { { 99,99 }, { 80,99 }, { 54,99 }, { 82,0 } }, 0, 0,0,0,0, 0, 94, 0, },
  {	/* OP4 */ { Ratio, 0.500, 5  }, { { 99,99 }, { 80,99 }, { 22,99 }, { 90,0 } }, 0, 0,0,0,0, 0, 94, 0, }, 
  {	/* OP5 */ { Ratio, 1.000, 2  }, { { 99,99 }, { 80,99 }, { 22,99 }, { 90,0 } }, 0, 0,0,0,0, 0, 94, 0, }, 
  {	/* OP6 */ { Ratio, 3.000, 0  }, { { 99,99 }, { 54,0  }, { 22,0  }, { 90,0 } }, 0, 0,0,0,0, 0, 94, 0, }, },
};

constexpr Patch STRINGS_1 = { 2,
{ {	/* OP1 */ { Ratio, 1.000, 0  }, { { 45,99 }, { 24,85 }, { 20,70 }, { 41,0 } },  0,0,0,0, 0,2, 99, 0, },
  {	/* OP2 */ { Ratio, 1.000, 0  }, { { 75,82 }, { 71,92 }, { 17,62 }, { 49,0 } }, 54,0,0,0, 0,1, 83, 0, }, 
  {	/* OP3 */ { Ratio, 1.000, 0  }, { { 44,99 }, { 45,85 }, { 20,82 }, { 54,0 } }, 56,0,0,0,97,0, 86, 0, },
  {	/* OP4 */ { Ratio, 1.000, 0  }, { { 96,99 }, { 19,92 }, { 20,86 }, { 54,0 } },  0,0,0,0, 0,2, 77, 0, }, 
  {	/* OP5 */ { Ratio, 3.000, 0  }, { { 53,86 }, { 19,92 }, { 20,86 }, { 54,0 } },  0,0,0,0, 0,2, 84, 0, }, 
  {	/* OP6 */ { Ratio, 14.00, 0  }, { { 53,99 }, { 19,92 }, { 20,86 }, { 54,0 } },  0,0,0,0, 0,2, 53, 0, }, },
};

constexpr Patch Presets[] = {
	TUB_BELLS, E_PIANO_1, E_ORGAN_1, HARPSICH_1, STRINGS_1
};

/////////////////// CONVERSION FUNCTIONS ///////////////////

double Detune(int detune) {
	constexpr float Detune[15] = { -3.5f, -3.f, -2.5f, -2.f, -1.5f, -1.f, -.5f, 0.f,
									 .5f, +1.f, +1.5f, +2.f, +2.5f, +3.f, +3.5f };									 
	return Detune[detune + 7];
}

struct {
	// Output level in units of .75dB
	const Table<int, 128> Output = FUNCTION(int) { 	
		y = (x < 20) ? (int[20]) { 0,5,9,13,17,20,23,25,27,29,31,33,35,37,39,41,42,43,45,46 } [x] : 28  + x;
	};
	
	const Table<int, 128> Actual = FUNCTION(int) { 	
		y = (x < 5)  ? 2 * x 
		  : (x < 17) ? 5 + x 
		  : (x < 20) ? 4 + x
		  			 : 14 + (x >> 1);
	};
	
	static int ScaleCurve(int group, int depth, int curve) {
		constexpr uint8_t exp_scale_data[] = {
			0,  1,  2,  3,  4,  5,  6,  7,  8,  9,  11, 14, 16, 19, 23, 27, 
			33, 39, 47, 56, 66, 80, 94, 110,126,142,158,174,190,206,222,238,250
		};
	
		int scale;
		if (curve == 0 || curve == 3) { // linear
			scale = (group * depth * 329) >> 12;
		} else { 						// exponential
			const int raw_exp = exp_scale_data[std::min(group, (int)sizeof(exp_scale_data) - 1)];
			scale = (raw_exp * depth * 329) >> 15;
		}
		if (curve < 2)
			scale = -scale;
		return scale;
	}
	
	static int ScaleLevel(int pitch, const Patch::Op& OP) {
		const int split = pitch - OP.BREAKPOINT - 17;
		if (split >= 0)
			return ScaleCurve( (1 + split) / 3, OP.RIGHTDEPTH, OP.RIGHTCURVE);
		else
			return ScaleCurve( (1 - split) / 3, OP.LEFTDEPTH, OP.LEFTCURVE);
	}
	
	static int ScaleVelocity(int velocity, int sensitivity) {
		constexpr uint8_t velocity_data[64] = {
			0, 	 70,  86,  97,  106, 114, 121, 126, 132, 138, 142, 148, 152, 156, 160, 163,
			166, 170, 173, 174, 178, 181, 184, 186, 189, 190, 194, 196, 198, 200, 202, 205, 
			206, 209, 211, 214, 216, 218, 220, 222, 224, 225, 227, 229, 230, 232, 233, 235, 
			237, 238, 240, 241, 242, 243, 244, 246, 246, 248, 249, 250, 251, 252, 253, 254
		};
		int clamped_vel = std::clamp(velocity, 0, 127);
		int vel_value = velocity_data[clamped_vel >> 1] - 239;
		int scaled_vel = ((sensitivity * vel_value + 7) >> 3) << 4;
		return scaled_vel;
	}
	
	int getOutputLevel(int pitch, int velocity, const Patch::Op& OP){
        int outlevel = Output[OP.OUTPUTLEVEL];
        outlevel += ScaleLevel(pitch, OP);
        outlevel = std::min(127, outlevel);
        outlevel = outlevel * 32;
        outlevel += ScaleVelocity(velocity, OP.KEYVELOCITY);
        outlevel = max(0, outlevel);								// up to 4256              
        return outlevel;
    }
    
    int getTargetLevel(int envlevel, int outlevel){
    	//return std::max(0, Actual[envlevel] * 64 + OutputLevel);	// up to 3840
    	return std::max(0, (Output[envlevel] << 5) - 224);
		//return std::max(0, (Output[envlevel] << 5) - 224 + (Output[outlevel] << 6) - 4256);
	}   
	
} Level;

struct Operator : public klang::Operator<Sine> {
	int pitch, velocity;
	const Patch::Op* OP = nullptr;
	int outlevel;
	signal last = 0, feedback = 0;

	struct Ramp : public Envelope::Linear {
		const Operator& op;
		const float sr_multiplier;
		
		bool rising;
		int qrate, shift;
		unsigned int i = 0; // sample counter
		
		bool step(int i) const {
			constexpr bool mask[4][8]={{0,1,0,1,0,1,0,1},
									   {0,1,0,1,0,1,1,1},
									   {0,1,1,1,0,1,1,1},
									   {0,1,1,1,1,1,1,1}};
			if (shift < 0) {
				const int shiftmask = (1 << -shift) - 1;
				if ((i & shiftmask) != shiftmask) return false;
				i >>= -shift;
			}
			return mask[qrate & 3][i & 7] != 0;
		}
				
				
		float attack(float out) const {
			const int slope = 17 - (int(out) >> 8);
			return slope << max(0, shift);
		}
		
		float decay() const {
  			return 1 << max(0, shift);
		}
							
		Ramp(const Operator* op) : op(*op), sr_multiplier(44100.f/klang::fs) { }
					
		void setRate(float rate) override {
			if(target != out){
				rising = target > out;
			
				const int rate_scaling = (op.OP->RATESCALING * min(31, max(0, op.pitch / 3 - 7))) >> 3;
				qrate = std::min(63, int(rate * 41.f/64) + rate_scaling);
				shift = (qrate >> 2) - 11;
			}
		}
	
		signal operator++(int) override {	
			if(step(i++)){
				rate = (rising ? attack(out) : decay()) * sr_multiplier;
				Linear::operator++(1);	
			}
			return out;
		}
	};
	
	void init(Pitch p, Velocity v, const Patch::Op& OP){
		Operator::OP = &OP;
		const auto& OSC = OP.OSC;

		pitch = int(p);
		velocity = int(v * 99.f); // DX7 only supports velocity 0-99
		
		const param fc = p -> Frequency;
		set((OSC.MODE == Ratio ? (float)fc * OSC.FREQ : OSC.FREQ) + DX::Detune(OSC.DETUNE), 0);
		
		outlevel = Level.getOutputLevel(pitch, velocity, OP);
		amp = outlevel;
		
		const auto* EG = OP.EG;	
		env.setMode(Envelope::Rate);
		env.set(new Operator::Ramp(this));
		env = { {  0, max(1716, Level.getTargetLevel(EG[3].LEVEL, outlevel))},
				 { EG[0].RATE,  Level.getTargetLevel(EG[0].LEVEL, outlevel) },
				 { EG[1].RATE,  Level.getTargetLevel(EG[1].LEVEL, outlevel) },
				 { EG[2].RATE,  Level.getTargetLevel(EG[2].LEVEL, outlevel) } };
		env.setLoop(3,3);
	}
	
	void release() {
		env.release(OP->EG[3].RATE, Level.getTargetLevel(OP->EG[3].LEVEL, outlevel));
	}
	
	// Input feedback signal from op
	Operator& operator<<(const Operator& op) {
		feedback = (op.last + op.out) * 0.5f;
		return *this;
	}
	
	virtual void process() override {
		Sine::set(+(in + feedback));
		
		last = out; // buffer for feedback
		Sine::process();
		out *= Amplitude(dB(((amp + env++) - 8096) * 0.0235f)) * 6.f;
	}
};

};

///////////////////// AUDIO PROCESSING /////////////////////

struct DX7 : Synth {

	struct MyNote : Note {
	
		// Declare components / variables
		DX::Operator op[6];
		param fc;
		ADSR adsr;
		int preset;
		void (MyNote::*algorithm)(void);
		
		// Note On
		event on(Pitch p, Velocity v) { 		
			preset = controls[0];
			fc = (p-12) -> Frequency;
			
			const auto& PATCH = DX::Presets[preset];
			const auto* OP = PATCH.OP;		
			for(int o=0; o<6; o++)
				op[o].init(p, v, OP[o]);
		
			adsr(0.005,0,1,1);
			
//			Envelope& env = op[0].env;
//			graph.clear();
//			for(int x=0; x<50; x++)
//				graph += env++;
			
			switch(PATCH.ALGORITHM){
			case 2:  algorithm = &MyNote::algorithm2; break;
			case 5:  algorithm = &MyNote::algorithm5; break;
			case 32: algorithm = &MyNote::algorithm32; break;
			};
		}
		
		event off(Velocity v){
			for(int o=0; o<6; o++)
				op[o].release();
			adsr.release();
		}
		
		void algorithm2(){	((op[1] << op[1]) >> op[0]) + (op[5] >> op[4] >> op[3] >> op[2]) >> out;   }
		void algorithm5(){	(op[1] >> op[0]) + (op[3] >> op[2]) + (op[5] >> op[4]) >> out;  }
		void algorithm32(){	(op[1] + op[0] + op[3] + op[2] + op[5] + op[4]) >> out;  }
		
		bool finished() {
			if(adsr.finished())
				return true;
			for(int o=0; o<6; o++)
				if(!op[o].env.finished()) 
					return false;
			return true;
		}

		// Apply processing (called once per sample)
		void process() {						
			if(algorithm)
				(this->*algorithm)();
			
			//(op[1] >> op[0]) >> out;
			//((op[1] << op[1]) >> op[0]) >> out;

			out *= adsr++ * 0.1f;
			if(finished())
				stop();
				
//			op[1] >> debug;
		}
	};
	
	
	
	// Initialise plugin (called once at startup)
	DX7() {		
		controls = { 
			Menu("Preset", "TUB BELLS", "E.PIANO 1", "E.ORGAN 1", "HARPSICH 1", "STRINGS 1" )
		};
		controls[0].size = { 50,50,100,25 };

		notes.add<MyNote>(32);
	}
}; 