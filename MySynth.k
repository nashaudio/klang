
#include <klang.h>
using namespace klang::optimised;

struct TB303 : Modifier {
	param cutoff = 0; 		// cutoff frequency
	param resonance = 0; 	// resonance (normalised)
	param drive = 1;		// overdrive

	float b0 = 0;           // coefficient for the first order sections
    float z[4] = { 0 };     // filter state (outputs of the 4 ladder stages)
    float k = 0;            // feedback factor in the loop
	param r = 0; 			// resonance (0 .. 1)
	float g = 1.0;			// output gain
	
	const float normalise;
	
	TB303() : normalise(1.f/tanh(1)) {}
	
	OnePole::HPF feedback;	// feedback highpass (to prevent self-oscillation)

	void reset() {
		cutoff = 0;
		resonance = 0;
		drive = 1;
    	z[0] = z[1] = z[2] = z[3] = 0; // reset filter state
    	feedback.reset();
	}
	
	void setHPF(param f){
		feedback.set(f);
	}

	void set(param cutoff, param resonance, param drive = 1){
		if(cutoff > fs.nyquist)
			cutoff = fs.nyquist;
			
		if(TB303::cutoff != cutoff || TB303::resonance != resonance || TB303::drive != drive){
			TB303::cutoff = cutoff;
			TB303::resonance = resonance;
			TB303::drive = drive;
    		r = (1.f-exp(-0.03f*resonance)) / (1.f-exp(-3.f));
		
			const float fx = cutoff * fs.inv * root2.inv; 
			b0 = (0.00045522346f + 6.1922189f * fx) / (1.f + 12.358354f * fx + 4.4156345f * (fx * fx)); 
			k  = fx*(fx*(fx*(fx*(fx*(fx+7198.6997f)-5837.7917f)-476.47308f)+614.95611f)+213.87126f)+16.998792f; 
			g  = k * 0.058823529411764705882352941176471f; // 17 reciprocal 
			g  = (g - 1.f) * r + 1.f;                     // r is 0 to 1.0
			g  = (g * (1.f + r)); 
			k  = k * r;                                   // k is ready now 
		}
	}

	static inline float shape(float x) {
		//constexpr float r_6 = 1.f/6.f;
		return (x > root2) ? root2.f : (x < -0.5) ? -0.5f : x;
	}
	
	static inline float hardclip(float x) {
		return (x > 1.f) ? 1.f : (x < -1.f) ? -1.f : x;
	}
	
	inline float softclip(float x, float drive) {		
		return tanh(hardclip(x) * drive) / (0.1 * drive + tanh(drive));
	}
	
	void process() {
		signal y0 = 0;
		((k * z[3]) >> feedback) * 0.9f * resonance >> y0;
		in -= shape(y0);  
		z[0] += 2.f * b0 * (in   -       z[0] + z[1]);
		z[1] +=       b0 * (z[0] - 2.f * z[1] + z[2]);
		z[2] +=       b0 * (z[1] - 2.f * z[2] + z[3]);
		z[3] +=       b0 * (z[2] - 2.f * z[3]);
		softclip (g * z[3], drive) >> out;
	}
};

struct MySynth : Synth {

	event control(int parameter, float value){
		debug.print("S %d = %f\n", parameter, value);
	}
	
	event preset(int index){
		debug.print("S Preset = %d\n", index);
	}

	struct MyNote : public Note {
		Saw saw;
		Square square;
		ADSR adsr;
		TB303 filter;
		Envelope flt;
		param f;
		
		event control(int parameter, float value){
			debug.print("N %d = %f\n", parameter, value);
		}
	
		event preset(int index){
			debug.print("N Preset = %d\n", index);
		}

		event on(Pitch pitch, Velocity velocity) { 
			f = pitch -> Frequency;
			square(f, 0);
			saw(f, 0);
			adsr(0.00, controls[2], 1, 0.1);
			filter.reset();
			flt = { { 0,1 }, { controls[2], 0.0 } };
		}

		event off(Velocity velocity){
			adsr.release();
		}
		
		void process() {
			const bool is_square = controls[3] == 1;
			const signal osc = is_square ? (signal)(square * 0.5f) : (signal)saw;
			const float fc = (f + (controls[0]^2) * fs.nyquist) * ((flt++)^2);
			const float hpf_f = 10.f + 490.f * controls[1]; // + 0.f * controls[0];
			filter.set(fc, (is_square ? 0.9f : 1.f) * controls[1], controls[4]);
			filter.setHPF(hpf_f);
			(osc >> filter) * adsr++ >> out;
			if (adsr.finished())
				stop();
		}
	};
	
	
	// Initialise plugin (called once at startup)
	MySynth() {		
		controls = { 
			// UI controls and parameters
			Dial("f", 0, 1, 1),
			Dial("Q", 0, 1, 0.5),
			Dial("Decay", 0.1, 1, 0.5),
			Menu("Waveform", "Saw", "Square"),
			Dial("Accent", 0.01, 10, 1)
		};
		
		notes.add<MyNote>(32);
	}
}; 