
#include <klang.h>
using namespace klang::optimised;

struct Filter : Synth {

	struct FilterNote : public Note {
		Square osc;
		ADSR adsr;
		
		Envelope env;
		LPF filter;
		
		event on(Pitch pitch, Amplitude velocity) {
			param f = pitch -> Frequency;
			osc(f, 0);
			
			adsr(0,0,1,0.25);
			env = { { 0, f * 2 }, { 0.25, f * 10 }, { 2, f * 5 } };
			
			filter.reset();
		}

		event off(Amplitude velocity) {
			adsr.release();
		}

		void process() {
			filter.set(env++, 10);
			osc >> filter >> out;
			
			out *= adsr++;
			if (adsr.finished())
				stop();
		}
	};

	Filter() {
		notes.add<FilterNote>(32);
	}
};