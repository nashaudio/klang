
#include <klang.h>
//#include <klang/tests.h>

using namespace klang::optimised;


double myFunction1(double x){ return sin(x * 2 * pi / 128); }
//const Table<double, 128> sin_lookup1 = myFunction1;

void myFunction2(double x, double& y){ y = sin(x * 2 * pi / 128); }
//const Table<double, 128> sin_lookup2 = myFunction2;

//#define FUNCTION(type) (void(*)(type, type&))[](type x, type& y)
//#define TABLE(name, type, size) Table<type, size> name = FUNCTION(type)
//#define F(type) (void(*)(type, type&))[](type x, type& y)/
	
const Table<double, 128> sin_lookup = FUNCTION(double) { y = sin(x * 2 * pi / 128); };

//const TABLE(sin_lookup2, double, 128) { y = sin(x * 2 * pi / 128); };

const Table<double, 4> val_lookup = { 0, 1, 2, 3 };



struct FM : Synth {

	struct MyNote : Note {
		// Declare components / variables
		
		Operator<Sine> op1, op2, op3;
		param fc;
		ADSR adsr;

		// Note On
		event on(Pitch p, Velocity v) { 		
			graph.clear();
			for(int x=0; x<128; x++){
				graph.add(sin_lookup[x]);
			}
		
			const param fc = p -> Frequency;
			const Frequency fd = fc * controls[0];
			
			op1.set(fd,0);
			op1 = { {0,0}, {3,1} };
			
			op2.set(fd,0);
			op2 = { {0,1.5}, {3,0.5} };
			
			op3.set(fc,0);
			
			adsr(controls[3],0.1,1,1);
		}
		
		event off(Velocity v){
			adsr.release();
		}

		// Apply processing (called once per sample)
		void process() {

			const param I1 = controls[1];
			const param I2 = controls[2];
					
			op1 * I1 >> op2 * I2 >> op3 >> out;
							
			out *= adsr++ * 0.1f;
			if(adsr.finished())
				stop();
		}
	};

	// Initialise plugin (called once at startup)
	FM() {		
		//test::run();
		
		controls = { 
			// UI controls and parameters
			Dial("Mod Freq", 0.001, 10.0, 1.0),
			Dial("Mod Index 1", 0.000, 10.0, 0.37),
			Dial("Mod Index 2", 0.000, 10.0, 0.37),
			Dial("Attack", 0.000, 1.0, 0.5),
		};

		presets = {
			
		};

		notes.add<MyNote>(32);
	}
};