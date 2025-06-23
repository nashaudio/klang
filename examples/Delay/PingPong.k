#include <klang.h>
using namespace klang::optimised;

struct PingPong : Stereo::Effect {
	Stereo::Delay<192000> delay;

	PingPong() {
		controls = { 
			{ "Left",  Dial("Time"),
			           Dial("Gain", 0, 0.99) },
			{ "Right", Dial("Time"),
			           Dial("Gain", 0, 0.99) }
		};
	}

	void process() {
		stereo::signal time = { controls[0], controls[2] };
		stereo::signal gain = { controls[1], controls[3] };
		
//		in.l >> delay.l;
		in.l + delay.l(time.l * fs) * gain.l >> out.l;
		delay.l << out.r;

//		in.r >> delay.r;
		in.r + delay.r(time.r * fs) * gain.r >> out.r;
		delay.r << out.l;
	}
};