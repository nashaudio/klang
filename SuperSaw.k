
#include <klang.h>
using namespace klang::optimised;

struct SuperSaw : Synth {

	struct MyNote : public Note {
		Saw osc[7];
		ADSR adsr;
		Sine lfo;

		event on(Pitch pitch, Amplitude velocity) { 
			const param f = pitch -> Frequency;
			const param detune = 0.01 * controls[2] * f;
			
			for(int s=0; s<7; s++)
				osc[s](f + (s-3) * detune * random(0.999, 1.001), 0, controls[1]);
			adsr(controls[0], 0.25, 1.0, 0.5);
		}

		event off(Amplitude velocity){
			adsr.release();
		}

		void process() {
			out = 0;
			for(int s=0; s<7; s++)
				out += osc[s] / 7;
			
			out *= adsr++;							
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
			Dial("Detune"),

		};

		notes.add<MyNote>(32);
	}
}; 