
#include <klang.h>
using namespace klang::optimised;

struct Expression : Synth {

	struct ExpressionNote : public Note {
		Saw osc[3];
		ADSR adsr;
		
		Envelope env;
		LPF filter;
		
		param gain;
		Sine lfo;
		Envelope vibrato;
		param f0;
		Envelope rate;
		
		event on(Pitch pitch, Amplitude velocity) {
			f0 = pitch -> Frequency;
			
			lfo.reset();
			rate = { { 0, 0 }, { 0.5, 0 }, { 1.f, 5 }, { 2.f, 3 } };

			adsr(random(0.2,0.3), random(0.2,0.3), random(0.3,0.4), random(1.5f, 2.f));
			env = { { 0, max(1000,f0 * 1.5) }, { 0.5, f0 * 6 }, { 2.0, max(1000,f0 * 1.5) } };
			vibrato = { { 0, 0.005f }, { 0.25f, 0.001f }, { 0.5f, -0.01f }, { 1.f, 0.005f } };
			
			filter.reset();
		}

		event off(Amplitude velocity) {
			adsr.release();
		}

		void process() {
			lfo.set(rate++);
			osc[0](f0 * (1.0000f + lfo     * vibrato++));
			osc[1](f0 * (0.9997f + lfo.out * -vibrato.out));
			osc[2](f0 * (1.0004f + lfo.out * vibrato.out * 2));
			signal oscs = (osc[0] - osc[1] + osc[2]) * 0.35f;		
			
			filter.set(env++, .25f);
			(oscs * 0.075f) + (oscs >> filter) >> out;
			
			out *= adsr++;
			if (adsr.finished())
				stop();
		}
	};

	Expression() {
		controls = {
			// UI controls and parameters
		};

		notes.add<ExpressionNote>(32);
	}
};