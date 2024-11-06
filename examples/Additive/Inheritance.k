
#include <klang.h>
using namespace klang::optimised;

struct Additive : Oscillator {
	Sine osc[32];
	
	void set(param frequency){
		for(int o=0; o<32; o++)
			osc[o].set(frequency * (o+1));
	}

	virtual void process() = 0;
};

struct AdditiveSaw : Additive {
	void process() {
		out = 0;
		for(int o=0; o<32; o++){
			int h = o + 1;
			if(osc[o].frequency < fs.nyquist)
				out += osc[o] / h;
		}
	}
};

struct AdditiveSquare : Additive {
	void process() {
		out = 0;
		for(int o=0; o<32; o++){
			int h = o + 1;
			if(h % 2 && osc[o].frequency < fs.nyquist)
				out += osc[o] / h;
		}
	}
};

struct AdditiveTriangle : Additive {
	void process() {
		out = 0;
		for(int o=0; o<32; o++){
			int h = o + 1;
			if(osc[o].frequency < fs.nyquist){
				if(h % 4 == 1)
					out += osc[o] / (h * h);// sqr(h);
				else if(h % 4 == 3)
					out -= osc[o] / (h * h);//sqr(h);
			}
		}
	}
};

struct Inheritance : Synth {

	struct InheritanceNote : public Note {
		AdditiveSaw saw;
		AdditiveSquare square;
		AdditiveTriangle triangle;
		
		Additive* osc = nullptr;

		event on(Pitch pitch, Amplitude velocity) {
			switch(int(controls[0])){
			case 0: osc = &saw; break;
			case 1: osc = &square; break;
			case 2: osc = &triangle; break;
			}
			osc->set(pitch -> Frequency);
		}

		void process() {
			*osc >> out;
		}
	};

	Inheritance() {
		controls = {
			{ "Group", 
				Menu("Waveform", "Saw", "Square", "Triangle"),
				Dial("A dial")
			}
		};
	
		notes.add<InheritanceNote>(32);
	}
};