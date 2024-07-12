
#include <klang.h>
//#include <klang/tests.h>

using namespace klang::optimised;

struct TB303 : Modifier {
	float fc = 0; 			// cutoff frequency
	float resonance = 0; 	// resonance (normalised)

	float b0 = 0;           // coefficient for the first order sections
    float z[4] = { 0 };     // filter state (outputs of the 4 ladder stages)
    float k = 0;            // feedback factor in the loop
	float r = 0; 			// resonance (0 .. 1)
	float g = 1.0;			// output gain
	
	OnePole::HPF feedback;	// feedback highpass (to prevent self-oscillation)
	
	TB303() {
		
	}
	
	void setHPF(param f, param Q){
		feedback.set(f);
	}

	void set(param cutoff, param resonance){
		if(fc != cutoff || TB303::resonance != resonance){
			fc = cutoff;
			TB303::resonance = resonance;
    		r = (1.f-exp(-0.03f*resonance)) / (1.f-exp(-3.f));
		
			const float fx = fc * fs.inv * root2.inv; 
			b0 = (0.00045522346f + 6.1922189f * fx) / (1.f + 12.358354f * fx + 4.4156345f * (fx * fx)); 
			k  = fx*(fx*(fx*(fx*(fx*(fx+7198.6997f)-5837.7917f)-476.47308f)+614.95611f)+213.87126f)+16.998792f; 
			g  = k * 0.058823529411764705882352941176471f; // 17 reciprocal 
			g  = (g - 1.f) * r + 1.f;                     // r is 0 to 1.0
			g  = (g * (1.f + r)); 
			k  = k * r;                                   // k is ready now 
		}
	}

	static inline float shape(float x) {
		constexpr float r_6 = 1.f/6.f;
		return (x > root2) ? root2 : (x < -root2) ? -root2 : (x - r_6*x*x*x);
	}
	
	void process() {
		signal y0 = 0;
		(k * (z[3])) >> feedback >> y0;
		in -= y0;  
		z[0] += 2.f * b0 * (in   -       z[0] + z[1]);
		z[1] +=       b0 * (z[0] - 2.f * z[1] + z[2]);
		z[2] +=       b0 * (z[1] - 2.f * z[2] + z[3]);
		z[3] +=       b0 * (z[2] - 2.f * z[3]);
		out = 2.f* g * z[3];
	}
};

struct MySynth : Synth {

	struct MyNote : public Note {
		Saw osc;
		ADSR adsr;
		TB303 filter[10];
		Envelope flt;
		param cutoff;

		event on(Pitch pitch, Velocity velocity) { 
			const param f = pitch -> Frequency;
			osc(f, 0, 0.1);
			adsr(0.001, 0, 1, 0.25);
			cutoff = controls[0] * random(0.25, 4.0);
			filter[0].setHPF(controls[2], controls[3]);
			flt = { { 0,1 }, { 1.0, 0 } };
		}

		event off(Velocity velocity){
			adsr.release();
		}
		
		void prepare() {
			for(int f=0; f<10; f++){
				
			}
		}

		void process() {
			const signal amp = osc * adsr++;
			//for(int f=0; f<10; f++){
			filter[0].set(cutoff * flt++, controls[1]);
				amp >> filter[0] >> out;
			//}
			if (adsr.finished())
				stop();
		}
	};
	
	
	// Initialise plugin (called once at startup)
	MySynth() {		
		controls = { 
			// UI controls and parameters
			Dial("f", 100, 20000, 1000),
			Dial("Q", 0, 1, 0),
			Dial("HPF f", 10, 250, 150),
			Dial("HPF q", 0, 10, 1)
		};
		
		notes.add<MyNote>(32);
	}
}; 