
#include <klang.h>
using namespace klang::basic;

struct PingPong : Stereo::Effect {
	
	Delay<192000> left, right;
	Sine lfo;
	HPF dcfilter[2];
	param delay;

	// Initialise plugin (called once at startup)
	PingPong() {
		controls = { 
			Dial("Feedback", 0.0, 0.999, 0.5),
			Dial("Delay", 0.001, 1.0, 0.5),
			Dial("Scratch", 0.0, 1.0, 0.0),
			Dial("Rate", 0.01, 1.0, 0.0),
			Dial("Dry", 0.001, 2.0, 1.0),
			Dial("Delay"),
		};
		
		presets = {
			{ "Phat + Sinister", { 0.958, 0.018, 0.166, 0.579, 0.001, 0.026 } },
			{ "Funky Beat", { 0.663, 0.248, 0.411, 0.594, 2.000, 0.283 } },
			{ "Station Concourse", { 0.584, 0.380, 0.000, 0.010, 0.775, 0.380 } },
			{ "Metal Voice", { 0.940, 0.025, 0.000, 0.010, 0.138, 0.025 } },
			{ "Bad Trip", { 0.881, 0.651, 0.560, 0.028, 0.138, 0.772 } },
			{ "Pitchy + Scratchy", { 0.881, 0.643, 0.4, 0.127, 0.001, 0.500 } },
			{ "Burpy Bubbles", { 0.272, 0.234, 0.648, 0.127, 0.001, 0.201 } },
			{ "Doctor Who?", { 0.325, 0.008, 0.382, 1.000, 0.001, 0.000 } },
		};
	}

	// Prepare for processing (called once per buffer)
	void prepare() {

		dcfilter[0].set(50,1);
		dcfilter[1].set(50,1);
	}

	// Apply processing (called once per sample)
	void process() {
		const param rate = sqr(controls[3]) * 100.f;
		const param new_delay = controls[5].smooth();
		if(std::abs(delay - new_delay) > 0.001){
			delay = new_delay;
			controls[1].set(new_delay);	
			lfo.set(rate, pi);
		}else{
			delay = controls[5];
			lfo.set(rate);
		}
			
		const param gain = controls[0];
		const param delay = controls[1].smooth();
		const param vibrato = sqr(controls[2]) * rate * root2;
		const param dry = controls[4];
		controls[1].set(controls[1] + lfo * vibrato * 0.00005);
		
		controls[1].smoothed >> debug;
			
		left.set(delay * fs);
		right.set(0.5f * delay * fs);
	
		dry * in.l + (1.f - dry) * ((in.l + right * gain) >> left) >> out.l;
		dry * in.r + (1.f - dry) * ((in.r + left * gain) >> right) >> out.r;	
		
		out.l >> dcfilter[0] >> out.l;
		out.r >> dcfilter[1] >> out.r;
	}
};