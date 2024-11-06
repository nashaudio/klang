
#include <klang.h>
using namespace klang::optimised;

#define BANDS 22

struct Vocoder : Effect {

	Noise noise;

	Saw saw[BANDS];
	BPF filter[2][BANDS];
	HPF hpf[2];
	signal bands[2][BANDS];
	Envelope::Follower follower[BANDS];
	
	HPF dcfilter;
	
	float f_scale[BANDS] = { 0 };
	
	// Initialise plugin (called once at startup)
	Vocoder() {
		controls = { 
			Dial("Pitch", 0, 100, 60),
			Dial("Range", 1, 4, 1),
			Dial("Attack", 0.01, 0.1, 0.02),
			Dial("Release", 0.01, 0.1, 0.04),
			Dial("Detail", 0.1, 15, 10),
		};
		
		const int width = 370 / BANDS;
		for(int b=0; b<BANDS; b++){
			controls.add("");
			controls[5+b].type = Control::METER;
			controls[5+b].size = { 25 + width * b, 150, width - 2, 100 };
		}
		
		controls.group("Vocoder Bands", 5, BANDS);
		
		presets = {
		
		};
		
	
		for(int b=0; b<BANDS; b++)
			f_scale[b] = random(0.99995f,1.00005f);
			
		dcfilter.set(50, 10);
	}

	// Prepare for processing (called once per buffer)
	void prepare() {
		const param f0 = Pitch(controls[0] + 12 * (controls[1] - 1.f)) -> Frequency;
	
		constant f_min = { f0 };
		constant f1_max = { f_min + 5000 };
		constant f2_max = { 5000 };
		constant r1 = { power(f1_max / f_min, 1.f / (BANDS - 1)) };
		constant r2 = { power(f2_max / f_min, 1.f / (BANDS - 1)) };
		
		for(int b=0; b<BANDS; b++){
			const float f_cnt1 = f_min * (r1 ^ b);
			filter[0][b].set(f_cnt1, controls[4]);
			
			const float f_cnt2 = f_min * (r2 ^ b);
			filter[1][b].set(f_cnt2, 0.5 * controls[4]);
		}
		
		hpf[0].set(3000 + controls[1] * 250, 1);
		hpf[1].set(f0 + 3000, 2);
		
		constant frequency = { controls[0] };
		constant detune = { 0.01 };

		for(int b=0; b<BANDS; b++){
			saw[b].set(frequency);
			follower[b] = RMS;
			follower[b].set(controls[2], controls[3]);
		}
	}
	
	static float _boost(float x){
		return 2.f * (1.f - power(1.f - x, 2.f));
	}
	
	// Apply processing (called once per sample)
	void process() {
		Function<float> boost(_boost);
		
		out = 0;
		
		in + 0.125 / controls[1] * (in >> hpf[0]) >> in;
		
		signal sidechain = 0;
		for(int s=0; s<BANDS; s++)
			sidechain += saw[s] / BANDS;// +*/ noise * 0.125;
			
		sidechain + 0.25 / controls[1] * (sidechain >> hpf[1]) >> sidechain;
		
		// split input into 16 bands
		for(int b=0; b<BANDS; b++){
			      (in >> filter[0][b] >> bands[0][b] >> abs >> boost) >> follower[b] >> controls[5 + b];
			sidechain >> filter[1][b] >> bands[1][b];
		}
		
		for(int b=0; b<BANDS; b++){
			out += bands[1][b] * follower[b].out * controls[4] / 7.5f;
		}
		
		out >> dcfilter >> out;
	}
};