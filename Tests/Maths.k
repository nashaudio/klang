
#include <klang.h>
#include <klang/tests.h>

using namespace klang::optimised;

//static float square(float x){ return power(x, 2); }

inline double pow_d(double x, double y){ return pow(x, y);   }
inline float pow_f(float x, float y){ return pow(x, y);   }
inline float pow_i(float x, int y) { return pow(x, y); }

inline float square(float x){ return x * x; }
inline float cube(float x){ return x * x * x; }

//float power_f(float x, float y){  return power(x, y);  }
//float power_i(float x, int y) { return power(x, y); }

struct Maths : Synth {

	struct MyNote : public Note {
		Triangle osc;
		ADSR adsr;

		event on(Pitch pitch, Amplitude velocity) { 
			const param f = pitch > Type::Frequency;
			osc(f, 0);
			adsr(0.001, 0, 1, 0.25);
		}

		event off(Amplitude velocity){
			adsr.release();
		}

		void process() {
			osc * adsr++ >> out;
			if (adsr.finished())
				stop();
				
			out * 0.5 >> debug;
		}
	};
	
	
	#define CHECK(func) debug.print("%-14s = %d\n", #func, func);

	// Initialise plugin (called once at startup)
	Maths() {		
		controls = { 
			// UI controls and parameters
		};
		
//		
//		
//		volatile float x = pi;
//		volatile int i = random(5,10);
//		volatile float f = random(5,10);
//		CHECK(power(x, 0));
//		CHECK(power(x, 1));
//		CHECK(power(x, 5));
//		CHECK(power(x, 1.f));
//		CHECK(power(x,-1));		
//		CHECK(power(x, i));
//		CHECK(power(x,-i));
//		CHECK(power(x, f));
//		CHECK(power(x,-f));
//		CHECK(square(x));
//		CHECK(cube(x));

		debug.print("%f\n", power(10, 2.5));
		debug.print("%f\n", power(10.f, 2.5));

		test::test("power", pow_f,power<float,float>);
	
		//klang::test::test("pow/int", pow_f, power_f);
		debug.print("pow(f) = %fms\n", test::profile(pow_f));
		debug.print("pow(d) = %fms\n", test::profile(pow_d));
		debug.print("pow(i) = %fms\n", test::profile(pow_i));
		debug.print("power(f,i) = %fms\n", test::profile(power<float,int>));
		debug.print("power(f,f) = %fms\n", test::profile(power<float,float>));
		debug.print("power(d,d) = %fms\n", test::profile(power<double,double>));
		
		debug.print("power(f,2) = %fms\n", test::profile(power<float,int>, 2));
		debug.print("square = %fms\n", test::profile(square));
		
		debug.print("power(f,3) = %fms\n", test::profile(power<float,int>, 3));
		debug.print("cube = %fms\n", test::profile(cube));
		
//		debug.print("\n");
//		PROFILE(power<float,int>, x, 0);
//		PROFILE(power<float,float>, x, 0.f);
//		PROFILE(power<float,int>, x, i);
//		PROFILE(power<float,float>, x, f);
//		PROFILE(cube, x);
//		PROFILE(square, x);
//		PROFILE(pow_i, x, 0);
//		PROFILE(pow_f, x, 0.f);	
//		PROFILE(pow_i, x, i);
//		PROFILE(pow_f, x, f);
//		debug.print("\n");
//		PROFILE(pow_i, x, 2);
//		PROFILE(pow_f, x, 2.f);
//		PROFILE(power<float,int>, x, 2);
//		PROFILE(power<float,float>, x, 2.f);
//		debug.print("\n");
//		PROFILE(pow_i, x, -1);
//		PROFILE(pow_f, x, -1.f);
//		PROFILE(power<float,int>, x, -1);
//		PROFILE(power<float,float>, x, -1.f);
//		debug.print("\n");
//		PROFILE(pow_i, x, 1);
//		PROFILE(pow_f, x, 1.f);
//		PROFILE(power<float,int>, x, 1);
//		PROFILE(power<float,float>, x, 1.f);

		notes.add<MyNote>(32);
	}
}; 