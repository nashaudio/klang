
#include <klang.h>
using namespace klang::optimised;

struct Compressor : Effect {
	Envelope::Follower follower;

	// Initialise plugin (called once at startup)
	Compressor() {
		controls = { 
			Meter("Input"),
			Dial("Threshold", -96, 0, 0),
			Dial("Ratio", 0.0, 100.0, 0.5),
			Meter("Compression"),
			Dial("Makeup Gain", -12, 12, 0),
			
			Dial("Pre Gain", -12, 12, 0),
			Dial("Attack", 0, 0.2, 0.5),
			Dial("Release", 0, 1, 0.5),
			Dial("Drive", -11, 11, 1),
		};
		
		compress.threshold_dB = controls[1];
		compress.ratio = controls[2];
		compress.compression = controls[3];
		compress.makeup = controls[4];
	}

	// Prepare for processing (called once per buffer)
	void prepare() {
		follower.set(sqr(controls[6]), sqr(controls[7]));
		follower = Envelope::Follower::Peak;
	}
	
	struct Dynamics : Modifier {
		param level = 0.f;
		
		ControlMap threshold_dB;
		param threshold;
		
		ControlMap ratio;
		ControlMap compression;
		ControlMap makeup;
		
		void set(param level){
			Dynamics::level = level;
			threshold = dB(Dynamics::threshold_dB) -> Amplitude;
		}
		
		float gain(float level){	
			if(level > threshold){
				return 1 - (level - threshold) * (1.f - 1.f/ratio);
			} if(level < threshold * 0.5) {
				return std::max(0.f,(2.f * level) / threshold);
			} else {
				return 1;
			}
		}
		
		void process() {
//			constant threshold = 0.4;
//			constant ratio = 1.f/4.f;
//		
//			if(level > threshold){
//				out = in * threshold + (level - threshold) / ratio;
//			} else {
//				out = in;
//			}

			const param c = gain(level);
			c >> compression;
			
			//const param makeup_gain = std::clamp(float(dB(-threshold_dB * (1.f - 1.f / ratio)) -> Amplitude), 0.f, 8.f) >> makeup;
			in * c * (dB(makeup) -> Amplitude) >> out;
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
				
		//return tanh(clip(x, threshold) * drive) / (0.1 * drive + tanh(drive));
	}

	// Apply processing (called once per sample)
	void process() {
		in >> follower >> controls[0];
		
		const signal gain = follower;
		
		//Dynamics::gain >> graph(0,1,0,1);
		
		in >> debug;
		(in * dB(controls[5]) -> Amplitude) >> compress(gain) >> out;
		
		out = softclip(out, controls[8], sqrt(follower.out));
	}
};