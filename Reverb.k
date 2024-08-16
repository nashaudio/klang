
#include <klang.h>
using namespace klang::optimised;

struct Reverb : Stereo::Effect {

	struct Room { float width, length, height; };

	struct EarlyReflections : Stereo::Modifier {
		static constexpr int MAX_REFLECTIONS = 20;
	
		Stereo::Delay<21600> delay; // up to 100ms (+10%) at 192kHz
		
		Array<float, MAX_REFLECTIONS> times;  // (in samples)
		Array<stereo::signal, MAX_REFLECTIONS> gains; // stereo gain
		
		param length;	// reverb length (ms)
		param size;		// room size (diagonal, in metres)
		
		Stereo::Bank<LPF> lpf; // reduce 'boominess'
		Stereo::Bank<HPF> hpf; // reduce 'grainy-ness'
		
		void update() {
			constexpr float primes[20] = { 2,3,5,7,11, 13,17,19,23,29, 31,37,41,43,47, 53,59,61,67,71 };
			
			gains.count = times.count = 10 + int(size * 10.999); // larger spaces ~ more taps
			
			const float scale = 50.f / primes[times.count - 1]; // 50-100ms range
			const float ms = fs / 1000.f; // ms to samples
			
			for (int r = 0; r < times.count; r++) {
				times[r] = ((50.f + primes[r] * scale) * ms * random(0.9f, 1.1f)); 
				
				const float x = float(r + 1.f) / times.count;
				gains[r] = random(0.5f, 1.5f) * expf(-3.f * x);
				
				const float pan = random(0.f, 1.f);
				gains[r] = { gains[r].l * (1.f - pan), gains[r].r * pan }; // random pan
				
				//const float time = random(0.8f, 1.f) * averageDelay * (1.f + cube(x-1.f)); 			// increasing density
				//const float gain = random(0.8f, 1.2f) * expf(-3.f * times[r] / length) / (r + 1.f);  // exponential decay
				//const float pan = random(0.f, 1.f);
				
				//gains.add( { gain * (1.f - pan), gain * pan } ); // random pan
				//if(time > 10.f || gain < 0.001f)
				//	break;
					
				//times.add(time * fs);  // (convert to samples)
				//gains.add(random(0.f,1.f) > 0.5 ? gain : -gain);			
				//debug.print("%f @ %f\n", times[r] * 1000.f/fs.f, gains[r].l.value);
			}		
		}
		
//		void randomise(const float factor = 0.2f) {
//			for (int r = 0; r < times.count; r++) {
//				const float x = float(r) / times.count;
//				times[r] *= random(1.f-factor, 1.f); 
//				gains[r] *= 1 + random(-factor,factor);
//			}
//			
//		}
		
		void set(param length, param size){
			length *= 1/1000.f; // convert ms to s
			if(EarlyReflections::length != length || EarlyReflections::size != size){
				EarlyReflections::length = length;
				EarlyReflections::size = size;
				update();
				
				hpf.set(100);
				lpf.set(15000);
			}
		}
		
//		void set(const EarlyReflections& in){
//			if(length != in.length || size != in.size){
//				length = in.length;
//				size = in.size;
//				times = in.times;
//				gains = in.gains;
//				hpf.set(100);
//				lpf.set(15000);
//			}
//		}
		
		void process(){
			in >> lpf >> hpf >> delay;
				
			out = 0;
			for (int d=0; d < times.count; d++)
				out += delay(times[d]) * gains[d];
		}
	};
	
	
	
	// Initialise plugin (called once at startup)
	Reverb() : reflections(controls) {
		controls = { 
			Dial("Direct", 0.0, 1.0, 0.0),
			Dial("Early", 0.0, 1.0, 1.0),
			Dial("Mid", 0.0, 1.0, 0.0),
			Dial("Late", 0.0, 1.0, 0.0),
			Dial("Wet", 0.0, 1.0, 1.0),
			
			Dial("Length", 0.0, 100.0, 10.0),
			Dial("Room Size", 0.0, 1.0, 1.0),
			Dial("Dampening", 0.01, 1.0, 1.0),
			Dial("Tail", 0.01, 1.0, 1.0),
			Dial("Crosstalk", 0.00, 0.2, 0.0),
		};
		
		presets = {
			{ "Large Hall", { 1.000, 0.000, 0.419, 0.329, 1.000, 10.000, 100.000, 0.500, 0.500, 0.100 } },
		};
	}
	
	struct LateReflections : Mono::Modifier {
		struct FilteredDelay : Mono::Modifier {
			Delay<192000> delay;
			LPF filter;
			param gain;
		
			void set(param time, param cutoff, param gain){
				delay.set(time * fs / 1000.f);
				filter.set(cutoff);
				FilteredDelay::gain = gain;
			}
		
			void process() {
				(in >> delay >> filter) * gain >> out;
			}
		};
	
		FilteredDelay delay[4];
		
		void set(const param* delays, param length, param dampening, param gain){
			length = 1;//60.040f;
			delay[0].set(delays[0] * random(.9f,1.1f), dampening, gain);
			delay[1].set(delays[1] * random(.9f,1.1f), dampening, gain);
			delay[2].set(delays[2] * random(.9f,1.1f), dampening, gain);
			delay[3].set(delays[3] * random(.9f,1.1f), dampening, gain);
		}
				
//		void randomise() {
//			for (int d = 0; d < 4; d++) {
//				const float delta = random(0.0005f, 0.0015f);
//				delay[d].delay.set(delay[d].delay.time * (1.f + (random(0,1) > 0.5 ? delta : -delta)));
//			}
//		}
	
		void process() {	
			const signals<4> delays = { delay[0], 
										delay[1], 
										delay[2], 
										delay[3] };
		
			constexpr Matrix matrix = {	0, 1, 1,-1,
			   					   	   -1, 0,-1, 1,
									   -1, 1, 0,-1,
										1,-1, 1, 0  };	
										
			const signals<4> fb = (delays >> matrix) + in;
		
			( (fb[0] >> delay[0])
			+ (fb[1] >> delay[1])
			+ (fb[2] >> delay[2])
			+ (fb[3] >> delay[3]) ) >> out;
		}
	};
	
	struct Reflections : Stereo::Modifier {
		Controls& controls;
		
		Reflections(Controls& controls) : controls(controls) { }
		
		EarlyReflections early;
		LateReflections mid[2];
		LateReflections late[2];
		
		void set(param length, param size, param dampening1, param dampening2) {
			early.set((length / 10.f) * 1000.f + 50.f, size); // 50-100ms
			
			dampening1 *= 10000.f; 		// up to 10kHz
			dampening2 *= dampening1;	// dampening2 is a ratio of dampening1
			const param gain = 0.25;
			//const param delays1[] = { 6.040f, 8.868f, 17.390f, 48.870f };
			const param delays1[] = { 7, 11, 13, 17 };
			mid[0].set(delays1, length, dampening1, 0.25);
			mid[1].set(delays1, length, dampening1, 0.25);
////			late[0].delay[0].set( 6.040 * length, dampening1, 0.25);
////			late[0].delay[1].set( 8.868 * length, dampening1, 0.25);
////			late[0].delay[2].set(17.390 * length, dampening1, 0.25);
////			late[0].delay[3].set(48.870 * length, dampening1, 0.25);
//		
//			const param delays2[] = { 34.270f, 61.720f, 74.603f, 96.103f};
			const param delays2[] = { 19, 23, 29, 31 };
			late[0].set(delays2, length, dampening2, 0.35);
			late[1].set(delays2, length, dampening2, 0.35);
////			late[1].delay[0].set(34.270 * length, dampening2, 0.35);
////			late[1].delay[1].set(61.720 * length, dampening2, 0.35);
////			late[1].delay[2].set(74.603 * length, dampening2, 0.35);
////			late[1].delay[3].set(96.130 * length, dampening2, 0.30);
		}
		
		void randomise() {
			//early.randomise();
			//late[0].randomise();
			//late[1].randomise();
		}
		
		void process() {
			const stereo::signal reflections1 = in >> early;
//			const mono::signal mids[2] = { reflections1.l >> mid[0], 
//									       reflections1.r >> mid[1]};
//			const stereo::signal reflections2 = {mids[0],mids[1]};
			const stereo::signal reflections2 = { reflections1.l >> mid[0], 
								 	   		  	  reflections1.r >> mid[1] };
								 	   		  	  
//			const mono::signal lates[2] = { reflections2.l >> late[0], 
//									        reflections2.r >> late[1]};										 
//			const stereo::signal reflections3 = {lates[0],lates[1]};
			const stereo::signal reflections3 = { reflections2.l >> late[0], 
    			 	   		  	  				  reflections2.r >> late[1] };
		
//			( controls[1] * reflections1 ) >> out;
//			( controls[2] * reflections2 ) >> out;
			( controls[1] * reflections1 
			+ controls[2] * reflections2 
			+ controls[3] * reflections3 ) >> out;
		}
	} reflections;


	param length, size, dampening, tail;

	// Prepare for processing (called once per buffer)
	void prepare() {	
		if(controls.changed()){
			random(272839);
			reflections.set(controls[5], controls[6], controls[7], controls[8]);
			//right.set(controls[5], controls[6], controls[7], controls[8]);
			//right.set(left.early,              controls[7], controls[8]);
			
			//left.early.randomise(0.1f);
			//left.early.gains.normalise(0.1f, RMS);
			//right.early.randomise(0.1f);
			//right.early.gains.normalise(0.1f, RMS);

			//random(251780);
			//left.randomise();
			//right.randomise();
			
//			right.late[0].delay[0].set( 6.040 + length) * 0.9995f,dampening1, gain);
//			right.late[0].delay[1].set( 8.868 + length * 1.002f, dampening1, gain);
//			right.late[0].delay[2].set(17.390 + length * 0.983f, dampening1, gain);
//			right.late[0].delay[3].set(48.870 + length * 1.013f, dampening1, gain);
//		
//			right.late[1].delay[0].set(34.270 * length * 0.989f, dampening2, 0.35);
//			right.late[1].delay[1].set(61.720 * length * 0.995f, dampening2, 0.35);
//			right.late[1].delay[2].set(74.603 * length * 1.003f, dampening2, 0.35);
//			right.late[1].delay[3].set(96.130 * length * 1.007f, dampening2, 0.30);
		}
	}

	// Apply processing (called once per sample)
	void process() {
		const param dry = controls[0];
		const param wet = controls[4];
		//const param crosstalk = controls[9];
			
		(in * dry + (in >> reflections) * wet) >> out;
		
		//(in.l + crosstalk * right.out) >> left;
		//(in.r + crosstalk * left.out) >> right;
		
		//(in.l * dry + left * wet) >> out.l;
		//(in.r * dry + right * wet) >> out.r;
	}
};