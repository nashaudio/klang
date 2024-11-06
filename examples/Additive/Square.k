
#include <klang.h>
using namespace klang::optimised;

struct Additive : Oscillator {
	Sine osc[32];
	
	void set(param frequency){
		for(int o=0; o<32; o++)
			osc[o].set(frequency * (o+1));
	}

	void process() {
		out = 0;
		for(int o=0; o<32; o++){
			int h = o + 1;
			if(h % 2 && osc[o].frequency < fs.nyquist)
				out += osc[o] / h;
		}
	}
};

struct Square : Synth {

	struct SquareNote : public Note {
		Additive osc;

		event on(Pitch pitch, Amplitude velocity) {
			osc.set(pitch -> Frequency);
		}

		void process() {
			osc >> out;
		}
	};

	Square() {
		notes.add<SquareNote>(32);
	}
};