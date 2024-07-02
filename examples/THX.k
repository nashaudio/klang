
#include <klang.h>

using namespace klang::optimised;

Control* parameters = nullptr;

struct THX : Stereo::Synth {
	template<int HARMONICS>
	struct Additive : Mono::Oscillator {
		struct Partial : Mono::Oscillator {
			Saw osc;
			Envelope env;
			param offset, scale, time;
			
			static signal shape(signal x){ return 2 * x - (x * x); }
			
			void set(param position) override {
				time = position;
			}
			
			void set(param f, param detune){
				offset = 0.0625f * f;
				scale = 0.9375f * f;
				env = { { 0,  0.20f * random(1 - detune * 2,  	 1 + detune * 2) }, 
						{ 4,  0.10f * random(1 - detune,  		 1 + detune) }, 
						{ 6,  0.125f * random(1 - detune,  		 1 + detune) }, 
						{ 8,  0.250f * random(1 - detune * 0.25, 1 + detune * 0.25) }, 
						{ 10, 0.500f * random(1 - detune * 0.1,  1 + detune * 0.1) }, 
						{ 14, 1.000f * random(1 - 0.0003, 		 1 + 0.0003) } };
			}
			
			void process(){
				osc(shape(env.at(time)) * scale + offset) >> out;
			}
		};
	
		Partial partial[HARMONICS][3];
		
		void set(param position) override {
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
			out = 0;
			if(frequency < 440.f){
				for(int p=0; p<HARMONICS; p++){
					out += partial[p][0];
					out += partial[p][1];
				}
			} else {
				for(int p=0; p<HARMONICS; p++){
					out += partial[p][0];
					out += partial[p][1];
					out += partial[p][2];
				}
			}
			out *= 0.25f;
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
		
		void process(mono::buffer buffer) {
			if(frequency < 440.f){
				process2(buffer);
			}else{
				process3(buffer);
			}
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
			for(int n=0; n<11; n++){
				notes[0][n].set(controls[2]);
				notes[0][n].process(buffer.left);
				notes[1][n].set(controls[2]);
				notes[1][n].process(buffer.right);
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
		};
		
		parameters = controls.items;
		
		controls(2).size = { 50, 150, 300, 10 };

		notes.add<MyNote>(32);
	}
	
	void process() {
		constexpr float _tanh = 0.761594155956f; // 1/tanh(1)
		tanh(in.l * 0.03f) * _tanh >> out.l;
		tanh(in.r * 0.03f) * _tanh >> out.r;
	}
};