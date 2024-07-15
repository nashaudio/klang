#include <klang.h>
using namespace klang::optimised;

struct Follower {

	struct AR : Modifier {
		param attack = 0;
		param release = 0;
	
		param A = 1, R = 1;
			
		void set(param attack, param release) {
			if(AR::attack != attack || AR::release != release){
				AR::attack = attack;
				AR::release = release;
				A = 1.f - (attack == 0.f ? 0.f : std::exp(-1.0f / (fs * attack)));
				R = 1.f - (release == 0.f ? 0.f : std::exp(-1.0f / (fs * release)));
			}
		}
		
		void process() {
			const float smoothing = in > out ? A : R;
			(out + smoothing * (in - out)) >> out;
		}
	};

	struct Base : Modifier {
		AR ar;
		
		Base() { set(0.01f, 0.1f); }
		
		void set(param attack, param release) {
			ar.set(attack, release);
		}
		
		virtual void process() = 0;
	};
	
	struct Test : Base {
		Square tone;
		
		Test() { tone.set(1.f); }
		
		void process() {
			(tone * 0.25 + 0.25) >> ar >> out;
		}
	};

	struct RMS : Base {
	
		Butterworth::LPF filter;
		signal level = 0;
	
		RMS() { filter.set(10); }
		
		void process() {
			in >> sqr >> filter >> ar >> sqrt >> out;
		}
	};

	template<int WINDOW>
	struct Average : Base
	{
		buffer buffer;
		double sum = 0; // NB: must be 64-bit to avoid cumulative rounding errors
		static constexpr constant window = { WINDOW };
			
		Average() : buffer(WINDOW, 0) { 
			set(0.0f, 0.1f);
		}
	
		void process() {
			sum -= double(buffer);
			in * in >> buffer;
			sum += double(buffer++);
			if(buffer.finished())
				buffer.rewind();
			sqrt(sum * window.inv) >> ar >> out;
			//sum * window.inv >> sqrt >> ar >> out;
		}
	};

	struct Peak : Base {
		void process() {
			in >> abs >> ar >> out;
		}
	};
};

struct Followers : Effect {

	Follower::RMS rms;
	Follower::Average<2000> window;
	Follower::Peak peak;
	Follower::Test test;

	// Initialise plugin (called once at startup)
	Followers() {
		controls = { 
			Dial("Gain", 0.0, 1.0, 0.5),
			Meter("Level", - 96, 12, -96),
			Menu("Follower", "RMS", "Average", "Peak", "Test")
		};
	}

	// Apply processing (called once per sample)
	void process() {
		const param gain = controls[0];
		
		switch((int)controls[2]){
		case 0: in * gain * 2 >> out >> rms >> debug;		break;
		case 1: in * gain * 2 >> out >> window >> debug; 	break;
		case 2: in * gain * 2 >> out >> peak >> debug;		break;
		case 3: in * gain * 2 >> out >> test >> debug;		break;
		}
		
		Amplitude(debug) -> dB >> controls[1];	
		out *= 0.1; // strink output so envelope (debug) can be seen
	}
};