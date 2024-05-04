
#include <klang.h>

using namespace klang::optimised;

enum { Ratio, Fixed }; // OSC_MODE (+ SYNC?!)

const float NA = NAN;

namespace DX {

struct Patch {
	struct Op {
		struct Osc { int MODE; param FREQ; int DETUNE; } OSC = { Ratio, 1.f, 0 };
		struct Env { param LEVEL, RATE; } EG[4] = { 0 };
		param BREAKPOINT;
		param LEFTCURVE;
		param LEFTDEPTH;
		param RIGHTCURVE;
		param RIGHTDEPTH;
		param RATESCALING;
		param OUTPUTLEVEL = 0;
		param KEYVELOCITY;
		// TODO: other OP stuff
	} OP[6];
	// TODO: GLOBAL stuff		
};

// TUB BELLS
const Patch PATCH = { 
{ {	/* OP1 */ { Ratio, 1.000, 2  }, { 0 }, NA,NA,NA,NA,NA,NA, 95, NA, },
  {	/* OP2 */ { Ratio, 3.500, 3  }, { 0 }, NA,NA,NA,NA,NA,NA, 78, NA, }, 
  {	/* OP3 */ { Ratio, 1.000, -5 }, { 0 }, NA,NA,NA,NA,NA,NA, 99, NA, },
  {	/* OP4 */ { Ratio, 3.500, -2 }, { 0 }, NA,NA,NA,NA,NA,NA, 75, NA, }, },
	
	// TODO: GLOBAL stuff
};

};

struct DX7 : Synth {

	struct MyNote : Note, DX::Patch {
	
		// Declare components / variables
		Operator<Sine> op[6];
		param fc;
		ADSR adsr;
		
//		MyNote() {
//			Sine sine; 
//			for(int o=0; o<6; o++){		
//				op[o] = sine;
//			}
//		}
		
		// Note On
		event on(Pitch p, Velocity v) { 		
			fc = p > Type::Frequency;
			
			constexpr float detune[15] = { -3.5f, -3.f, -2.5f, -2.f, -1.5f, -1.f, -.5f, 0.f,
											 .5f, +1.f, +1.5f, +2.f, +2.5f, +3.f, +3.5f };									 
			for(int o=0; o<6; o++){		
				const Op::Osc& OSC = OP[o].OSC;
				op[o].set((OSC.MODE == Ratio ? fc * OSC.FREQ : OSC.FREQ) + detune[OSC.DETUNE+7],0);
				op[o] = { {0,1} };
				op[o].amp = Gain(0.495f * (OP[o].OUTPUTLEVEL - 99.f));
			}
		
			adsr(0,0.1,1,1);
		}
		
		event off(Velocity v){
			adsr.release();
		}

		// Apply processing (called once per sample)
		void process() {						

			(op[1] >> op[0]) + (op[3] >> op[2]) + (op[5] >> op[4]) >> out;
								
			out *= adsr++ * 0.25;
			if(adsr.finished())
				stop();
		}
		
		void load(const DX::Patch& patch){
			Patch::operator=(patch);
		}
	};

	// Initialise plugin (called once at startup)
	DX7() {		
		controls = { 
			// UI controls and parameters
			Dial("Level", 0, .99, .75),
			Toggle("OPTION", false),
		};

		presets = {
			
		};

		notes.add<MyNote>(32);
		load(DX::PATCH);
	}
	
	void load(const DX::Patch& patch){
		for(int n=0; n<32; n++)
			((MyNote*)notes[n])->load(patch);
	}
};