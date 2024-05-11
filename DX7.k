
#include <klang.h>

using namespace klang::optimised;

enum { Ratio, Fixed }; // OSC_MODE (+ SYNC?!)

constexpr double NA = NAN;

namespace DX {

//////////////////////// PATCH DATA ////////////////////////

struct Patch {
	struct Osc { int MODE; float FREQ; int DETUNE; };
	struct Env { float RATE, LEVEL; };
	
	struct Op {
		Osc OSC = { Ratio, 1.f, 0 };
		Env EG[4] = { 0 };
		float BREAKPOINT;
		float LEFTCURVE;
		float LEFTDEPTH;
		float RIGHTCURVE;
		float RIGHTDEPTH;
		float RATESCALING;
		float OUTPUTLEVEL = 0;
		float KEYVELOCITY;
		// TODO: other OP stuff
	} OP[6];
	// TODO: GLOBAL stuff		
};

// TUB BELLS
constexpr Patch TUB_BELLS = { 
{ {	/* OP1 */ { Ratio, 1.000, 2  }, { { 95,99 }, { 33,0 }, { 71,32 }, { 28,0 } }, 0,0,0,0,0,2, 95, 0, },
  {	/* OP2 */ { Ratio, 3.500, 3  }, { { 98,99 }, { 12,0 }, { 71,32 }, { 28,0 } }, 0,0,0,0,0,2, 78, 0, }, 
  {	/* OP3 */ { Ratio, 1.000, -5 }, { { 95,99 }, { 33,0 }, { 71,32 }, { 25,0 } }, 0,0,0,0,0,2, 99, 0, },
  {	/* OP4 */ { Ratio, 3.500, -2 }, { { 98,99 }, { 12,0 }, { 71,32 }, { 28,0 } }, 0,0,0,0,0,2, 75, 0, }, 
  {	/* OP5 */ { Ratio, 323.6, 0  }, { { 76,0 },  { 78,0 }, { 71, 0 }, { 70,0 } }, 0,0,0,0,0,2, 00, 0, }, 
  {	/* OP6 */ { Ratio, 2.000, -7 }, { { 98,0 },  { 91,0 }, {  0, 0 }, { 28,0 } }, 0,0,0,0,0,2, 00, 0, }, },
};

constexpr Patch E_PIANO_1 = { 
{ {	/* OP1 */ { Ratio, 1.000, 3  }, { { 96,99 }, { 25,75 }, { 25,99 }, { 67,0 } }, NA,NA,NA,NA,NA,NA, 99, NA, },
  {	/* OP2 */ { Ratio, 14.00, 0  }, { { 95,99 }, { 50,75 }, { 35,99 }, { 78,0 } }, NA,NA,NA,NA,NA,NA, 58, NA, }, 
  {	/* OP3 */ { Ratio, 1.000, 0  }, { { 95,99 }, { 20,95 }, { 20,99 }, { 50,0 } }, NA,NA,NA,NA,NA,NA, 99, NA, },
  {	/* OP4 */ { Ratio, 1.000, 0  }, { { 95,99 }, { 29,95 }, { 20,99 }, { 50,0 } }, NA,NA,NA,NA,NA,NA, 89, NA, }, 
  {	/* OP5 */ { Ratio, 1.000, -7 }, { { 95,99 }, { 20,95 }, { 20,99 }, { 50,0 } }, NA,NA,NA,NA,NA,NA, 99, NA, }, 
  {	/* OP6 */ { Ratio, 1.000, -7 }, { { 95,99 }, { 29,95 }, { 20,99 }, { 50,0 } }, NA,NA,NA,NA,NA,NA, 79, NA, }, },
};

constexpr Patch HARPSICH_1 = { 
{ {	/* OP1 */ { Ratio, 4.000, 0  }, { { 95,99 }, { 28,90 }, { 27,0  }, { 47,0  } }, NA,NA,NA,NA,NA,NA, 89, NA, },
  {	/* OP2 */ { Ratio, 0.500, 0  }, { { 95,99 }, { 72,97 }, { 71,91 }, { 99,98 } }, NA,NA,NA,NA,NA,NA, 99, NA, }, 
  {	/* OP3 */ { Ratio, 1.000, -1 }, { { 95,99 }, { 28,90 }, { 27,0  }, { 47,0  } }, NA,NA,NA,NA,NA,NA, 85, NA, },
  {	/* OP4 */ { Ratio, 3.000, 0  }, { { 95,99 }, { 72,97 }, { 71,91 }, { 99,98 } }, NA,NA,NA,NA,NA,NA, 99, NA, }, 
  {	/* OP5 */ { Ratio, 4.000, -1 }, { { 95,99 }, { 28,90 }, { 27,0  }, { 47,0  } }, NA,NA,NA,NA,NA,NA, 83, NA, }, 
  {	/* OP6 */ { Ratio, 6.000, 0  }, { { 95,99 }, { 72,97 }, { 71,91 }, { 99,98 } }, NA,NA,NA,NA,NA,NA, 87, NA, }, },
};

constexpr Patch Presets[] = {
	TUB_BELLS, E_PIANO_1, HARPSICH_1
};

/////////////////// CONVERSION FUNCTIONS ///////////////////

double Detune(int detune) {
	constexpr float Detune[15] = { -3.5f, -3.f, -2.5f, -2.f, -1.5f, -1.f, -.5f, 0.f,
									.5f, +1.f, +1.5f, +2.f, +2.5f, +3.f, +3.5f };									 
	return Detune[detune + 7];
}

struct {
	const Table<double, 128> Attack = F(double) { 	y = 38.7308 * exp(-0.1067 * x); };
	const Table<double, 128> Decay = F(double) { 	y = 311.5048 * exp(-0.1110 * x); };
	const Table<double, 128> Percent = F(double) {  		y =   x < 32  ? 0.00001 
		: x < 36  ? y[-1] + 0.005	: x < 51  ? y[-1] + 0.008 	: x < 61  ? y[-1] + 0.010	
		: x < 71  ? y[-1] + 0.011	: x < 81  ? y[-1] + 0.015	: x < 91  ? y[-1] + 0.020
		: x < 96  ? y[-1] + 0.032	: x < 100 ? y[-1] + 0.035 	: 1.0; };
			
	double operator()(int rate, int level1, int level2) const {
		double duration = (level2 >= level1 ? Attack : Decay)[rate];  
		duration *= fabs(Percent[level2]-Percent[level1]);
		return duration;		
	}
} Duration;

struct {
	const Table<float, 128> Level = F(float) { 	
		constexpr float level[] = { 0, 5, 9, 13, 17, 20, 23, 25, 27, 29, 31, 33, 35, 37, 39, 41, 42, 43, 45, 46 };
		y = (x < 20 ? level[(int)x] : 28 + x);
	};

	float operator()(int level) const {
		if(level == 0) 
			return 0.f;

		//const double scale = 20.0 * (log(2.0) / log(10.0)) / 8.0;
		//return Amplitude(Gain(scale * (Level[level] - 128.0)));
	
		return Amplitude(Gain(0.1f * (Level[level] - 127.f))); // Good for Tubular Bells
//		return Gain(0.495f * (OP[o].OUTPUTLEVEL - 100.f));
	}	
	
	
	int ScaleCurve(int group, int depth, int curve) {
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
	
	int ScaleLevel(int pitch, const Patch::Op& OP) {
		const int split = pitch - OP.BREAKPOINT - 17;
		if (split >= 0)
			return ScaleCurve( (1 + split) / 3, OP.RIGHTDEPTH, OP.RIGHTCURVE);
		else
			return ScaleCurve( (1 - split) / 3, OP.LEFTDEPTH, OP.LEFTCURVE);
	}
	
	
	int ScaleVelocity(int velocity, int sensitivity) {
		constexpr uint8_t velocity_data[64] = {
			0, 	 70,  86,  97,  106, 114, 121, 126, 132, 138, 142, 148, 152, 156, 160, 163,
			166, 170, 173, 174, 178, 181, 184, 186, 189, 190, 194, 196, 198, 200, 202, 205, 
			206, 209, 211, 214, 216, 218, 220, 222, 224, 225, 227, 229, 230, 232, 233, 235, 
			237, 238, 240, 241, 242, 243, 244, 246, 246, 248, 249, 250, 251, 252, 253, 254
		};
		int clamped_vel = max(0, min(127, velocity));
		int vel_value = velocity_data[clamped_vel >> 1] - 239;
		int scaled_vel = ((sensitivity * vel_value + 7) >> 3) << 4;
		return scaled_vel;
	}
	
	
	float getOutputLevel(int pitch, int velocity, const Patch::Op& OP){
        int outlevel = Level[OP.OUTPUTLEVEL];
        //outlevel += ScaleLevel(pitch, OP);
        outlevel = min(127, outlevel);
        //outlevel = outlevel << 5;
        //outlevel += ScaleVelocity(velocity, OP.KEYVELOCITY);
        outlevel = max(0, outlevel);
        
        return Amplitude(Gain(0.1f * (outlevel - 127.f))); // unconfirmed
    }
	
	
} Level;

struct Operator : public klang::Operator<Sine> {
	int pitch, velocity;
	const Patch::Op* OP;

	struct Ramp : public Envelope::Linear {
		int pitch, velocity;
		const Operator& op;
		
		Ramp(const Operator* op) : op(*op) { }
		
		inline float nonlinear(float x) const {
			return x;
		}
		
	
		signal operator++(int) override {
		
//			float actuallevel = Level.getOutputLevel(pitch, velocity, *OP);
//			actuallevel = actuallevel * 64 + outlevel_ - 4256;
		
//		  int newlevel = levels_[ix_];
//        int actuallevel = scaleoutlevel(newlevel) >> 1;
//        actuallevel = (actuallevel << 6) + outlevel_ - 4256;
//        actuallevel = actuallevel < 16 ? 16 : actuallevel;
//        // level here is same as Java impl
//        targetlevel_ = actuallevel << 16;
//        rising_ = (targetlevel_ > level_);
//
//        // rate
//        int qrate = (rates_[ix_] * 41) >> 6;
//        qrate += rate_scaling_;
//        qrate = min(qrate, 63);
//
//        inc_ = (4 + (qrate & 3)) << (2 + LG_N + (qrate >> 2));
//        // meh, this should be fixed elsewhere
//        inc_ = (int)(((int64_t)inc_ * (int64_t)sr_multiplier) >> 24);
		
			Linear::operator++(1);
			return out = nonlinear(out);
		}						
	};
	
	void init(Pitch pitch, Velocity velocity, const Patch::Op& OP){
		Operator::OP = &OP;
		Operator::pitch = int(pitch);
		Operator::velocity = int(velocity * 127.f);
		
		const param fc = pitch > Type::Frequency;
		const auto& OSC = OP.OSC;
		set((OSC.MODE == Ratio ? (float)fc * OSC.FREQ : OSC.FREQ) + DX::Detune(OSC.DETUNE) / 2, 0);

		amp = DX::Level.getOutputLevel(Operator::pitch, Operator::velocity, OP);
		const auto* EG = OP.EG;
		env = 					  		{ { 0, std::max(0.01f,Level(EG[3].LEVEL)) },
			{ Duration(EG[0].RATE, EG[3].LEVEL, EG[0].LEVEL), Level(EG[0].LEVEL) },
			{ Duration(EG[1].RATE, EG[0].LEVEL, EG[1].LEVEL), Level(EG[1].LEVEL) },
			{ Duration(EG[2].RATE, EG[1].LEVEL, EG[2].LEVEL), Level(EG[2].LEVEL) },
		};
		env.sequence();
		env.set(new Operator::Ramp(this));
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
		
		// Note On
		event on(Pitch p, Velocity v) { 		
			preset = controls[0];
			fc = (p-12) > Type::Frequency;
			
			const auto& PATCH = DX::Presets[preset];
			const auto* OP = PATCH.OP;		
			for(int o=0; o<6; o++)
				op[o].init(p, v, OP[o]);
		
			adsr(0.005,0,1,1);
			
//			graph.clear();
//			//for(int o=0; o<6; o++){		
//				debug.print("\nOP %d:\n", 1);
//				for(int p=0; p<6; p++)
//					debug.print("%d: %f,%f\n", p, op[0].env[p].x, op[0].env[p].y);
//				for(int x=0; x<44100; x++){
//					float y = op[0].env++;
//					if(!(x % 10))
//						graph += { (double)x, (float)y };
//					
//				}
//				
//				//graph(0,.01);
//			//}
//			op[0].env.initialise();
		}
		
		event off(Velocity v){
			adsr.release();
		}

		// Apply processing (called once per sample)
		void process() {						

			(op[1] >> op[0]) + (op[3] >> op[2]) + (op[5] >> op[4]) >> out;
			
			//adsr++ >> graph;
								
			out *= adsr++ * 0.5f;
			if(adsr.finished())
				stop();
		}
	};
	
	// Initialise plugin (called once at startup)
	DX7() {		
		controls = { 
			Menu("Preset", "TUB BELLS", "E.PIANO 1", "HARPSICH 1" ),
		};

		//presets = { };

		notes.add<MyNote>(32);
	}
}; 