
#include <klang.h>

using namespace klang::optimised;

struct THX : Stereo::Synth {
	template<int HARMONICS>
	struct Additive : Stereo::Oscillator {
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
				env = { { 0,  0.20f * random(1 - detune * 2,  	 1 + detune * 2) }, 
						{ 4,  0.10f * random(1 - detune,  		 1 + detune) }, 
						{ 6,  0.125f * random(1 - detune,  		 1 + detune) }, 
						{ 8,  0.250f * random(1 - detune * 0.25, 1 + detune * 0.25) }, 
						{ 10, 0.500f * random(1 - detune * 0.1,  1 + detune * 0.1) }, 
						{ 14, 1.000f * random(1 - 0.0003, 		 1 + 0.0003) } };
				osc(f,0,1);
			}
			
			void process(){
				osc(shape(env.at(time)) * scale + offset) >> out;
			}
		};
	
		Partial partial[HARMONICS][3];
		
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
	
		void process() { 
//			out = 0;
//			if(frequency < 440.f){
//				for(int p=0; p<HARMONICS; p++){
//					(partial[p][0].right ? out.r : out.l) += partial[p][0];
//					(partial[p][1].right ? out.r : out.l) += partial[p][1];
//				}
//			} else {
//				for(int p=0; p<HARMONICS; p++){
//					(partial[p][0].right ? out.r : out.l) += partial[p][0]; 
//					(partial[p][1].right ? out.r : out.l) += partial[p][1]; 
//					(partial[p][2].right ? out.r : out.l) += partial[p][2];
//				}
//			}
//			out *= 0.25f;
		}
		
		void process2(mono::buffer buffer) {
			for(int p=0; p<HARMONICS; p++){
				buffer.rewind();
				while(buffer)
					buffer++ += (partial[p][0] + partial[p][1]) * 0.25f;
			}
		}
		
		void process3(mono::buffer buffer) {
			for(int p=0; p<HARMONICS; p++){
				buffer.rewind();
				while(buffer)
					buffer++ += (partial[p][0] + partial[p][1] + partial[p][2]) * 0.25f;
			}
		}

		
		template<int PARTIALS>
		void partials(stereo::buffer buffer){
			for(int p=0; p<HARMONICS; p++){
				buffer.rewind();
				while(buffer){
					buffer.channel(partial[p][0].right) += partial[p][0] * 0.25f;
					if constexpr (PARTIALS >= 2)
						buffer.channel(partial[p][1].right) += partial[p][1] * 0.25f;
					if constexpr (PARTIALS >= 3)
						buffer.channel(partial[p][2].right) += partial[p][2] * 0.25f;
					buffer++;
				}
			}
		}
		void process(mono::buffer buffer) {
			(frequency < 440.f) ? process2(buffer) : process3(buffer);
		}
		
		void process(stereo::buffer buffer) {
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
		
		Additive<4> notes[2][11];
		ADSR adsr;

		// Note On
		event on(Pitch pitch, Amplitude velocity) { 
			const float* pitches = controls[3] ? minor : major;
			for(int n=0; n<11; n++){
				notes[0][n].set(controls[2]);
				notes[0][n].set(Pitch(pitches[n] - 26 + pitch) > Type::Frequency, controls[1]);
				notes[1][n].set(controls[2]);
				notes[1][n].set(Pitch(pitches[n] - 26 + pitch) > Type::Frequency, controls[1]);
			}
			adsr(controls[0], 0, 1, 2.0);
		}

		event off(Amplitude velocity){
			adsr.release();
		}

		// Apply processing (called once per sample)
		void process() { }
		
		bool process(stereo::buffer buffer) {
			if(controls[4]){
				for(int n=0; n<11; n++){
					notes[0][n].set(controls[2]);
					notes[0][n].process(buffer.left);
					notes[1][n].set(controls[2]);
					notes[1][n].process(buffer.right);
				}
			}else{
				for(int n=0; n<11; n++){
					notes[0][n].set(controls[2]);
					notes[0][n].process(buffer);
				}
			}
			
			buffer.rewind();
			while(buffer)
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
			Dial("Attack", 0, 10, 10),
			Dial("Detune", 0, 0.5, 0.5),
			Slider("Chaos - Order", 0, 14, 0),
			Toggle("Minor"),
			Toggle("Hyperdrive"),
		};
			
		controls(2).size = { 50, 150, 300, 10 };

		notes.add<MyNote>(32);
	}
	
	void process() {
		const float gain = controls[4] ? 0.03f : 0.06f;
		constexpr float _tanh = 0.761594155956f; // 1/tanh(1)
		tanh(in.l * gain) * _tanh >> out.l;
		tanh(in.r * gain) * _tanh >> out.r;
	}
};