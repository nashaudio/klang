
#include <klang.h>
using namespace klang::basic;

float cubic(float x) { return x * x * x; }

struct Banjo : Synth {
	float playingNotes = 0;
	IIR filter;
	HPF dcfilter;
	const float normalise;
	
	struct MyNote : Note {
		Delay<44100> strum;
		Banjo* synth;
	
		struct Excitation : Generator {
			Envelope impulse;
			Delay<44100> delay;
			Noise noise;
			
			void process() {
				const signal exc = impulse * noise;
				exc - (exc >> delay) >> out;
			}
		} pluck;
			
		struct Resonator : Modifier {
			Delay<44100> delay;
			IIR filter;
			param gain;
			
			void process() {
				(in + delay * gain) >> filter >> out >> delay;
			}
		} string;
		
		Resonator string2;
		
		ADSR adsr;
		
		// Note On
		event on(Pitch pitch, Amplitude velocity) {	
			param f = (pitch - 12) -> Frequency;
			param delay = 1/f * klang::fs;// - 1.5;
					
			synth = (Banjo*)getSynth();
			strum.clear();
			strum.set(synth->playingNotes * random(0.004, 0.01) * klang::fs);	
			synth->playingNotes++;
			
			velocity = velocity * random(0.99, 1.01);			
			
			param dampen = 0;
				 if(pitch  > 64) dampen = 0.2;
			else if(pitch == 64) dampen = 0.1;
			else if(pitch  > 59) dampen = 0.15;
			else if(pitch == 59) dampen = 0.05;
			else if(pitch  > 55) dampen = 0.25;
			else if(pitch == 55) dampen = 0.25;
			else if(pitch  > 50) dampen = 0.3;
			else if(pitch == 50) dampen = 0.2;
			else if(pitch  > 45) dampen = 0.45;
			else if(pitch == 45) dampen = 0.4;
			else if(pitch == 40) dampen = 0.5;
			else 				 dampen = 0.55;
			
			dampen = dampen - controls[1];
			if(dampen < 0) dampen = 0;
			if(dampen > 1) dampen = 1;
					
			param finger = controls[2];// + 0.0125 * (pitch - 36) / 12;
			if(finger < 0) finger = 0;
			if(finger > 1) finger = 1;
			
			param resonance = controls[0];// + 0.002 * pitch / 12;
			if(resonance > 1.0) resonance = 1.0;
			
			string.delay.clear();
			string.delay.set(delay);
			string.filter.set(1 - dampen);
			string.gain = resonance;
			
			string2.delay.clear();
			string2.delay.set(delay * random(0.996, 0.998));
			string2.filter.set(1 - dampen);
			string2.gain = resonance;
			
			pluck.delay.clear();
			pluck.delay.set(finger * delay);
			pluck.impulse = { { 0.000, velocity }, { 0.001, velocity }, { 0.002, 0 }, { 0.003, 0 } };

			adsr(0,0,1,3);
		}
		
		event off(Amplitude velocity){
			string.gain = 0.980;
			synth->playingNotes--;
			adsr.release();
		}
		
		// Apply processing (called once per sample)
		void process() {
			const signal excitation = pluck >> strum;
			//signal string1, string;
			//excitation >> string >> string1;
			//excitation >> string2;
			
			
			((excitation >> string) + (excitation >> string2)) * 0.5 >> out;
					
			out *= 0.3; adsr;
			if(adsr.finished())
				stop();
		}
	};
	
	// Initialise plugin (called once at startup)
	Banjo() : normalise(0.25f/tanh(3)) {
		controls = {
			Dial("Resonance", 0.97, 0.999, 0.99),
			Dial("Brightness", -0.1, 0.1, 0.0),
			Dial("Fingering", 0, 0.25, 0.02),
		};
			
		notes.add<MyNote>(32);
	}
	
	void process() {
		in *= 1 + controls[4];
		if(in > 1)
			out = 1;
		else if(in < -1)
			out = -1;
		else
			out = in;
		
		tanh(out * 3) * normalise >> out;
		if(controls[4] < 2)
			out *= 1 + (2 - controls[4]);
	}
};