
#include <klang.h>
using namespace klang::optimised;

signal hardclip(signal in){
	if(in > 1)
		return 1;
	else if(in < -1)
		return -1;
	return in;
}

struct Waveguide : Synth {

	struct WaveguideNote : public Note {
	
		struct Exciter : Generator {
			Noise noise;
			Envelope impulse;
			Delay<19200> delay;
			LPF filter;
			
			void set(param frequency, param position, param material) {
				const param time = fs.f / frequency - 2;
				delay.set(position * time);
				delay.clear();
				impulse = { { 0.000, 0.0 }, { 0.001, 1.0 }, { 0.003, -1.0 }, { 0.004, 0.0 } };
				filter.reset();
				filter.set(material * 20000);
			}
			
			void process() {
				signal excitation = impulse;
				excitation - (excitation >> delay) >> filter >> out;
			}
		} exciter;
			
		struct Resonator : Modifier {
			Delay<19200> delay;
			IIR<1> filter;
			param gain;
			
			void set(param frequency, param resonance, param dampening){
				const param time = fs.f / frequency - 2;
				delay.set(time);
				delay.clear();
				filter.reset();
				filter.set(dampening);
				gain = resonance;
			}
			
			void process() {
				signal feedback = (delay) * gain;
				(in + feedback) >> out;
				delay << hardclip(out);
			}
		} resonator;
			
		ADSR adsr;

		event on(Pitch pitch, Amplitude velocity) {
			const param f = pitch -> Frequency;
			
			exciter.set(f, controls[0], controls[1]);
			resonator.set(f, controls[2], controls[3]);
			
			adsr(0, 0, 1, .25);
		}

		event off(Amplitude velocity) {
			adsr.release();
		}

		void process() {
			exciter >> resonator >> out;
			
			out *= adsr;
			if (adsr.finished())
				stop();
		}
	};

	// Initialise plugin (called once at startup)
	Waveguide() {
		controls = {
			{ "Pluck", 
				Dial("Position", 0, 1, 0.25),
				Dial("Material", 0, 1, 0.5),
			},
			{ "String", 
				Dial("Resonance", 0.9, 1, 0.999),
				Dial("Material", 0, 1, 0.9),
			},
		};

		notes.add<WaveguideNote>(32);
	}
};