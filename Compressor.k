
#include <klang.h>
using namespace klang::optimised;

struct Compressor : Effect {
	Envelope::Follower follower;

	Compressor() {
		controls = { 
			Meter("Input"),
			Dial("Threshold", -96, 0, 0),
			Dial("Gate", -18, 0, 0),
			Dial("Ratio", -32, 32.0, 0),
			Meter("Compression", 0, 2, 1),
			
			Dial("Pre Gain", -12, 12, 0),
			Dial("Attack", 0, 1, 0.5),
			Dial("Release", 0.1, 0.9, 0.5),
			Dial("Makeup Gain", -12, 12, 0),
			Dial("Drive", -11, 11, 0),
		};
		
		presets = {
			{ "Punchy Bass", { 0, -7.057, -0.493, -32.000, 0, 2.358, 0.234, 0.100, -0.340, 0.000 } },
			{ "Dirty Break",   { 0, -11.239, 0.000, 32.000, 0, 0.000, 0.000, 0.663, 6.000, 2.105 } },
			{ "Power Upgrade", { 0, -40.916, 0.000, 12.742, 0, 0.000, 0.000, 0.900, 12.000, -1.000 } },
			{ "Industrial", { 0, -1.851, -1.506, 32.000, 0, 0.000, 0.000, 0.500, 6.034, -2.029 } },
			{ "Funky Beat", { 0, -5.781, -5.000, 32.000, 0, 0.000, 0.000, 0.500, -12.000, -11.000 } },
			{ "Tight Beat", { 0, -3.375, -0.377, 32.000, 0, 2.532, 0.035, 0.900, -12.000, -11.000 } },
			{ "Deverb", { 0, -3.764, -13.989, -9.353, 0, 0.000, 0.095, 0.351, -5.142, -2.000 } },
		};
		
	}

	void prepare() {
		follower.set(sqr(controls[6]), sqr(controls[7]));
		follower = Peak;
		
		compress.threshold = dB(controls[1]) -> Amplitude;
		compress.gate =         controls[2] == 0 ? 0.f 
		                                         : float(dB(controls[1]+controls[2]) -> Amplitude);
		compress.ratio =    abs(controls[3]) < 1 ? 1.f  
		                      : controls[3]  < 0 ? 1.f + controls[3] / 31.5f
		                                         : controls[3];	
		compress.meter = 	    controls[4]; // map to meter
		compress.makeup =    dB(controls[8]) -> Amplitude;			
	}
	
	struct Dynamics : Modifier {
		param level = 0.f;
		param threshold, gate, ratio, makeup;
		
		ControlMap meter;
		
		void set(param level){
			Dynamics::level = level;
		}
		
		float gain(float level){	
			if(level > threshold){
				return 1 - (level - threshold) * (1.f - 1.f/ratio);
			} if(gate && level < gate) {
				return 10.f * (level - gate) + 1.f;
			} else {
				return 1;
			}
		}
		
		void process() {
			const param compression = gain(level) >> meter;		
			
			in * compression * makeup >> out;
		}
		
	} compress;
	
	signal clip(signal in, const float threshold) {
		if(in > threshold)
			return threshold;
		else if(in < -threshold)
			return -threshold;
		else
			return in;
	}
	
	inline float softclip(float x, float c, const float threshold) {		
		x = clip(x * (1 + abs(c)), 1);

		if(c < 0)
			c *= 0.005f;
						
		if(x > 0)
			return (1 + c) * x / (1 + c * x);
		else
			return (1 + c) * x / (1 - c * x);
	}

	// Apply processing (called once per sample)
	void process() {
		in * dB(controls[5]) -> Amplitude >> in;
	
		(in >> debug) >> follower >> controls[0];
		const signal gain = follower;	
		
		in >> compress(gain) >> out;	
		
		if(controls[9])
			out = softclip(out, controls[9], sqrt(gain));
	}
};