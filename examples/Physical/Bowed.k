
#include <klang.h>
using namespace klang::optimised;

struct Bowed : Synth {

	struct BowedNote : public Note {
		
		struct Bow : public Generator {
			Noise noise;
			BPF filter;
			Envelope shape;
			param velocity;
									
			void start(param f){
				filter.set(f, 1);
				shape = { { 0,0.25 }, { 0.125, 1 }, { 0.5, 0.5 }, { 1.5, 1 } };
				shape.setLoop(1,3);
			}
			
			void stop() {
				shape.release(0.01);
			}
			
			signal bow(signal x){
				if(x > 0.95)
					return x - 0.6;
				else if(x < -0.95)
					return x + 0.6;
				else
					return 0;
			}
			
			void process(){
			 	bow(noise) * shape >> filter >> out;
			}
		} bow;
			
		struct Resonator : Modifier {	
			Delay<19200> delay;
			IIR filter;
			param gain;
			
			void set(param frequency, param resonance, param dampening){
				const param time = fs.f / frequency - 1;
				delay.clear();
				delay.set(time);
				filter.reset();
				filter.set(dampening);
				gain = resonance;
			}
			
			void process() {
				signal feedback = (delay >> filter) * gain;
				(in + feedback) >> out;
				delay << out;
			}
		} resonator;
			
		ADSR adsr;

		event on(Pitch pitch, Amplitude velocity) {
			const param f = pitch -> Frequency;
			
			bow.start(f);
			resonator.set(f, controls[0], controls[1]);
			
			adsr(0, 0, 1, 1);
		}

		event off(Amplitude velocity) {
			bow.stop();
			adsr.release();
		}

		void process() {
			bow >> resonator >> out;
			
			out *= adsr;
			if (adsr.finished())
				stop();
		}
	};

	// Initialise plugin (called once at startup)
	Bowed() {
		controls = {
			{ "Resonance", 
				Dial("Feedback", 0.9, 1, 1.000),
				Dial("Material", 0, 1, 0.95),
			},
		};

		notes.add<BowedNote>(32);
	}
};