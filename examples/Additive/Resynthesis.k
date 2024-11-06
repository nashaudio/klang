
#include <klang.h>
using namespace klang::optimised;

// Measurements from Korg M1 Organ sample (at C2):
// ====================================================
//    65Hz,   131Hz,   196Hz,   263Hz,   327Hz,   393Hz
// -14.5dB, -14.2dB, -14.3dB, -50.0dB, -47.2dB, -24.5dB

const float FREQ[6] = { 65/65.f, 131/65.f, 195/65.f, 263/65.f, 327/65.f, 393/65.f }; // (normalised to f0)
const dB GAIN[6] = 	 { -14.5+12, -14.2+12, -14.3+12, -50.0+12, -47.2+12, -24.5+12 }; // (in dB)

struct Organ : Oscillator {
	Sine osc[6];
	
	void set(param frequency){
		for(int o=0; o<6; o++)
			osc[o].set(frequency * FREQ[o]);
	}

	void process() {
		out = 0;
		for(int o=0; o<6; o++)
			out += osc[o] * GAIN[o]->Amplitude;
	}
};


struct Resynthesis : Synth {

	struct ResynthesisNote : public Note {
		Organ osc;
		ADSR adsr;

		event on(Pitch pitch, Amplitude velocity) {
			osc.set(pitch -> Frequency);
			osc.reset();
			adsr(0.001, 0.1, 0.5, 0.1);
		}
		
		event off(Amplitude velocity){
			adsr.release();
		}

		void process() {
			osc * adsr >> out;
			if(adsr.finished())
				stop();
		}
	};

	Resynthesis() {
		notes.add<ResynthesisNote>(32);
	}
};