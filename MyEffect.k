#include <klang.h>
using namespace klang::optimised;

struct MyEffect : Effect {

	Envelope::Follower rms;
	Envelope::Follower peak;
	Envelope::Follower::Window<2000> window;
	Envelope::Follower::Window<2000> mean;

	// Initialise plugin (called once at startup)
	MyEffect() {
		peak = Peak;
		mean = Mean;
	
		controls = { 
			Dial("Gain4", 0.0, 1.0, 0.5),
			Meter("Level", 0, 1, 0),
			Menu("Follower", "RMS", "Window", "Peak", "Mean")
		};
	}

	// Apply processing (called once per sample)
	void process() {
		const param gain = controls[0];
		
		const signal input = in * gain * 2;
		const signal _rms = input >> rms;
		const signal _window = input >> window;
		const signal _peak = input >> peak;
		const signal _mean = input >> mean;
		
		
//		const signal dp = _peak2 - _rms + 0.5;
		
		switch((int)controls[2]){
		case 0: _rms >> debug;		break;
		case 1: _window >> debug; 	break;
		case 2: _peak >> debug;		break;
		case 3: _mean >> debug;		break;
//		case 4: dp >> debug;		break;
		}
		
		Amplitude(debug) >> controls[1];	
		out = input * 0.1; // strink output so envelope (debug) can be seen
	}
};