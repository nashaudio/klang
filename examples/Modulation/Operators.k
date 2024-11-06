
#include <klang.h>
using namespace klang::optimised;

struct Operators : Synth {

	struct OperatorsNote : public Note {
		Operator<Sine> op1, op2, op3;
		ADSR adsr;
		param f0;
		HPF dcfilter;

		event on(Pitch pitch, Amplitude velocity) {
			f0 = pitch -> Frequency;
			op1.set(f0 * controls[0], 0);
			op1 = { { 0,1 }, { 1, 0.5 }, { 5, 0.125 } };
			op2.set(f0 * controls[2], 0);
			op2 = { { 0,1 }, { 2, 0.5 }, { 5, 0.25 } };
			op3.set(controls[4], 0);
			op3 = { { 0,1 }, { 3, 1 }, { 5, 0.5 } };
			dcfilter.set(50);
			adsr(0.0, 0.05, 0.25, 0.5);
		}

		event off(Amplitude velocity) {
			adsr.release();
		}

		void process() {
			((op1 * controls[1]) >> (op2 * controls[3]) >> op3) * adsr >> dcfilter >> out;
			//op2 >> debug;
			if (adsr.finished())
				stop();
		}
	};

	// Initialise plugin (called once at startup)
	Operators() {
		controls = {
			{ "Op1", Dial("f", 0, 2), Dial("Amt", 0, 10) },
			{ "Op2", Dial("f", 0, 2), Dial("Amt", 0 ,10) },
			{ "Op3", Dial("f", 0, 2),  },
		};
		
		presets = {
			{ "Bass", { 1.000, 0.500, 1.000, 4.296, 2 } },
		};

		notes.add<OperatorsNote>(32);
	}
};