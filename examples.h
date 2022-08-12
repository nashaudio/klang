//
//  SynthExtra.h
//  Additional Plugin Code
//
//  This file is a workspace for developing new DSP objects or functions to use in your plugin.
//

#include <klang.h>

namespace klang {

	template<int HARMONICS>
	struct Harmonics : public Oscillator {
		Sine harmonics[HARMONICS];

		void set(param frequency) {
			for (int n = 0; n < HARMONICS; n++)
				harmonics[n].set(frequency * (n + 1));
		}

		signal output() {
			signal mix = 0;
			for (int n = 0; n < HARMONICS; n++)
				if(harmonics[n].frequency < (fs * 0.5))
					mix += harmonics[n] * 1.f / (n + 1);
			return mix;
		}
	};

	class Simple : public Note {
		Harmonics<8> osc;

		event on(Pitch pitch, Amplitude velocity) {
			osc.set(pitch > Type::Frequency);
		}

		signal output() {
			return osc * velocity;
		}
	};

	template<int HARMONICS>
	class Additive : public Note {
		Sine harmonics[HARMONICS];

		event on(Pitch pitch, Amplitude velocity) {
			const param fundamental(pitch > Type::Frequency);
			for (int n = 0; n < HARMONICS; n++)
				harmonics[n].set(fundamental * (n + 1));
		}

		signal output() {
			signal mix = 0;
			for (int n = 0; n < HARMONICS; n++)
				mix += harmonics[n] * 1.f / (n + 1);
			return mix;
		}
	};

	class Subtractive1 : public Note {
		Saw osc1, osc2;
		LPF lpf;
		Sine lfo1, lfo2;

		event on(Pitch pitch, Amplitude velocity) {
			const Frequency f(pitch > Type::Frequency);
			osc1.set(f * 0.99f);
			osc2.set(f * 1.01f);
			lfo1.set(6);
			lfo2.set(1);
		}

		signal output() {
			signal mod = lfo1 * 0.5 + 0.5;
			signal mix = (osc1 + osc2) * 0.5f >> lpf(mod);
			return mix * lfo2;
		}
	};

	class Subtractive2 : public Note {
		Saw osc;
		Envelope env;

		event on(Pitch pitch, Amplitude velocity) {
			const Frequency frequency(pitch > Type::Frequency);
			osc.set(frequency);

			env = { { 0, 0 }, { 1, 1 }, { 2, 0.1f }, { 3, 1 } };
		}

		signal output() {
			signal mix = osc;
			return mix * env++;
		}
	};

	class Subtractive3 : public Note {
		Harmonics<8> osc;
		ADSR adsr;
		LPF lpf1,lpf2;
		Sine lfo;

		event on(Pitch pitch, Amplitude velocity) {
			const Frequency frequency(pitch > Type::Frequency);
			osc.set(frequency);
			adsr(0.25, 0.25, 0.5, 5.0);
			lfo.set(3);
		}

		event off() {
			adsr.release();
		}

		signal output() {
			signal mod = lfo * 0.5 + 0.5;
			signal mix = osc * adsr++ >> lpf1(mod) >> lpf2(mod);
			if (adsr.finished())
				stop();
			return mix;
		}
	};

	class Delay1 : public Note {
		Harmonics<8> osc;
		ADSR adsr;
		Delay<192000> delay;

		event on(Pitch pitch, Amplitude velocity) {
			const Frequency frequency(pitch > Type::Frequency);
			osc.set(frequency);
			adsr(0.01, 0.05, 0.01, 3.00);
		}

		event off() {
			adsr.release();
		}

		signal output() {
			signal mix = osc * adsr++;
			signal echo = mix >> delay(0.5 * fs);
			if (adsr.finished())
				stop();
			return mix + echo * 0.5;
		}
	};

	class Stab : public Note {
		Harmonics<8> osc;
		ADSR adsr;

		event on(Pitch pitch, Amplitude velocity) {
			const Frequency frequency(pitch > Type::Frequency);
			osc.set(frequency);
			adsr(0.01, 0.05, 0.05, 0.25);
		}

		event off() {
			adsr.release();
		}

		signal output() {
			signal mix = osc * adsr++;
			if (adsr.finished())
				stop();
			return mix;
		}
	};

	class Physical : public Note {
		struct Exciter {
			Envelope impulse;
			Delay<44100> delay;
		};
		
		struct Resonator {
			Delay<44100> delay;
			LPF filter;
		};
		
		Exciter exciter;
		Resonator resonator;
		Envelope amp;
		HPF dcfilter;

		event on(Pitch pitch, Amplitude velocity) {
			const Frequency frequency((pitch - 12) > Type::Frequency);
			
			exciter.impulse = { { 0, 0 }, { 0.001, 1 }, { 0.003, -1 }, { 0.004, 0} };
			param delay = fs / frequency - 2;
			resonator.delay.set(delay);
			exciter.delay.set(delay * 0.5);

			amp = { { 0,0 }, { 0.001, 1 } };
			amp.setLoop(1, 1);
			dcfilter.set(0.9);
		}

		event off() {
			amp.release(1.0);
		}

		signal excitation() {
			signal excitation = exciter.impulse++;
			excitation += -(excitation >> exciter.delay);
			return excitation;
		}

		signal feedback() {
			return resonator.delay * 0.999f >> resonator.filter(0.75);
		}

		signal output() {
			signal mix = excitation() + feedback();
			mix >> resonator.delay;

			if (amp.finished())
				stop();

			return (mix * amp++ * 0.5f) >> dcfilter;
		}
	};

	class Test {
		Sine osc1, osc2, osc3;
		Noise noise;
		Sine lfo;
		LPF lpf;
		Delay<44100> delay;
		Envelope env, pluck;

		Delay<44100> delay1, delay2;
	public:
		void noteOn(Pitch pitch, Amplitude velocity) {
			const Frequency frequency(pitch);

			osc1.set(frequency);
			osc2.set(frequency * 1.5f);
			osc3.set(frequency * 1.5f * 1.5f);
			lfo.set(1.f);
			lpf.set(0.1f);
			delay.set(fs * 0.5f);

			delay1.set(fs/frequency);
			delay2.set(fs/frequency * 0.25f);

			//env.set(Envelope::Points(0, 0)(1, 1)(2, 0.1f)(3,0));
			env = { {0, 0}, { 1, 1} };
			env.setLoop(1, 1);

			pluck = { { 0, 0 }, { 0.001f, 1 }, { 0.003f, -1 }, { 0.004f, 0} };
		}

		bool noteOff() {
			env.release(2.5f);
			return false;
		}

		bool output(Buffer buffer) {
			signal mix, mod;

			while (buffer) {

				//// Karplus-Strong
				//Signal excitation = pluck++ * 0.1f;
				//excitation += -1.f * (excitation >> delay2);
				//Signal feedback = delay1 * 0.999f >> lpf(0.75f);
				//Signal output = excitation + feedback;
				//output >> buffer;
				//output >> delay1;

				(osc1 + (osc2 * 0.5f) + (osc3 * 0.25f)) >> mix;
				mix += noise * 0.1f;

				//mix >> lpf(mod) >> mix;
				//mix >> buffer;

				mod = lfo * 0.5 + 0.5;
				mix >> lpf(mod) >> mix;
				//Signal del = (mix * 0.25f) >> delay;

				mix * env++ * 0.25f >> buffer;

				//buffer = (osc1 + osc2 * 0.5f + osc3 * 0.25f) >> lpf(mod);

				buffer++;
			}
			return env != Envelope::Off;
		}
	};

};