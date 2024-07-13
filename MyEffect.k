#include <klang.h>
using namespace klang::basic;

float square(float x){
	return x*x;
}

struct Follower1 : Modifier {
	Butterworth::LPF filter;
	signal level = 0;
	
	
	Follower1() { filter.set(1); }
	
	void process() {
		in >> square >> filter >> sqrt >> out;
	}
};

template<int WINDOW>
struct Follower : Modifier
{
	buffer buffer;
	signal sum = 0;
	static constexpr constant window = { WINDOW };
	Follower() : buffer(WINDOW, 0) { }
	
	void process()
	{
		sum -= buffer;
		in * in >> buffer;
		sum += buffer++;
		if(buffer.finished())
			buffer.rewind();
		sum * window.inv >> sqrt >> out;
	}
};

struct Peak : Modifier {
	param attack;
	param release;
	
	param A,R;
	
	Peak() { set(0.01f, 0.1f); }
	
    void set(param attack, param release) {
    	if(Peak::attack != attack || Peak::release != release){
        	A = attack == 0.f ? 1.f : std::exp(-1.0f / (fs * attack));
        	R = release == 0.f ? 1.f : std::exp(-1.0f / (fs * release));
        }
    }

    void process() {
        const float in = abs(Peak::in);
        const float delta = out - in;
        if(in > out) 	// attacking
        	in + A * delta >> out;
        else 			// releasing
        	in + R * delta >> out;
    }
};

struct MyEffect : Effect {

	Sine lfo;
	Peak level;	
	//Follower<2000> level;	
	Delay<44100> delay;

	// Initialise plugin (called once at startup)
	MyEffect() {
		//square >> graph;
	
		controls = { 
			Dial("Gain", 0.0, 1.0, 0.5),
			Meter("Level")
		};

		presets = { 
			{ "Copied Preset", { 0.500 } },
		};
	}

	// Prepare for processing (called once per buffer)
	void prepare() {
		//char* ptr = nullptr;
		//lfo.set(*ptr);		
		//delay.set(22050);
	}

	// Apply processing (called once per sample)
	void process() {
		//0.5 >> debug;
		param gain = controls[0];
		
		//in + (in >> delay) >> out;
		
		in * gain * 2 >> out >> level >> controls[1] >> debug;
	}
};