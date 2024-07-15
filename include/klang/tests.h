#pragma once

#include "../klang.h"
#include <cstdarg>
#include <chrono>

namespace klang {
	using namespace optimised;

	struct profile {
		void start() {
			//std::time
		}
	};
	
	namespace test {
		using klang::random;
		template<typename TYPE>
		static TYPE random() { return random(std::numeric_limits<TYPE>::min(), std::numeric_limits<TYPE>::max()); }

		static void fail(const char* format, ...) {
			static char buffer[1024] = { "klang: "};  // Static buffer for formatted string
			va_list args;                     // Initialize the variadic argument list

			va_start(args, format);           // Start variadic argument processing
			vsnprintf(buffer, 1024, format, args);  // Safely format the string into the buffer
			va_end(args);                     // Clean up the variadic argument list

			debug.print("klang: %s\n", buffer);
//			__debugbreak();
		}

		template<typename TYPE> struct all {
			static constexpr TYPE min = std::numeric_limits<TYPE>::min();
			static constexpr TYPE max = std::numeric_limits<TYPE>::max();
			static constexpr TYPE arg2 = max;
		};

		template<typename TYPE> struct positive {
			static constexpr TYPE min = TYPE(0);
			static constexpr TYPE max = std::numeric_limits<TYPE>::max();
			static constexpr TYPE arg2 = max;
		};

		template<typename TYPE>	struct upto1000 {							
			static constexpr TYPE min = TYPE(0);	
			static constexpr TYPE max = TYPE(1000);	
			static constexpr TYPE arg2 = max;		
		};

		template<typename TYPE>	struct upto100000 {
			static constexpr TYPE min = TYPE(0);
			static constexpr TYPE max = TYPE(100000);
			static constexpr TYPE arg2 = max;
		};

		template<typename TYPE>	struct upto1000000 {
			static constexpr TYPE min = TYPE(0);
			static constexpr TYPE max = TYPE(1000000);
			static constexpr TYPE arg2 = max;
		};

		template<typename TYPE> struct radians {
			static constexpr TYPE min = TYPE(0);
			static constexpr TYPE max = TYPE(2.0 * 3.1415926535897932384626433832795);
			static constexpr TYPE arg2 = max;
		};

		template<typename TYPE> struct one {
			static constexpr TYPE min = TYPE(0);
			static constexpr TYPE max = TYPE(1.0);
			static constexpr TYPE arg2 = max;
		};

		template<int X, typename TYPE> struct only {
			static constexpr TYPE min = TYPE(X);
			static constexpr TYPE max = TYPE(X);
			static constexpr TYPE arg2 = max;
		};
		//template<int X, typename TYPE> struct only1 {
		//	static constexpr TYPE min = TYPE(X);
		//	static constexpr TYPE max = TYPE(X);
		//	static constexpr TYPE arg2 = max;
		//};

		constexpr int MAX_TOLERANCE = 19;
		constexpr double TOLERANCES[MAX_TOLERANCE+1] = { 0,1E-1,1E-2,1E-3,1E-4,1E-5,1E-6,1E-7,1E-8,1E-9,1E-10,1E-11,1E-12,1E-13,1E-14,1E-15,1E-16,1E-17,1E-18,1E-19 };
		constexpr const char* TOLERANCE_STRINGS[MAX_TOLERANCE+1] = { "0","0.1","0.01","0.001","0.0001","0.00001","0.000001","0.0000001","0.00000001","0.000000001","0.0000000001","0.00000000001","0.000000000001","0.0000000000001","0.00000000000001","0.000000000000001","0.0000000000000001","0.00000000000000001","0.000000000000000001","0.0000000000000000001" };

		template<typename T, template<typename> class TEST = all, int TOLERANCE = -1>
		static const T* compare(T(*func1)(T), T(*func2)(T)) {
			constexpr T tolerance = T(TOLERANCES[TOLERANCE]);
			static T x = 0;
			for (int t = 0; t < 1000; t++) {
				x = random<T>(TEST<T>::min, TEST<T>::max);
				const double y1 = func1(x);
				const double y2 = func2(x);
				if (tolerance == 0 && (y1 != y2))
					return &x; // failed for f(x) - no tolerance
				else if(abs(y1 - y2) > tolerance)
					return &x; // failed for f(x) - above tolerance
			}
			return nullptr; // success
		};

		template<typename T, template<typename> class TEST = all, int TOLERANCE = 0>
		static const T* compare(T(*func1)(T,T), T(*func2)(T,T)) {
			constexpr T tolerance = T(TOLERANCES[TOLERANCE]);
			static T x = 0;
			for (int t = 0; t < 1000; t++) {
				x = random<T>(TEST<T>::min, TEST<T>::max);
				const double y1 = func1(x, TEST<T>::arg2);
				const double y2 = func2(x, TEST<T>::arg2);
				if (tolerance == 0 && (y1 != y2))
					return &x; // failed for f(x) - no tolerance
				else if (std::abs(y1 - y2) > tolerance)
					return &x; // failed for f(x) - above tolerance
			}
			return nullptr; // success
		};

		// find the minimum tolerance for two functions are equivalent
		template<typename T, template<typename> class TEST = all, int t = MAX_TOLERANCE>
		static int tolerance(T(*func1)(T), T(*func2)(T)) {
			if (!compare<T, TEST, t>(func1, func2))
				return t;
			if constexpr (t > 0)  // Continue until t == 0 
				return tolerance<T, TEST, t - 1>(func1, func2);
			return 0;
		}

		// find the minimum tolerance for two functions are equivalent
		template<typename T, template<typename> class TEST = all, int t = MAX_TOLERANCE>
		static int tolerance(T(*func1)(T, T), T(*func2)(T, T)) {
			if (!compare<T, TEST, t>(func1, func2))
				return t;
			if constexpr (t > 0)  // Continue until t == 0
				return tolerance<T, TEST, t - 1>(func1, func2);
			return 0;
		}

		template<typename T, template<typename> class TEST = all, int TOLERANCE = 0>
		static bool test(const char* name, T(*func1)(T), T(*func2)(T)) {
			if (const T* error = compare<T, TEST, TOLERANCE>(func1, func2)) {
				const T y1 = func1(*error), y2 = func2(*error);										// retrieve failed comparison
				const int t = tolerance<T, TEST, TOLERANCE == 0 ? 16 : TOLERANCE - 1>(func1, func2);	// widen tolerance possible?
				if (t)
					fail("%s(%f) test failed\n    %.16f\n != %.16f\n -> accurate to tolerance %d(%s), not %d(%s)\n", name, *error, y1, y2, t, TOLERANCE_STRINGS[t], TOLERANCE, TOLERANCE_STRINGS[TOLERANCE]);
				else
					fail("%s(%f) test failed\n    %.16f\n != %.16f\n -> no feasible tolerance versus expected %d(%s)\n", name, *error, y1, y2, TOLERANCE, TOLERANCE_STRINGS[TOLERANCE]);
				return false;
			}
			return true;
		}

		template<typename T, template<typename> class TEST = all, int TOLERANCE = 0>
		static bool test(const char* name, T(*func1)(T,T), T(*func2)(T,T)) {
			if (const T* error = compare<T, TEST, TOLERANCE>(func1, func2)) {
				const T y1 = func1(*error, TEST<T>::arg2);
				const T y2 = func2(*error, TEST<T>::arg2);
				const int t = tolerance<T, TEST, TOLERANCE == 0 ? 16 : TOLERANCE - 1>(func1, func2); // widen tolerance?
				if (t)
					fail("%s(%f) test failed\n    %.16f\n != %.16f\n -> accurate to tolerance %d(%s), not %d(%s)\n", name, *error, y1, y2, t, TOLERANCE_STRINGS[t], TOLERANCE, TOLERANCE_STRINGS[TOLERANCE]);
				else
					fail("%s(%f) test failed\n    %.16f\n != %.16f\n -> no feasible tolerance versus expected %d(%s)\n", name, *error, y1, y2, TOLERANCE, TOLERANCE_STRINGS[TOLERANCE]);
				return false;
			}
			return true;
		}

		inline float fmodf_1(float x) { return fmodf(x, 1.f);		   }
		inline double fmod_1(double x) { return fmod(x, 2.0);          }
		inline float fmodf_2pi(float x) { return fmodf(x, 2.f * pi.f); }
		inline double fmod_2pi(double x) { return fmod(x, 2.0 * pi.d); }

		template<typename T, template<typename> class TEST = all>
		inline double profile(T(*func1)(T)) {
			std::array<T, 1000> x;
			for (int t = 0; t < 1000; t++)
				x[t] = random<T>(TEST<T>::min, TEST<T>::max);

			auto start = std::chrono::high_resolution_clock::now();
			T y = 0.0;
			for (int t = 0; t < 1000; t++) {
				y += func1(x[t]);
			}
			auto stop = std::chrono::high_resolution_clock::now();
			return std::chrono::duration_cast<std::chrono::nanoseconds>(stop - start).count() / 1000000.0;
		}

		template<typename T, template<typename> class TEST = all>
		inline double profile(T(*func1)(T, T)) {
			std::array<T, 1000> x;
			for (int t = 0; t < 1000; t++)
				x[t] = random<T>(TEST<T>::min, TEST<T>::max);

			auto start = std::chrono::high_resolution_clock::now();
			T y = 0.0;
			for (int t = 0; t < 1000; t++) {
				y += func1(x[t], TEST<T>::arg2);
			}
			auto stop = std::chrono::high_resolution_clock::now();
			return std::chrono::duration_cast<std::chrono::nanoseconds>(stop-start).count() / 1000000.0;
		}

		template<typename T1, typename T2, template<typename> class TEST = all>
		inline double profile(T1(*func1)(T1, T2)) {
			std::array<T1, 1000> x;
			std::array<T2, 1000> y;
			for (int t = 0; t < 1000; t++) {
				x[t] = random<T1>(TEST<T1>::min, TEST<T1>::max);
				y[t] = random<T1>(TEST<T2>::min, TEST<T2>::max);
			}
			auto start = std::chrono::high_resolution_clock::now();
			T1 sum = 0.0;
			for (int t = 0; t < 1000; t++) {
				sum += func1(x[t], y[t]);
			}
			auto stop = std::chrono::high_resolution_clock::now();
			return std::chrono::duration_cast<std::chrono::nanoseconds>(stop - start).count() / 1000000.0;
		}

		template<typename T1, typename T2, template<typename> class TEST = all>
		inline double profile(T1(*func1)(T1, T2), T2 arg2) {
			std::array<T1, 1000> x;
			for (int t = 0; t < 1000; t++) {
				x[t] = random<T1>(TEST<T1>::min, TEST<T1>::max);
			}
			auto start = std::chrono::high_resolution_clock::now();
			T1 sum = 0.0;
			for (int t = 0; t < 1000; t++) {
				sum += func1(x[t], arg2);
			}
			auto stop = std::chrono::high_resolution_clock::now();
			return std::chrono::duration_cast<std::chrono::nanoseconds>(stop - start).count() / 1000000.0;
		}

		static void run() {
			// signal tests
			for (int t = 0; t < 1000; t++) {
				const float x = random<float>();
				const signal s = x;
				const float x2 = s;
				if (x != x2)
					fail("klang::signal tests failed (1) - %f != %f\n", x, x2);
			}

			// param tests
			for (int t = 0; t < 1000; t++) {
				const float x = random<float>();
				const param s = x;
				const float x2 = s;
				if (x != x2) {
					fail("klang::param tests failed (1) - %f != %f\n", x, x2);
					break;
				}
			}

			// phase tests



			// oscillators tests
			
			//if (error = compare<float, radians>(sinf, Oscillators::Fast::fastsin)) {
			//	fail("fastsin test failed at f($x) %f != %f\n", sinf(*error), Oscillators::Fast::fastsin(*error));
			//}

			// fast_mod == fmodf for 0 to 2pi with tolerance 0.000001
			//test<float, one, 6>("fast_mod<float>", fmodf, fast_mod);
			//test<double, one, 16>("fast_mod<double>", fmod, fast_mod);

			//debug.print("profile: fast_mod<double, one> = %.4fms vs  fmod = %.4fms\n", profile<double, radians>(fast_mod), profile<double, radians>(fmod));
			//debug.print("profile: fast_mod<double, one> = %.4fms vs  fmod = %.4fms\n", profile<double, radians>(fast_mod), profile<double, radians>(fmod));

			//test<float, upto1000, 6>("fast_mod1<float>", fmodf_1, fast_mod1);
			//test<double, upto1000, 16>("fast_mod1<double>", fmod_1, fast_mod1);

			debug.print("profile: fast_mod1<float,  upto1000> = %.4fms vs fmodf_1 = %.4fms\n", profile<float, upto1000>(fast_mod1), profile<float, upto1000>(fmodf_1));
			debug.print("profile: fast_mod1<double, upto1000> = %.4fms vs  fmod_1 = %.4fms\n", profile<double, upto1000>(fast_mod1), profile<double, upto1000>(fmod_1));

			//test<float, upto1000, 6>("fast_mod2pi<float>", fmodf_2pi, fast_mod2pi);
			//test<double, upto1000, 16>("fast_mod2pi<double>", fmod_2pi, fast_mod2pi);

			debug.print("profile: fast_mod2pi<float,  upto1000> = %.4fms vs fmodf_2pi = %.4fms\n", profile<float, upto1000>(fast_mod2pi), profile<float, upto1000>(fmodf_2pi));
			debug.print("profile: fast_mod2pi<double, upto1000> = %.4fms vs  fmod_2pi = %.4fms\n", profile<double, upto1000>(fast_mod2pi), profile<double, upto1000>(fmod_2pi));

			test<float, upto1000, 3>("fast_mod<float>", fmodf, fast_mod);
			test<double, upto1000, 16>("fast_mod<double>", fmod, fast_mod);

			debug.print("profile: fast_mod<float,  upto1000> = %.4fms vs fmodf = %.4fms\n", profile<float, upto1000>(fast_mod), profile<float, upto1000>(fmodf));
			debug.print("profile: fast_mod<double, upto1000> = %.4fms vs  fmod = %.4fms\n", profile<double, upto1000>(fast_mod), profile<double, upto1000>(fmod));

			//test<float, upto100000, 1>("fast_mod<float>", fmodf, fast_mod);
			test<double, positive, 16>("fast_mod<double>", fmod, fast_mod);
			debug.print("profile: fast_mod<double, positive> = %.4fms vs  fmod = %.4fms\n", profile<double, positive>(fast_mod), profile<double, positive>(fmod));

			//for (int t = 0; t < 1000; t++) {
			//	const double x = random<double>(0, 2 * pi.d);
			//	double m = fmod(x, 2 * pi.d);
			//	double m2 = fast_mod(x, 2 * pi.d);
			//	phase<double, unsigned long long> p = x;
			//	double mp = p;
			//	if (m != mp) {
			//		fail("klang::param tests failed (1) - %f != %f (%f)\n", m, mp, m2);
			//		break;
			//	}
			//}

			//float phase = random<float>();
			//Phase p = phase;
			//for (int t = 0; t < 1000; t++) {
			//	const float increment = random<float>();
			//	phase = fmodf(phase + increment, 2 *pi);
			//	p += increment;
			//	if (phase != p) {
			//		fail("klang::param tests failed (1) - %f != %f\n", phase, float(p));
			//		break;
			//	}
			//}
		}
	};
};