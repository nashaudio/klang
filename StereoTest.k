
#include <klang.h>
using namespace klang::optimised;

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