
#include <klang.h>

using namespace klang::optimised;

struct MySynth : Synth {

	struct MyNote : Note {
	
		// Declare components / variables
		//Wavetable sine = Sine();
		Sine sine;
		ADSR adsr;
		
		// Note On
		event on(Pitch p, Velocity v) { 		
			param f = p > Type::Frequency;
			sine.set(f,0);
			
			adsr(0.005,0,1,1);
		}
		
		event off(Velocity v){
			adsr.release();
		}

		// Apply processing (called once per sample)
		void process() {						
								
			out = sine * adsr++ * 0.1;
			if(adsr.finished())
				stop();
		}
	};
	
	// Initialise plugin (called once at startup)
	MySynth() {		
		controls = { 
			// UI controls and parameters
		};

		presets = {
			
		};

		notes.add<MyNote>(32);
	}
}; 