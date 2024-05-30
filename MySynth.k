
#include <klang.h>

using namespace klang::optimised;

struct MySynth : Synth {

	struct MyNote2 : public Note {
		Saw osc[7];
		ADSR adsr;

		// Note On
		event on(Pitch pitch, Amplitude velocity) { 
			param f = pitch > Type::Frequency;
		
			param detune = f * controls[2] * 0.01;
			for(int o=0; o<7; o++)
				osc[o](f + (o-3) * detune, 0, controls[1]);
			adsr(controls[0], 0.25, 0.5, 0.5);
		}

		event off(Amplitude velocity){
			adsr.release();
		}

		// Apply processing (called once per sample)
		void process() {
			out = 0;
			for(int o=0; o<7; o++)
				out += osc[o] / 7;
			
			out *= 1 + controls[2] + controls[1];
			out *= (adsr++) * 0.25;							
			if (adsr.finished())
				stop();
				
			adsr >> debug;			
		}
	};

	// Initialise plugin (called once at startup)
	MySynth() {		
		controls = { 
			// UI controls and parameters
			Dial("Attack"),
			Dial("Saw-Tri"),
			Dial("Detune")
		};

		notes.add<MyNote2>(32);
	}
}; 