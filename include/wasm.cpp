#include <stdio.h>
#include <cstdlib>
#include <math.h>
#include <wajic.h>

#include "klang.h"

#define SYNTH THX
#define CHANNELS 2
using namespace klang::optimised;

//#define SYNTH Guitar
//using namespace klang::optimised;

float cubic(float x) { return x * x * x; }

////////////////////////// Guitar //////////////////////////

struct Guitar : Synth {
	float playingNotes = 0;
	LPF filter;
	HPF dcfilter;
	const float normalise;
	
	struct MyNote : Note {
		Delay<44100> strum;
		Guitar* synth;
	
		struct Excitation : Generator {
			Envelope impulse;
			Delay<44100> delay;
			LPF filter[2];
			Noise noise;
			
			void process() {
				const signal exc = impulse++ * noise;
				exc - (exc >> delay) >> filter[0]  >> out;
			}
		} pluck;
			
		struct Resonator : Modifier {
			Delay<44100> delay;
			LPF filter[2];
			param gain;
			
			void process() {
				(in + delay * gain) >> filter[0]  >> out >> delay;
			}
		} string;
		
		Resonator string2;
		
		ADSR adsr;
		
		// Note On
		event on(Pitch pitch, Amplitude velocity) {	
			param f = (pitch - 12) -> Frequency;		
			param delay = 1/f * klang::fs - 1.5;
					
			synth = (Guitar*)getSynth();
			strum.clear();
			strum.set(synth->playingNotes * random(0.004, 0.01) * klang::fs);	
			synth->playingNotes++;
			
			velocity = velocity * random(0.99, 1.01);			
			
			param dampen = 0;
				 if(pitch  > 64) dampen = 0.2;
			else if(pitch == 64) dampen = 0.1;
			else if(pitch  > 59) dampen = 0.15;
			else if(pitch == 59) dampen = 0.05;
			else if(pitch  > 55) dampen = 0.25;
			else if(pitch == 55) dampen = 0.25;
			else if(pitch  > 50) dampen = 0.3;
			else if(pitch == 50) dampen = 0.2;
			else if(pitch  > 45) dampen = 0.45;
			else if(pitch == 45) dampen = 0.4;
			else if(pitch == 40) dampen = 0.5;
			else 				 dampen = 0.55;
			
			dampen = dampen - controls[1];
			if(dampen < 0) dampen = 0;
			if(dampen > 1) dampen = 1;
					
			param finger = controls[2];// + 0.0125 * (pitch - 36) / 12;
			if(finger < 0) finger = 0;
			if(finger > 1) finger = 1;
			
			param resonance = controls[0];// + 0.002 * pitch / 12;
			if(resonance > 1.0) resonance = 1.0;
			
			string.delay.clear();
			string.delay.set(delay);
			string.filter[0].set(1 - dampen);
			string.filter[1].set(1 - dampen);
			string.gain = resonance;
			
			pluck.delay.clear();
			pluck.delay.set(finger * delay);
			pluck.filter[0].set(cubic(controls[3]));
			pluck.filter[1].set(cubic(controls[3]));
			pluck.impulse = { { 0.000, 0 }, { 0.001, velocity }, { 0.003, -velocity }, { 0.004, 0 } };

			adsr(0,0,1,3);
		}
		
		event off(Amplitude velocity){
			string.gain = 0.992;
			synth->playingNotes--;
			adsr.release();
		}
		
		// Apply processing (called once per sample)
		void process() {
			pluck >> strum >> string >> out;
					
			out *= 0.3;
			adsr++;
			if(adsr.finished())
				stop();
		}
	};
	
	// Initialise plugin (called once at startup)
	Guitar() : normalise(0.25f/tanh(3)) {
		controls = {
			Dial("Resonance", 0.97, 0.999, 0.994),
			Dial("Brightness", -0.1, 0.1, 0.025),
			Dial("Fingering", 0, 0.25, 0.14),
			Dial("Pluck", 0.5, 1.0, 0.6),
			Dial("Overdrive", 1.0, 11.0, 1.0),
		};
		
		presets = {
			{ "Classical", { 0.994, 0.025, 0.14, 0.6, 1 } },
		};
		
		notes.add<MyNote>(32);
	}
	
	void process() {
		in *= 1 + controls[4];
		if(in > 1)
			out = 1;
		else if(in < -1)
			out = -1;
		else
			out = in;
		
		tanh(out * 3) * normalise >> out;
		if(controls[4] < 2)
			out *= 1 + (2 - controls[4]);
	}
};

////////////////////////// Banjo ///////////////////////////

struct Banjo : Synth {
	float playingNotes = 0;
	LPF filter;
	HPF dcfilter;
	const float normalise;
	
	struct MyNote : Note {
		Delay<44100> strum;
		Banjo* synth;
	
		struct Excitation : Generator {
			Envelope impulse;
			Delay<44100> delay;
			Noise noise;
			
			void process() {
				const signal exc = impulse++ * noise;
				exc - (exc >> delay) >> out;
			}
		} pluck;
			
		struct Resonator : Modifier {
			Delay<44100> delay;
			LPF filter;
			param gain;
			
			void process() {
				(in + delay * gain) >> filter  >> out >> delay;
			}
		} string;
		
		Resonator string2;
		
		ADSR adsr;
		
		// Note On
		event on(Pitch pitch, Amplitude velocity) {	
			param f = (pitch - 12) -> Frequency;
			param delay = 1/f * klang::fs;// - 1.5;
					
			synth = (Banjo*)getSynth();
			strum.clear();
			strum.set(synth->playingNotes * random(0.004, 0.01) * klang::fs);	
			synth->playingNotes++;
			
			velocity = velocity * random(0.99, 1.01);			
			
			param dampen = 0;
				 if(pitch  > 64) dampen = 0.2;
			else if(pitch == 64) dampen = 0.1;
			else if(pitch  > 59) dampen = 0.15;
			else if(pitch == 59) dampen = 0.05;
			else if(pitch  > 55) dampen = 0.25;
			else if(pitch == 55) dampen = 0.25;
			else if(pitch  > 50) dampen = 0.3;
			else if(pitch == 50) dampen = 0.2;
			else if(pitch  > 45) dampen = 0.45;
			else if(pitch == 45) dampen = 0.4;
			else if(pitch == 40) dampen = 0.5;
			else 				 dampen = 0.55;
			
			dampen = dampen - controls[1];
			if(dampen < 0) dampen = 0;
			if(dampen > 1) dampen = 1;
					
			param finger = controls[2];// + 0.0125 * (pitch - 36) / 12;
			if(finger < 0) finger = 0;
			if(finger > 1) finger = 1;
			
			param resonance = controls[0];// + 0.002 * pitch / 12;
			if(resonance > 1.0) resonance = 1.0;
			
			string.delay.clear();
			string.delay.set(delay);
			string.filter.set(1 - dampen);
			string.gain = resonance;
			
			string2.delay.clear();
			string2.delay.set(delay * random(0.996, 0.998));
			string2.filter.set(1 - dampen);
			string2.gain = resonance;
			
			pluck.delay.clear();
			pluck.delay.set(finger * delay);
			pluck.impulse = { { 0.000, velocity }, { 0.001, velocity }, { 0.002, 0 } };

			adsr(0,0,1,3);
		}
		
		event off(Amplitude velocity){
			string.gain = 0.980;
			synth->playingNotes--;
			adsr.release();
		}
		
		// Apply processing (called once per sample)
		void process() {
			const signal excitation = pluck >> strum;
		
			((excitation >> string) + (excitation >> string2)) * 0.5 >> out;
					
			out *= 0.3; adsr++;
			if(adsr.finished())
				stop();
		}
	};
	
	// Initialise plugin (called once at startup)
	Banjo() : normalise(0.25f/tanh(3)) {
		controls = {
			Dial("Resonance", 0.97, 0.999, 0.99),
			Dial("Brightness", -0.1, 0.1, 0.0),
			Dial("Fingering", 0, 0.25, 0.02),
		};
			
		notes.add<MyNote>(32);
	}
	
	void process() {
		in *= 1 + controls[4];
		if(in > 1)
			out = 1;
		else if(in < -1)
			out = -1;
		else
			out = in;
		
		tanh(out * 3) * normalise >> out;
		if(controls[4] < 2)
			out *= 1 + (2 - controls[4]);
	}
};

///////////////////////// SuperSaw /////////////////////////

struct SuperSaw : Synth {
	
    struct MyNote : Note {
        Saw osc[7];
        ADSR adsr;

        // Note On
        event on(Pitch pitch, Amplitude velocity) {
            param f = pitch -> Frequency;

            param detune = f * controls[2] * 0.01;
            for (int o = 0; o < 7; o++)
                osc[o](f + detune * (o - 3), 0, controls[1]);
            adsr(controls[0], 0.25, 0.5, 0.5);
        }

        event off(Amplitude velocity) {
            adsr.release();
        }

        // Apply processing (called once per sample)
        void process() {
            out = 0;
            for (int o = 0; o < 7; o++)
                out += osc[o] / 7;

            out *= 1 + controls[2] + controls[1];
            out *= (adsr++) * 0.1;
            if (adsr.finished())
                stop();
        }
    };

    // Initialise plugin (called once at startup)
    SuperSaw() {
        controls = {
            // UI controls and parameters
            Dial("Attack"),
            Dial("Saw-Tri"),
            Dial("Detune")
        };

        controls[2].set(0.6);

        notes.add<MyNote>(32);
    }
};

struct StereoTest : Stereo::Synth {
	
	struct MyNote : Stereo::Note {
		Saw osc;
		Sine lfo;
		
		// Note On
		event on(Pitch pitch, Amplitude velocity) {
			param f = pitch -> Frequency;
			osc(f);
			lfo(1);
		}
		
		// Apply processing (called once per sample)
		void process() {
			signal mono = osc;
			signal pan = lfo;
			mono * pan >> out.l;
			mono * (1-pan) >> out.r;
		}
	};
	
	// Initialise plugin (called once at startup)
	StereoTest() {
		notes.add<MyNote>(32);
	}
};

/////////////////////////// DX7 ////////////////////////////

enum { Ratio, Fixed }; // OSC_MODE (+ SYNC?!)
constexpr int NA = 0;
typedef unsigned char uint8;

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
		TUB_BELLS, E_PIANO_1, /*E_ORGAN_1,*/ HARPSICH_1/*, STRINGS_1*/
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
			y = (x < 20) ? (int[20]) { 0, 5, 9, 13, 17, 20, 23, 25, 27, 29, 31, 33, 35, 37, 39, 41, 42, 43, 45, 46 } [x] : 28 + x;
		};

		const Table<int, 128> Actual = FUNCTION(int) {
			y = (x < 5) ? 2 * x
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
			}
			else { 						// exponential
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
				return ScaleCurve((1 + split) / 3, OP.RIGHTDEPTH, OP.RIGHTCURVE);
			else
				return ScaleCurve((1 - split) / 3, OP.LEFTDEPTH, OP.LEFTCURVE);
		}

		static int ScaleVelocity(int velocity, int sensitivity) {
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

		int getOutputLevel(int pitch, int velocity, const Patch::Op& OP) {
			int outlevel = Output[OP.OUTPUTLEVEL];
			outlevel += ScaleLevel(pitch, OP);
			outlevel = std::min(127, outlevel);
			outlevel = outlevel * 32;
			outlevel += ScaleVelocity(velocity, OP.KEYVELOCITY);
			outlevel = max(0, outlevel);								// up to 4256              
			return outlevel;
		}

		int getTargetLevel(int envlevel, int outlevel) {
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
				constexpr bool mask[4][8] = { {0,1,0,1,0,1,0,1},
										   {0,1,0,1,0,1,1,1},
										   {0,1,1,1,0,1,1,1},
										   {0,1,1,1,1,1,1,1} };
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

			Ramp(const Operator* op) : op(*op), sr_multiplier(44100.f / klang::fs) { }

			void setRate(float rate) override {
				if (target != out) {
					rising = target > out;

					const int rate_scaling = (op.OP->RATESCALING * min(31, max(0, op.pitch / 3 - 7))) >> 3;
					qrate = std::min(63, int(rate * 41.f / 64) + rate_scaling);
					shift = (qrate >> 2) - 11;
				}
			}

			signal operator++(int) override {
				if (step(i++)) {
					rate = (rising ? attack(out) : decay()) * sr_multiplier;
					Linear::operator++(1);
				}
				return out;
			}
		};

		void init(Pitch p, Velocity v, const Patch::Op& OP) {
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
			env.setLoop(3, 3);
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
		void (MyNote::* algorithm)(void);

		// Note On
		event on(Pitch p, Velocity v) {
			preset = 0;
			for(int b=0; b < 3; b++){
				if(controls[b])
					preset = b;
			}
			fc = (p - 12) -> Frequency;		

			const auto& PATCH = DX::Presets[preset];
			const auto* OP = PATCH.OP;
			for (int o = 0; o < 6; o++)
				op[o].init(p, v, OP[o]);

			adsr(0.005, 0, 1, 1);

			switch (PATCH.ALGORITHM) {
			case 2:  algorithm = &MyNote::algorithm2; break;
			case 5:  algorithm = &MyNote::algorithm5; break;
			case 32: algorithm = &MyNote::algorithm32; break;
			};
		}

		event off(Velocity v) {
			for (int o = 0; o < 6; o++)
				op[o].release();
			adsr.release();
		}

		void algorithm2() { ((op[1] << op[1]) >> op[0]) + (op[5] >> op[4] >> op[3] >> op[2]) >> out; }
		void algorithm5() { (op[1] >> op[0]) + (op[3] >> op[2]) + (op[5] >> op[4]) >> out; }
		void algorithm32() { (op[1] + op[0] + op[3] + op[2] + op[5] + op[4]) >> out; }

		bool finished() {
			if (adsr.finished())
				return true;
			for (int o = 0; o < 6; o++)
				if (!op[o].env.finished())
					return false;
			return true;
		}

		// Apply processing (called once per sample)
		void process() {
			(this->*algorithm)();

			out *= adsr++ * 0.1f;
			if (finished())
				stop();
		}
	};

	void onControl(int index, float value) override {
		for(int b=0; b<3; b++)
			controls[b].set(b == index ? 1.f : 0.f);
	}

	// Initialise plugin (called once at startup)
	DX7() {
		controls = {
			Toggle("TUB BELLS"),
			Toggle("E.PIANO 1"),
			Toggle("HARPSICH 1"),
			//Menu("Preset", "TUB BELLS", "E.PIANO 1", "E.ORGAN 1", "HARPSICH 1", "STRINGS 1")
		};
		controls[0].size = { 50,50,100,25 };

		notes.add<MyNote>(32);
	}
};

#define THX_MODE 2

struct THX : Stereo::Synth {
	template<int HARMONICS>
	struct Additive : Stereo::Oscillator {
#if THX_MODE == 1
		struct Partial : Mono::Oscillator {
			Saw osc;
			Envelope env;
			param offset, scale, time;
			bool right = true;
			
			static signal shape(signal x){ return 2 * x - (x * x); }
			
			void set(param position) {
				time = position;
			}
			
			void set(param f, param detune) {
				offset = 0.0625f * f;
				scale = 0.9375f * f;
				right = random(0.f, 1.f) > 0.5;
				env = { { 0,  0.20f * random(1 - detune * 2,  	 1 + detune * 2) }, 	// low pitch - high detune
						{ 4,  0.10f * random(1 - detune,  		 1 + detune) }, 
						{ 6,  0.125f * random(1 - detune,  		 1 + detune) }, 		
						{ 8,  0.250f * random(1 - detune * 0.25, 1 + detune * 0.25) },  // low pitch - low detune
						{ 10, 0.500f * random(1 - detune * 0.1,  1 + detune * 0.1) }, 
						{ 14, 1.000f * random(1 - 0.0003, 		 1 + 0.0003) } };
				osc(f,0);
			}
			
			void process(){
				osc(shape(env.at(time)) * scale + offset) >> out;
			}
		};

		void set(param position) {
			for(int p=0; p<HARMONICS; p++){
				partial[p][0].set(position);
				partial[p][1].set(position);
				partial[p][2].set(position);
			}
		}
		
		void set(param f0, param detune){
			frequency = f0;
			for(int p=0; p<HARMONICS; p++){
				const float f = f0 * (1 + p);
				partial[p][0].set(f, detune);
				partial[p][1].set(f, detune);
				partial[p][2].set(f, detune);
			}
		}
#else
		struct Partial : Mono::Oscillator {
			Saw osc;
			param f0, range, seed;
			bool right;
			const Envelope transposition = { 	{	0.00,	0.0625	},  // -4ve
												{	0.08,	0.078125},  // (+Maj3)
												{	0.17,	0.09375	},	// (+Perf5)
												{	0.25,	0.125	},  // -3ve
												{	0.33,	0.15625	},	//
												{	0.41,	0.1875	},	//
												{	0.50,	0.25	},  // -2ve
												{	0.58,	0.3125	},	//
												{	0.66,	0.375	},  //
												{	0.75,	0.5		},  // -1ve
												{	0.83,	0.625	},  //
												{	0.91,	0.75	},	//
												{	1.00,	1		} };// f0

			const Envelope detunes = { 			{ 0,   0.0f   },  	// no detune (organ)
												{ 0.6, 0.015f },    // strings
												{ 0.7, 0.05f  },    // hell strings
												{ 0.8, 0.1f  },     // 
												{ 0.9, 0.5f  },     // chaos
												{ 1.0, 2.00f  } }; 	// noise
			
			static signal shape(signal x){ return 2 * x - (x * x); }
			
			void set(param transpose, param detune) {
				const float detuned = transposition.at(transpose) * (1.f + seed * detunes.at(detune));
				float f = f0 + detuned * range;
				if(f < 0) 
					f = -f;
				else if(f >= klang::fs.nyquist)
					f = f * 0.5f;
				osc(f);
			}
			
			// note start
			void set(param f, param transpose, param detune) {
				f0 = 0 * f; 					// = lowest pitch, -4 octaves below f
				range = 1 * f;  				// = highest pitch, f
				right = random(0.f, 1.f) > 0.5;		// random stereo channel
				seed = random(-1.f, 1.f);			// random detune seed (persists through note)
				if(abs(seed) < 0.0003)				// ensure minimal phasing
					seed = random(0.f, 1.f) > 0.5 ? 0.0003 : -0.0003;

				set(transpose, detune);
				osc.reset();

				// env = { { 0,  0.20f * random(1 - detune * 2,  	 1 + detune * 2) }, 	// low pitch - high detune
				// 		{ 4,  0.10f * random(1 - detune,  		 1 + detune) }, 
				// 		{ 6,  0.125f * random(1 - detune,  		 1 + detune) }, 		
				// 		{ 8,  0.250f * random(1 - detune * 0.25, 1 + detune * 0.25) },  // low pitch - low detune
				// 		{ 10, 0.500f * random(1 - detune * 0.1,  1 + detune * 0.1) }, 
				// 		{ 14, 1.000f * random(1 - 0.0003, 		 1 + 0.0003) } };
				// osc(f,0);
			}
			
			void process(){
				osc >> out;
			}
		};

		Partial partial[HARMONICS][3];

		void set(param transpose, param detune) {
			for(int p=0; p<HARMONICS; p++){
				partial[p][0].set(transpose, detune);
				partial[p][1].set(transpose, detune);
				partial[p][2].set(transpose, detune);
			}
		}
		
		void set(param f0, param transpose, param detune){
			frequency = f0;
			for(int p=0; p<HARMONICS; p++){
				const float f = f0 * (1 + p);
				partial[p][0].set(f, transpose, detune);
				partial[p][1].set(f, transpose, detune);
				partial[p][2].set(f, transpose, detune);
			}
		}
#endif
	
		void process() { } // dummy per-sample processor (not used)
		
		template<int PARTIALS>
		void partials(mono::buffer buffer){
			for(int p=0; p<HARMONICS; p++){
				buffer.rewind();
				while(!buffer.finished()){
					if constexpr (PARTIALS >= 3)
						buffer += (partial[p][0] + partial[p][1] + partial[p][1]) * 0.25f;
					else
						buffer += (partial[p][0] + partial[p][1]) * 0.25f;
					buffer++;
				}
			}
		}
		
		template<int PARTIALS>
		void partials(stereo::buffer buffer){
			for(int p=0; p<HARMONICS; p++){
				buffer.rewind();
				while(!buffer.finished()){
					buffer.channel(partial[p][0].right) += partial[p][0] * 0.25f;
					if constexpr (PARTIALS >= 2)
						buffer.channel(partial[p][1].right) += partial[p][1] * 0.25f;
					if constexpr (PARTIALS >= 3)
						buffer.channel(partial[p][2].right) += partial[p][2] * 0.25f;
					buffer++;
				}
			}
		}

		template<typename BUFFER>
		void process(BUFFER buffer) {
			(frequency < 440.f) ? partials<2>(buffer) : partials<3>(buffer);
		}
	};
	
	static signal cubic(signal x) { return x*x; }

	struct MyNote : public Stereo::Note {
	
		const float minor[11] = {
			26, 33, 
			38, 45, 
			50, 53, 57, 
			64, 67, 71,
			76
		};
		
		const float major[11] = {
			26, 33, 
			38, 45, 
			50, 54, 57, 
			64, 68, 71,
			76
		};
		
#if THX_MODE == 1
		Additive<4> notes[2][11];
#else
		Additive<4> notes[11];
#endif
		ADSR adsr;

		// Note On
		event on(Pitch pitch, Amplitude velocity) { 
			const float* pitches = controls[3] ? minor : major;
			for(int n=0; n<11; n++){
#if THX_MODE == 1
				notes[0][n].set();
				notes[0][n].set(Pitch(pitches[n] - 26 + pitch) -> Frequency, controls[1]);
				notes[1][n].set(controls[2]);
				notes[1][n].set(Pitch(pitches[n] - 26 + pitch) -> Frequency, controls[1]);			
#else
				notes[n].set(Pitch(pitches[n] - 26 + pitch) -> Frequency, controls[2], controls[1]);
				notes[n].set(Pitch(pitches[n] - 26 + pitch) -> Frequency, controls[2], controls[1]);
#endif
			}
			adsr(controls[0], 0, 1, 2.0);
		}

		event off(Amplitude velocity){
			adsr.release();
		}

		// Apply processing (called once per sample)
		void process() { }
		
		bool process(stereo::buffer buffer) {
			// if(controls[4]){
			// 	for(int n=0; n<11; n++){
			// 		notes[0][n].set(controls[2]);
			// 		notes[0][n].process(buffer.left);
			// 		notes[1][n].set(controls[2]);
			// 		notes[1][n].process(buffer.right);
			// 	}
			// }else{
				for(int n=0; n<11; n++){
					notes[n].set(controls[2], controls[1]);
					notes[n].process(buffer);
			// }
			}
			
			buffer.rewind();
			while(!buffer.finished())
				buffer++ *= adsr++;			
			if (adsr.finished())
				stop();
			return !finished();
		}
	};

	// Initialise plugin (called once at startup)
	THX() {		
		controls = { 
			// UI controls and parameters
			Dial("Attack", 0, 5, 0.5),
			Dial("Detune", 0, 1, 0.5),
			Dial("Pitch", 0, 1, 0),
			Toggle("Minor"),
			//Toggle("Hyperdrive"),
		};
			
		controls[2].size = { 50, 150, 300, 10 };

		notes.add<MyNote>(2);
	}
	
	void process() {
		constexpr float gain = 0.5f;//controls[4] ? 0.03f : 0.06f;
		constexpr float _tanh = 0.761594155956f; // 1/tanh(1)
		tanh(in.l * gain) * _tanh >> out.l;
		tanh(in.r * gain) * _tanh >> out.r;
	}
};

struct KSynth : public SYNTH {
    float voice_buffer[16384] = { 0 };

	struct MIDI {
		struct Event {
			unsigned int time = 0;
			uint8 status = 0;
			uint8 byte1 = 0;
			uint8 byte2 = 0;
			uint8 reserved = 0;
		};
		struct Buffer : public Array<Event, 1024> {
			unsigned int offset = 0;
			void clear(){
				Array::clear();
				offset = 0;
			}
			void add(uint8 status, uint8 byte1, uint8 byte2, const float time = 0){
				Array::add( { time == 0 ? 0 : ((unsigned int)(time * klang::fs) - offset), status, byte1, byte2 } );
			}
		};
	};

	MIDI::Buffer _midi[2];
	MIDI::Buffer *playing = &_midi[0], *queue = &_midi[1];

	void process(const MIDI::Event& event){
		switch(event.status & 0xF0){
		case 0x90: // Note On
		{	const int n = notes.assign();
			Note* note = notes[n];
			note->start(event.byte1, event.byte2 / 127.f);
		}	break;
		case 0x80: // Note Off
			for (int i = 0; i < notes.count; i++) {
				Note* note = notes[i];
				if (note->stage != Note::Off && note->pitch == event.byte1)
					note->release(event.byte2 / 127.f);
			} break;
		}
	}
	
    void noteOn(int pitch, int velocity, float time = 0) {
		queue->add(0x90, pitch & 0x7F, velocity & 0x7F, time);
	}

    void noteOff(int pitch, int velocity, float time = 0) {
		queue->add(0x80, pitch & 0x7F, velocity & 0x7F, time);
	}

	void processNotes(buffer& buffer, int numSamples) {
#if CHANNELS == 1
		klang::buffer voice_buffer = { KSynth::voice_buffer, numSamples };

		for (int i = 0; i < notes.count; i++) {
			Note* note = notes[i];
			if (note->stage != Note::Off) {
                voice_buffer.clear(numSamples);
                voice_buffer.rewind();
				if (!note->process(voice_buffer))
					note->stop();
                for(int s=0; s<numSamples; s++)
					buffer[s] += voice_buffer[s]; // (downmix as required)
			}
		}
#else
		klang::buffer voice_buffer_l = { KSynth::voice_buffer, numSamples };
		klang::buffer voice_buffer_r = { &KSynth::voice_buffer[numSamples], numSamples };
		klang::stereo::buffer voice_buffer = { voice_buffer_l, voice_buffer_r };

		for (int i = 0; i < notes.count; i++) {
			Note* note = notes[i];
			if (note->stage != Note::Off) {
                voice_buffer.clear(numSamples);
                voice_buffer.rewind();
				if (!note->process(voice_buffer))
					note->stop();
                for(int s=0; s<numSamples; s++)
					buffer[s] += voice_buffer[s].mono(); // (downmix as required)
			}
		}
#endif
	}

	void processNotes(stereo::buffer& buffer, int numSamples) {
#if CHANNELS == 1
		processNotes(buffer.left, numSamples);
		buffer.right = buffer.left; // upmix from mono
#else
		klang::buffer voice_buffer_l = { KSynth::voice_buffer, numSamples };
		klang::buffer voice_buffer_r = { &KSynth::voice_buffer[numSamples], numSamples };
		klang::stereo::buffer voice_buffer = { voice_buffer_l, voice_buffer_r };

		for (int i = 0; i < notes.count; i++) {
			Note* note = notes[i];
			if (note->stage != Note::Off) {
                voice_buffer.clear(numSamples);
                voice_buffer.rewind();
				if (!note->process(voice_buffer))
					note->stop();
                for(int s=0; s<numSamples; s++){
					buffer[s] += voice_buffer[s];
				}
			}
		}
#endif
	}

    void process(float* buffer, int numSamples) {
		// process MIDI messages
		std::swap(playing, queue);
		for(int m=0; m<playing->count; m++){
			process((*playing)[m]);
		}

		klang::buffer main_buffer = { buffer, numSamples };
		main_buffer.clear(numSamples);
		processNotes(main_buffer, numSamples);
		main_buffer.rewind();
#if CHANNELS == 1
		SYNTH::Synth::process(main_buffer);
#endif
		playing->clear();
	}

	void process(float* left, float* right, int numSamples) {
		// process MIDI messages
		std::swap(playing, queue);
		for(int m=0; m<playing->count; m++){
			process((*playing)[m]);
		}

		klang::buffer main_buffer_l = { left, numSamples };
		klang::buffer main_buffer_r = { right, numSamples };
		klang::stereo::buffer main_buffer = { main_buffer_l, main_buffer_r };

		main_buffer.clear(numSamples);
		processNotes(main_buffer, numSamples);
		main_buffer.rewind();
#if CHANNELS == 2
		SYNTH::Synth::process(main_buffer);
#endif
		playing->clear();
	}

	float getParameter(int index) const {
		return controls[index];
	}

	void setParameter(int index, float value) {
		controls[index].set(std::clamp(value, controls[index].min, controls[index].max));
		onControl(index, controls[index]);
		if(controls[index].type == Control::BUTTON)
		 	controls[index].set(0.f); // release push button
	}

	void setParameters(float* parameters, int numParameters) {
		if (numParameters > 128)
			numParameters = 128;
		for (int i = 0; i < numParameters; i++){
			controls[i].set(std::clamp(parameters[i], controls[i].min, controls[i].max));
		}
	}

	const klang::Control* getControl(int control) const {
		return &controls.items[control];
	}

	int getControlCount() const {
		return controls.count;
	}
};

struct SharedBuffer {
	float* ptr;
	int length;
} _buffer;

WA_EXPORT(setBuffer) void setBuffer(float* buffer, int length) {
	_buffer.ptr = buffer;
	_buffer.length = length;
}

WA_EXPORT(synthCreate) KSynth* synthCreate(float sampleRate) {
    //::stk::Stk::setSampleRate(sampleRate);
    klang::fs = sampleRate;
	return new KSynth();
}

WA_EXPORT(synthDestroy) void synthDestroy(KSynth* synth) {
    printf("synthDestroy\n");
	delete synth;
}

WA_EXPORT(getBackground) void getBackground(KSynth* synth, void** data, int* size) {
    printf("getBackground\n");
    //*data = (void*)Background::data;
    //*size = Background::size;
}

WA_EXPORT(noteOnStart) void noteOnStart(KSynth* synth, int pitch, int velocity) {
    if (synth)
		synth->noteOn(pitch, velocity);
}

WA_EXPORT(noteOnStop) void noteOnStop(KSynth* synth, int pitch, int velocity) {
    if (synth)
		synth->noteOff(pitch, velocity);
}

WA_EXPORT(noteOnPitchWheel) void noteOnPitchWheel(KSynth* synth, void* note, int value) {
    printf("noteOnPitchWheel\n");
    //try {
    //    ((DSP::Note*)note)->onPitchWheel(value);
    //    return 0;
    //} CATCH_ALL(1)
}

WA_EXPORT(noteOnControlChange) void noteOnControlChange(KSynth* synth, int controller, int value) {
    printf("noteOnControlChange\n");
    //try {
    //    ((DSP::Note*)note)->onControlChange(controller, value);
    //    return 0;
    //} CATCH_ALL(1)
}
//
//WA_EXPORT(noteProcess) void noteProcess(void* note, float* outputBuffers, int numSamples, bool* shouldContinue = nullptr) {
//
//    float* pLeft = outputBuffers, * pRight = outputBuffers + numSamples;
//    for (unsigned int i = 0; i < numSamples; i++)
//    {
//        // Render 220 HZ sine wave at 25% volume into both channels
//        static size_t waveCount;
//        float wave = (((waveCount++) % 44100) / 44100.0f);
//        pLeft[i] = pRight[i] = sinf(2.0f * 3.14159f * 220.0f * wave) * 0.25f;
//    }
//    if(shouldContinue)
//        *shouldContinue = true;
//    //try {
//    //    bool bContinue = ((DSP::Note*)note)->process(outputBuffers, 2, numSamples);
//    //    if (shouldContinue) *shouldContinue = bContinue;
//    //    return 0;
//    //} CATCH_ALL(1)
//}

WA_EXPORT(synthProcess) void synthProcess(KSynth* synth, float* buffer, int numSamples) {
	if (synth)
		synth->process(buffer, numSamples);
}

WA_EXPORT(synthProcess2) void synthProcess2(KSynth* synth, float* buffer, int numSamples) {
	if (synth){
#if CHANNELS==1
		synth->process(buffer, numSamples);
#else
		synth->process(buffer, &buffer[numSamples], numSamples);
#endif
	}
}

WA_EXPORT(synthProcessShared) void synthProcessShared(KSynth* synth, int numSamples) {
	if (synth){
		numSamples = std::min(_buffer.length, numSamples);
#if CHANNELS==1
		synth->process(_buffer.ptr, numSamples);
		memcpy(&_buffer.ptr[numSamples], _buffer.ptr, sizeof(float) * numSamples);
#else
		synth->process(_buffer.ptr, &_buffer.ptr[numSamples], numSamples);
#endif
	}
}

WA_EXPORT(setParameter) void setParameter(KSynth* synth, int index, float value) {
	if (synth)
		synth->setParameter(index, value);
}

WA_EXPORT(getParameter) float getParameter(KSynth* synth, int index) {
	if (synth)
		return synth->getParameter(index);
	return 0.f;
}

WA_EXPORT(setParameters) void setParameters(KSynth* synth, float* parameters, int numParameters) {
	if (synth)
		synth->setParameters(parameters, numParameters);
}

WA_EXPORT(getControl) const klang::Control* getControl(KSynth* synth, int control) {
	if (synth)
		return synth->getControl(control);
	return nullptr;
}

WA_EXPORT(getControlCount) int getControlCount(KSynth* synth) {
	if (synth)
		return synth->getControlCount();
	return 0;
}
