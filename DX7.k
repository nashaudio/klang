
#include <klang.h>

using namespace klang::optimised;

enum { Ratio, Fixed }; // OSC_MODE (+ SYNC?!)

constexpr double NA = NAN;

namespace DX {

struct Operator : public klang::Operator<Sine> {
	struct Ramp : public Envelope::Linear {
		
		static inline float nonlinear(float x){
			return x * x * x * x * 0.1;
		}
	
		signal operator++(int) override {
			return nonlinear(Linear::operator++(1));
		}						
	};
	
	klang::Operator<Sine>& operator=(const Envelope& points) override {
		env = points;
		env.set(new DX::Operator::Ramp());
		return *this;
	}
};


//////////////////////// PATCH DATA ////////////////////////

struct Patch {
	struct Op {
		struct Osc { int MODE; double FREQ; int DETUNE; } OSC = { Ratio, 1.f, 0 };
		struct Env { double RATE, LEVEL; } EG[4] = { 0 };
		double BREAKPOINT;
		double LEFTCURVE;
		double LEFTDEPTH;
		double RIGHTCURVE;
		double RIGHTDEPTH;
		double RATESCALING;
		double OUTPUTLEVEL = 0;
		double KEYVELOCITY;
		// TODO: other OP stuff
	} OP[6];
	// TODO: GLOBAL stuff		
};

// TUB BELLS
constexpr Patch TUB_BELLS = { 
{ {	/* OP1 */ { Ratio, 1.000, 2  }, { { 95,99 }, { 33,0 }, { 71,32 }, { 28,0 } }, NA,NA,NA,NA,NA,NA, 95, NA, },
  {	/* OP2 */ { Ratio, 3.500, 3  }, { { 98,99 }, { 12,0 }, { 71,32 }, { 28,0 } }, NA,NA,NA,NA,NA,NA, 78, NA, }, 
  {	/* OP3 */ { Ratio, 1.000, -5 }, { { 95,99 }, { 33,0 }, { 71,32 }, { 25,0 } }, NA,NA,NA,NA,NA,NA, 99, NA, },
  {	/* OP4 */ { Ratio, 3.500, -2 }, { { 98,99 }, { 12,0 }, { 71,32 }, { 28,0 } }, NA,NA,NA,NA,NA,NA, 75, NA, }, 
  {	/* OP5 */ { Ratio, 323.6, 0  }, { { 76,0 },  { 78,0 }, { 71, 0 }, { 70,0 } }, NA,NA,NA,NA,NA,NA, 00, NA, }, 
  {	/* OP6 */ { Ratio, 2.000, -7 }, { { 98,0 },  { 91,0 }, {  0, 0 }, { 28,0 } }, NA,NA,NA,NA,NA,NA, 00, NA, }, },
	
	// TODO: GLOBAL stuff
};

constexpr Patch E_PIANO_1 = { 
{ {	/* OP1 */ { Ratio, 1.000, 3  }, { { 96,99 }, { 25,75 }, { 25,99 }, { 67,0 } }, NA,NA,NA,NA,NA,NA, 99, NA, },
  {	/* OP2 */ { Ratio, 14.00, 0  }, { { 95,99 }, { 50,75 }, { 35,99 }, { 78,0 } }, NA,NA,NA,NA,NA,NA, 58, NA, }, 
  {	/* OP3 */ { Ratio, 1.000, 0  }, { { 95,99 }, { 20,95 }, { 20,99 }, { 50,0 } }, NA,NA,NA,NA,NA,NA, 99, NA, },
  {	/* OP4 */ { Ratio, 1.000, 0  }, { { 95,99 }, { 29,95 }, { 20,99 }, { 50,0 } }, NA,NA,NA,NA,NA,NA, 89, NA, }, 
  {	/* OP5 */ { Ratio, 1.000, -7 }, { { 95,99 }, { 20,95 }, { 20,99 }, { 50,0 } }, NA,NA,NA,NA,NA,NA, 99, NA, }, 
  {	/* OP6 */ { Ratio, 1.000, -7 }, { { 95,99 }, { 29,95 }, { 20,99 }, { 50,0 } }, NA,NA,NA,NA,NA,NA, 79, NA, }, },
	
	// TODO: GLOBAL stuff
};

constexpr Patch HARPSICH_1 = { 
{ {	/* OP1 */ { Ratio, 4.000, 0  }, { { 95,99 }, { 28,90 }, { 27,0  }, { 47,0  } }, NA,NA,NA,NA,NA,NA, 89, NA, },
  {	/* OP2 */ { Ratio, 0.500, 0  }, { { 95,99 }, { 72,97 }, { 71,91 }, { 99,98 } }, NA,NA,NA,NA,NA,NA, 99, NA, }, 
  {	/* OP3 */ { Ratio, 1.000, -1 }, { { 95,99 }, { 28,90 }, { 27,0  }, { 47,0  } }, NA,NA,NA,NA,NA,NA, 85, NA, },
  {	/* OP4 */ { Ratio, 3.000, 0  }, { { 95,99 }, { 72,97 }, { 71,91 }, { 99,98 } }, NA,NA,NA,NA,NA,NA, 99, NA, }, 
  {	/* OP5 */ { Ratio, 4.000, -1 }, { { 95,99 }, { 28,90 }, { 27,0  }, { 47,0  } }, NA,NA,NA,NA,NA,NA, 83, NA, }, 
  {	/* OP6 */ { Ratio, 6.000, 0  }, { { 95,99 }, { 72,97 }, { 71,91 }, { 99,98 } }, NA,NA,NA,NA,NA,NA, 87, NA, }, },
	
	// TODO: GLOBAL stuff
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
	const Table<double, 128> Level = F(double) { 	
		constexpr double level[] = { 0, 5, 9, 13, 17, 20, 23, 25, 27, 29, 31, 33, 35, 37, 39, 41, 42, 43, 45, 46 };
		y = (x < 20 ? level[(int)x] : 48 + (x - 20));
	};

	double operator()(int level) const {
//		const double scale = 20.0 * (log(2.0) / log(10.0)) / 8.0;
//		return Amplitude(Gain(scale * (Level[level] - 128.0)));
	

		return Amplitude(Gain(0.1f * (Level[level] - 128.f))); // Good for Tubular Bells
//		return Gain(0.495f * (OP[o].OUTPUTLEVEL - 100.f));
	}	
} Level;

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
			fc = p > Type::Frequency;
			
			const auto& PATCH = DX::Presets[preset];
			const auto* OP = PATCH.OP;
			
			for(int o=0; o<6; o++){
				
				const auto& OSC = OP[o].OSC;
				op[o].set((OSC.MODE == Ratio ? (double)fc * OSC.FREQ : OSC.FREQ) + DX::Detune(OSC.DETUNE) / 2, 0);
				op[o].amp = DX::Level(OP[o].OUTPUTLEVEL);
				const auto* EG = OP[o].EG;
				op[o] = 							 { { 0, std::max(0.01,DX::Level(EG[3].LEVEL)) },
					{ DX::Duration(EG[0].RATE, EG[3].LEVEL, EG[0].LEVEL), DX::Level(EG[0].LEVEL) },
					{ DX::Duration(EG[1].RATE, EG[0].LEVEL, EG[1].LEVEL), DX::Level(EG[1].LEVEL) },
					{ DX::Duration(EG[2].RATE, EG[1].LEVEL, EG[2].LEVEL), DX::Level(EG[2].LEVEL) },
				};
				op[o].env.sequence();
			}
		
			adsr(0.005,0,1,1);
			
			//graph.clear();
			//for(int o=0; o<6; o++){		
			//	debug.print("\nOP %d:\n", o+1);
			//	for(int p=0; p<4; p++){
			//		//graph += { op[0].env[p].x, op[0].env[p].y };
			//		debug.print("%d: %f,%f\n", p, op[o].env[p].x, op[o].env[p].y);
			//	}
			//}
		}
		
		event off(Velocity v){
			adsr.release();
		}

		// Apply processing (called once per sample)
		void process() {						

			(op[1] >> op[0]) + (op[3] >> op[2]) + (op[5] >> op[4]) >> out;
			//(op[1] >> op[0]) >> out;
								
			out *= adsr++ * 0.5;
			if(adsr.finished())
				stop();
		}
	};

	// Initialise plugin (called once at startup)
	DX7() {		
		controls = { 
			// UI controls and parameters
			Menu("Preset", "TUB BELLS", "E.PIANO 1", "HARPSICH 1" ),
		};

		presets = {
			
		};

		notes.add<MyNote>(32);
	}
}; 