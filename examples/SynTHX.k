
#include <klang.h>
using namespace klang::optimised;

#define THX_MODE 2

struct SynTHX : Stereo::Synth {
	template<int HARMONICS>
	struct Additive : Stereo::Oscillator {
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
				if(fabs(seed) < 0.0003)				// ensure minimal phasing
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
		
		Additive<4> notes[11];
		ADSR adsr;

		// Note On
		event on(Pitch pitch, Amplitude velocity) { 
			const float* pitches = controls[3] ? minor : major;
			for(int n=0; n<11; n++){
				notes[n].set(Pitch(pitches[n] - 26 + pitch) -> Frequency, controls[2], controls[1]);
				notes[n].set(Pitch(pitches[n] - 26 + pitch) -> Frequency, controls[2], controls[1]);
			}
			adsr(controls[0], 0, 1, 2.0);
		}

		event off(Amplitude velocity){
			adsr.release();
		}

		// Apply processing (called once per sample)
		void process() { }
		
		bool process(stereo::buffer buffer) {
			for(int n=0; n<11; n++){
				notes[n].set(controls[2], controls[1]);
				notes[n].process(buffer);
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
	SynTHX() {		
		controls = { 
			// UI controls and parameters
			Dial("Attack", 0, 5, 0.5),
			Dial("Detune", 0, 1, 0.5),
			Slider("Pitch", 0, 1, 0.6),
			Toggle("Minor", 1),
			Toggle("Hyper"),
		};
			
		controls[2].size = { 50, 150, 300, 20 };

		notes.add<MyNote>(32);
	}
	
	void process() {
		constexpr float gain = 0.5f;//controls[4] ? 0.03f : 0.06f;
		constexpr float _tanh = 0.761594155956f; // 1/tanh(1)
		tanh(in.l * gain) * _tanh >> out.l;
		tanh(in.r * gain) * _tanh >> out.r;
	}
};