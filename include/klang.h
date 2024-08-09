#pragma once

#include <stdio.h>
#include <stdlib.h>
#include <cmath>
#include <limits>
#include <assert.h>
#include <array>
#include <memory>
#include <vector>
#include <string>
#include <cstdarg>
#include <algorithm>
#include <type_traits>
#include <mutex>
#include <functional>

#include <float.h>

// provide access to original math functions through std:: prefix
namespace std {
	namespace klang {
		template<typename TYPE> TYPE sqrt(TYPE x) { return ::sqrt(x); }
		template<typename TYPE> TYPE abs(TYPE x) { return ::abs(x); }
	}
};

#define IS_SIMPLE_TYPE(type)																					\
	static_assert(!std::is_polymorphic_v<type>, "signal has virtual table");									\
	static_assert(!std::has_virtual_destructor_v<type>, "signal has virtual destructor");						\
	static_assert(std::is_trivially_copyable_v<type>, "signal is not trivially copyable");						\
	static_assert(std::is_trivially_copy_assignable_v<type>, "signal is not trivially copy assignable");		\
	static_assert(std::is_trivially_copy_constructible_v<type>, "signal is not trivially copy assignable");		\
	static_assert(std::is_trivially_destructible_v<type>, "signal is not trivially copy assignable");			\
	static_assert(std::is_trivially_move_assignable_v<type>, "signal is not trivially copy assignable");		\
	static_assert(std::is_trivially_move_constructible_v<type>, "signal is not trivially copy assignable");	

namespace klang {
	enum Mode { Peak, RMS, Mean };

	//template<typename Base, typename Derived>
	//struct is_base_of_any : std::false_type {};

	//template<typename Base, typename... Deriveds>
	//struct is_base_of_any<Base, std::tuple<Deriveds...>> : std::disjunction<std::is_base_of<Base, Deriveds>...> {};

	//template<typename Base, typename... Deriveds>
	//using is_base_of_any_t = typename is_base_of_any<Base, std::tuple<Deriveds...>>::type;

	#define DENORMALISE 1.175494e-38f

	struct constant {
		constexpr constant(double value) noexcept
			: d(value), f(static_cast<float>(value)), i(static_cast<int>(value)), inv(value == 0.0f ? 0.0f : static_cast<float>(1.0 / value)) { }

		const double d;
		const float f;
		const int i;
		const float inv;

		constexpr operator float() const noexcept { return f; }

		float operator^(float x) const { return std::pow(f, x); }
		float operator^(int x) const { return static_cast<float>(std::pow(d, x)); }
		float operator^(double x) const { return static_cast<float>(std::pow(d, x)); }
	};

	#define CONSTANT constexpr constant

#if __cplusplus == 201703L
	#pragma warning(disable:4996) // disable deprecated warning
	template <typename T>
	inline constexpr bool is_literal = std::is_literal_type_v<T>;
#else
	template <typename T>
	inline constexpr bool is_literal = std::is_trivially_constructible_v<T> && std::is_trivially_copyable_v<T> && std::is_trivially_destructible_v<T>;
#endif
	
	template<typename Base, typename Derived>
	constexpr bool is_derived_from() {
		return std::is_base_of_v<Base, Derived>;
	}

	template<typename Head, typename... Tail>
	constexpr bool are_scalars(Head&& head, Tail&&... tail) {
		using T = std::decay_t<decltype(head)>;
		return std::is_scalar_v<T> && ((sizeof... (Tail)) == 0 || are_scalars(std::forward<decltype(tail)>(tail)...));
	}

	template <typename BASE, int EXP>
	inline constexpr BASE poweri(BASE base) {
		BASE result = 1;
		constexpr bool negative = EXP < 0;
		int exp = negative ? -EXP : EXP;
		while (exp > 0) {
			if (exp % 2 == 1)
				result *= base;
			base *= base;
			exp >>= 1;
		}
		return negative ? 1 / result : result;
	}

	template <typename BASE, typename EXP>
	using power_t = typename std::conditional_t<std::is_integral_v<BASE>, float, BASE>;

	template <typename BASE, typename EXP, std::enable_if_t < is_literal<EXP>, bool>>
	inline constexpr power_t<BASE, EXP> power(BASE base, EXP exp) {
		if constexpr (std::is_integral_v<EXP>) {
			switch (exp) {
			case 0:  return (BASE)1;
			case 1:  return base;
			case 2:  return base * base;
			case 3:  return base * base * base;
			case 4:  return base * base * base * base;
			case -1: return (BASE)1 / base;
			case -2: return (BASE)1 / (base * base);
			case -3: return (BASE)1 / (base * base * base);
			case -4: return (BASE)1 / (base * base * base * base);
			default:
				return poweri<BASE, exp>(base);
			}
		} else if constexpr (std::is_floating_point_v<EXP>) {
			if constexpr (exp == (EXP)0)	   return 1;
			else if constexpr (exp == (EXP)1)  return base;
			else if constexpr (exp == (EXP)2)  return base * base;
			else if constexpr (exp == (EXP)3)  return base * base * base;
			else if constexpr (exp == (EXP)4)  return base * base * base * base;
			else if constexpr (exp == (EXP)-1) return 1 / base;
			else if constexpr (exp == (EXP)-2) return 1 / (base * base);
			else if constexpr (exp == (EXP)-3) return 1 / (base * base * base);
			else if constexpr (exp == (EXP)-4) return 1 / (base * base * base * base);
		}
		return (BASE)std::pow(base, exp);;
	}

	template <typename BASE, typename EXP>
	inline constexpr power_t<BASE, EXP> power(BASE base, EXP exp) {
		if constexpr (std::is_integral_v<BASE>) {
			return power(float(base), exp);
		} else if constexpr (std::is_integral_v<EXP>) {
			switch (exp) {
			case 0:  return (BASE)1;
			case 1:  return base;
			case 2:  return base * base;
			case 3:  return base * base * base;
			case 4:  return base * base * base * base;
			case -1: return (BASE)1 / base;
			case -2: return (BASE)1 / (base * base);
			case -3: return (BASE)1 / (base * base * base);
			case -4: return (BASE)1 / (base * base * base * base);
			}
		} else if constexpr (std::is_floating_point_v<EXP>) {
			if (base == (BASE)10)	 return (power_t<BASE, EXP>)::exp(exp * (EXP)2.3025850929940456840179914546843642076011014886287729760333279009);
			else if (exp == (EXP)0)	 return (power_t<BASE, EXP>)1;
			else if (exp == (EXP)1)  return base;
			else if (exp == (EXP)2)  return base * base;
			else if (exp == (EXP)3)  return base * base * base;
			else if (exp == (EXP)4)  return base * base * base * base;
			else if (exp == (EXP)-1) return (power_t<BASE, EXP>)1 / base;
			else if (exp == (EXP)-2) return (power_t<BASE, EXP>)1 / (base * base);
			else if (exp == (EXP)-3) return (power_t<BASE, EXP>)1 / (base * base * base);
			else if (exp == (EXP)-4) return (power_t<BASE, EXP>)1 / (base * base * base * base);
		}
		return power_t<BASE, EXP>(std::pow(base, exp));
	}

	template<typename TYPE1, typename TYPE2> inline TYPE1 min(TYPE1 a, TYPE2 b) { return a < b ? a : (TYPE1)b; };
	template<typename TYPE1, typename TYPE2> inline TYPE1 max(TYPE1 a, TYPE2 b) { return a > b ? a : (TYPE1)b; };

	constexpr constant pi =    { 3.1415926535897932384626433832795 };
	constexpr constant ln2 =   { 0.6931471805599453094172321214581 };
	constexpr constant root2 = { 1.4142135623730950488016887242097 };

	template<typename TYPE>
	static TYPE random(const TYPE min, const TYPE max) { return rand() * ((max - min) / (TYPE)RAND_MAX) + min; }
	static void random(const unsigned int seed) { srand(seed); }

	//static float sin(float phase) { return sinf(phase); }

	typedef void event;

	template<typename TYPE, int CAPACITY>
	struct Array {
		TYPE items[CAPACITY];
		unsigned int count = 0;

		static int capacity() { return CAPACITY; }
		unsigned int size() const { return count; }

		void add(const TYPE& item) {
			if(count < CAPACITY)
				items[count++] = item;
		}
		TYPE* add() {
			if (count < CAPACITY)
				return &items[count++];
			return nullptr;
		}
		void clear() { count = 0; }

		float max() const {
			float max = 0.f;
			for (unsigned int i = 0; i < count; i++)
				if (abs(items[i]) > max)
					max = items[i];
			return max;
		}

		float mean() const {
			float sum = 0.f;
			for (unsigned int i = 0; i < count; i++)
				sum += abs(items[i]);
			return sum / count;
		}

		float rms() const {
			float sum = 0.f;
			for (unsigned int i = 0; i < count; i++)
				sum += items[i] * items[i];
			return sqrt(sum / count);
		}

		void normalise(float target = 1.f, int mode = Peak) {
			if (!count) return;
			
			const float current = (mode == Peak) ? max() : (mode == Mean) ? mean() : rms();
			const float scale = current == 0.f ? 0.f : target / current;
			for (int i = 0; i < count; i++)
				items[i] *= scale;
		}
        
		TYPE& operator[](int index) { return items[index]; }
		const TYPE& operator[](int index) const { return items[index]; }

		//// Constructor that enables inline initialisation without adding a vtable
		//template<typename... Args, typename = std::enable_if_t<(sizeof...(Args) <= CAPACITY) && (std::is_constructible<TYPE, Args>::value && ...)>>
		//Array(Args&&... args) : items{ std::forward<Args>(args)... }, count(sizeof...(Args)) {}

		// Initializer list constructor to handle narrowing conversions
		Array(std::initializer_list<TYPE> init_list) {
			count = std::min(static_cast<unsigned int>(init_list.size()), static_cast<unsigned int>(CAPACITY));
			std::copy_n(init_list.begin(), count, items);
		}

		Array() = default; // Default constructor to allow empty initialization
	};

	template<int SIZE>
	struct Text {
		char string[SIZE + 1] = { 0 };
		int capacity() const { return SIZE; }

		operator const char* () const { return string; }
		const char* c_str() const { return string; }

		static Text from(const char* in) {
			Text text;
			text = in;
			return text;
		}

		void operator=(const char* in) {
			memcpy(string, in, SIZE);
			string[SIZE] = 0;
		}

		bool operator==(const char* in) const {
			return strcmp(string, in) == 0;
		}

		bool operator!=(const char* in) const {
			return !operator==(in);
		}
	};

	typedef Text<32> Caption;

	struct Output;

	struct relative;

	// a lightweight class (=== float) for passing signals between audio processing functions
	struct signal {
		float value;

		// signals can be created from any literal number / scalar type
		signal(constant initial) : value(initial.f) { }
		signal(const float initial = 0.f) : value(initial) { }
		signal(const double initial) : value((const float)initial) { }
		signal(const int value) : value((const float)value) { }

		// feedback operator (prevents further processing)
		const signal& operator<<(const signal& input) {
			value = input;
			return *this;
		}

		// feedforward operator (allows further processing)
		signal& operator>>(signal& destination) const {
			destination.value = value;
			return destination;
		}

		//signal& operator=(const signal& in) { value = in; return *this; };
		signal& operator=(Output& in);  // e.g. out = ramp		(these operator trigger processing of the Output)
		signal& operator+=(Output& in); // e.g. out += ramp

		signal& operator+=(const signal& x) { value += x.value; return *this; }
		signal& operator-=(const signal& x) { value -= x.value; return *this; }
		signal& operator*=(const signal& x) { value *= x.value; return *this; }
		signal& operator/=(const signal& x) { value /= x.value; return *this; }

		signal& operator+=(float x) { value += x; return *this; }
		signal& operator-=(float x) { value -= x; return *this; }
		signal& operator*=(float x) { value *= x; return *this; }
		signal& operator/=(float x) { value /= x; return *this; }

		signal& operator+=(double x) { value += (float)x; return *this; }
		signal& operator-=(double x) { value -= (float)x; return *this; }
		signal& operator*=(double x) { value *= (float)x; return *this; }
		signal& operator/=(double x) { value /= (float)x; return *this; }

		signal& operator+=(int x) { value += (float)x; return *this; }
		signal& operator-=(int x) { value -= (float)x; return *this; }
		signal& operator*=(int x) { value *= (float)x; return *this; }
		signal& operator/=(int x) { value /= (float)x; return *this; }

		signal operator+(float x) const { return value + x; }
		signal operator-(float x) const { return value - x; }
		signal operator*(float x) const { return value * x; }
		signal operator/(float x) const { return value / x; }
		
		signal operator+(double x) const { return value + (float)x; }
		signal operator-(double x) const { return value - (float)x; }
		signal operator*(double x) const { return value * (float)x; }
		signal operator/(double x) const { return value / (float)x; }
		
		signal operator+(int x) const { return value + (float)x; }
		signal operator-(int x) const { return value - (float)x; }
		signal operator*(int x) const { return value * (float)x; }
		signal operator/(int x) const { return value / (float)x; }

		signal operator^(float x) const { return power(value, x); }
		signal operator^(double x) const { return power(value, x); }
		signal operator^(int x) const { return power(value, x); }

		operator const float() const {	return value; }
		operator float&() {				return value; }

		bool isDenormal() const {
			const unsigned int bits = *(const unsigned int*)&value;
			return !(bits & 0x7F800000) && (bits & 0x007FFFFF);
		}

		int channels() const { return 1; }

		relative operator+() const;	// unary + operator produces relative signal
		relative relative() const;	// 
	};

	// signal MUST compile as a float
	static_assert(sizeof(signal) == sizeof(float), "signal != float");
	IS_SIMPLE_TYPE(signal)

	// class representing a signal used to offset relative to another signal (e.g. phase modulation)
	struct relative : public signal { };
	inline relative signal::operator+() const { return { value }; }
	inline relative signal::relative() const { return { value }; }

	// allow literals / scalar types to be streamed into signals
	inline static signal& operator>>(float input, signal& destination) { // CHECK: should this be signal, rather than float?
		destination << signal(input);
		return destination;
	}

	// multi-channel signal (e.g. stereo)
	template<int CHANNELS = 2>
	struct signals {
		union {
			signal value[CHANNELS];
			struct { signal l, r; };
		};

		signal mono() const { return (l + r) * 0.5f; }

		signal& operator[](int index) { return value[index]; }
		const signal& operator[](int index) const { return value[index]; }

		signals(float initial = 0.f) : l(initial), r(initial) { }
		signals(double initial) : l((float)initial), r((float)initial) { }
		signals(int initial) : l((float)initial), r((float)initial) { }

		signals(float left, float right) : l(left), r(right) { }
		signals(double left, double right) : l((float)left), r((float)right) { }
		signals(int left, int right) : l((float)left), r((float)right) { }

		template <typename... Args, typename = std::enable_if_t<(std::is_convertible_v<Args, signal> && ...)>>
		signals(Args&... initial) : value{ initial... } { }

		template <typename... Args, typename = std::enable_if_t<(std::is_scalar_v<Args> && ...)>>
		signals(Args... initial) : value { initial... } { }

		int channels() const { return CHANNELS; }

		const signals& operator<<(const signals& input) {
			return operator=(input);
		}

		signals& operator>>(signals& destination) const {
			destination = *this;
			return destination;
		}

		signals& operator+=(const signals x) { for (int v = 0; v < CHANNELS; v++) value[v] += x[v]; return *this; }
		signals& operator-=(const signals x) { for (int v = 0; v < CHANNELS; v++) value[v] -= x[v]; return *this; }
		signals& operator*=(const signals x) { for (int v = 0; v < CHANNELS; v++) value[v] *= x[v]; return *this; }
		signals& operator/=(const signals x) { for (int v = 0; v < CHANNELS; v++) value[v] /= x[v]; return *this; }

		signals operator+(const signals x) const { signals s = *this; for (int v = 0; v < CHANNELS; v++) s[v] += x[v]; return s; }
		signals operator-(const signals x) const { signals s = *this; for (int v = 0; v < CHANNELS; v++) s[v] -= x[v]; return s; }
		signals operator*(const signals x) const { signals s = *this; for (int v = 0; v < CHANNELS; v++) s[v] *= x[v]; return s; }
		signals operator/(const signals x) const { signals s = *this; for (int v = 0; v < CHANNELS; v++) s[v] /= x[v]; return s; }

		//signals& operator+=(const signal x) { for (int v = 0; v < CHANNELS; v++) value[v] += x; return *this; }
		//signals& operator-=(const signal x) { for (int v = 0; v < CHANNELS; v++) value[v] -= x; return *this; }
		//signals& operator*=(const signal x) { for (int v = 0; v < CHANNELS; v++) value[v] *= x; return *this; }
		//signals& operator/=(const signal x) { for (int v = 0; v < CHANNELS; v++) value[v] /= x; return *this; }

		signals operator+(const signal x) const { signals s = *this; for (int v = 0; v < CHANNELS; v++) s[v] += x; return s; }
		signals operator-(const signal x) const { signals s = *this; for (int v = 0; v < CHANNELS; v++) s[v] -= x; return s; }
		signals operator*(const signal x) const { signals s = *this; for (int v = 0; v < CHANNELS; v++) s[v] *= x; return s; }
		signals operator/(const signal x) const { signals s = *this; for (int v = 0; v < CHANNELS; v++) s[v] /= x; return s; }

		signals operator+(float x) const { signals s = *this; for(int v = 0; v < CHANNELS; v++) s[v] += x; return s; }
		signals operator-(float x) const { signals s = *this; for(int v = 0; v < CHANNELS; v++) s[v] -= x; return s; }
		signals operator*(float x) const { signals s = *this; for(int v = 0; v < CHANNELS; v++) s[v] *= x; return s; }
		signals operator/(float x) const { signals s = *this; for(int v = 0; v < CHANNELS; v++) s[v] /= x; return s; }

		signals operator+(double x) const { signals s = *this; for (int v = 0; v < CHANNELS; v++) s[v] += x; return s; }
		signals operator-(double x) const { signals s = *this; for (int v = 0; v < CHANNELS; v++) s[v] -= x; return s; }
		signals operator*(double x) const { signals s = *this; for (int v = 0; v < CHANNELS; v++) s[v] *= x; return s; }
		signals operator/(double x) const { signals s = *this; for (int v = 0; v < CHANNELS; v++) s[v] /= x; return s; }

		signals operator+(int x) const { signals s = *this; for (int v = 0; v < CHANNELS; v++) s[v] += x; return s; }
		signals operator-(int x) const { signals s = *this; for (int v = 0; v < CHANNELS; v++) s[v] -= x; return s; }
		signals operator*(int x) const { signals s = *this; for (int v = 0; v < CHANNELS; v++) s[v] *= x; return s; }
		signals operator/(int x) const { signals s = *this; for (int v = 0; v < CHANNELS; v++) s[v] /= x; return s; }
	};

	template<int CHANNELS = 2> inline signals<CHANNELS> operator+(float x, const signals<CHANNELS>& y) { return y + x; }
	template<int CHANNELS = 2> inline signals<CHANNELS> operator-(float x, const signals<CHANNELS>& y) { return -y + x; }
	template<int CHANNELS = 2> inline signals<CHANNELS> operator*(float x, const signals<CHANNELS>& y) { return y * x; }
	template<int CHANNELS = 2> inline signals<CHANNELS> operator/(float x, const signals<CHANNELS>& y) { 
		signals<CHANNELS> s = { 0.f };
		for(int c=0; c<CHANNELS; c++)
			s[c] = x / y[c];
		return std::move(s);
	}

	// bounded increment (e.g. phase or wavetable)
	struct increment {
		float amount;
		const float size;

		increment(float amount, const float size = 2 * pi) : amount(amount), size(size) { }
		increment(float amount, int size) : amount(amount), size((float)size) { }
		increment(float amount, double size) : amount(amount), size((float)size) { }

		increment& operator=(float in) { amount = in; return *this;  }
	};

	struct Control;

	// signal used as a control parameter (possibly at audio rate)
	struct param : public signal {
		param(constant in) : signal(in.f) { }
		param(const float initial = 0.f) : signal(initial) { }
		param(const signal& in) : signal(in) { }
		param(signal& in) : signal(in) { }
		param(Control& in); // see Control declaration

		param& operator+=(const increment& increment) {
			value += increment.amount;
			if (value >= increment.size)
				value -= increment.size;
			return *this;
		}
	};

	// param should also be binary compatible with float / signal
	static_assert(sizeof(param) == sizeof(float), "param != float");

	// support left-to-right and right-to-left signal flow
	inline static param& operator>>(param& from, param& to) {
		to << from;
		return to;
	}

	struct params {
		param* parameters;
		const int size;

		params(param p) : parameters(new param[1]), size(1) { parameters[0] = p; }
		params(std::initializer_list<param> params) : parameters(new param[params.size()]), size((int)params.size()) {
			int index = 0;
			for(param p : params)
				parameters[index++] = p;
		}

		param& operator[](int index) { return parameters[index]; }
	};

	inline float fast_mod(float x, float y) {
		const unsigned int i = (((unsigned int)(x * (float(UINT_MAX) + 1.f) / y)) >> 9) | 0x3f800000;
		return (*(const float*)&i - 1.f) * y;
	}

	inline double fast_mod(double x, double y) {
		const unsigned long long i = (((unsigned long long)(x * (double(ULLONG_MAX) + 1.0) / y)) >> 12ULL) | 0x7FF0000000000000ULL;
		return (*(const double*)&i - 1.0) * y;
	}

	inline float fast_mod1(float x) {
		const unsigned int i = ((unsigned int)(x * (float(UINT_MAX) + 1.f)) >> 9) | 0x3f800000;
		return *(const float*)&i - 1.f;
	}

	inline double fast_mod1(double x) {
		const unsigned long long i = (((unsigned long long)(x * (double(ULLONG_MAX) + 1.0))) >> 12ULL) | 0x7FF0000000000000ULL;
		return (*(const double*)&i - 1.0);
	}

	inline float fast_mod2pi(float x) {
		constexpr float twoPi = float(2.0 * 3.1415926535897932384626433832795);
		const unsigned int i = (((unsigned int)(x * (float(UINT_MAX) + 1.f) / twoPi)) >> 9) | 0x3f800000;
		return (*(const float*)&i - 1.f) * twoPi;
	}

	inline float fast_modp(unsigned int x) {
		constexpr float twoPi = float(2.0 * 3.1415926535897932384626433832795);
		const unsigned int i = (x >> 9) | 0x3f800000;
		return (*(const float*)&i - 1.f) * twoPi;
	}

	inline double fast_mod2pi(double x) {
		constexpr double twoPi = (2.0 * 3.1415926535897932384626433832795);
		const unsigned long long i = (((unsigned long long)(x * (double(ULLONG_MAX) + 1.0) / twoPi)) >> 12ULL) | 0x7FF0000000000000ULL;
		return (*(const double*)&i - 1.0) * twoPi;
	}

	template<int SIZE>
	inline double fast_modi(double x) {
		constexpr double size = double(SIZE);
		constexpr double sizeInv = 1.0 / double(SIZE);
		const unsigned long long i = (((unsigned long long)(x * (double(ULLONG_MAX) + 1.0) * sizeInv)) >> 12ULL) | 0x7FF0000000000000ULL;
		return (*(const double*)&i - 1.0) * size;
	}

	//template<int X, int Y = X>
	struct Matrix {
		float v[4][4] = { 0 };

		float* operator[](int col) { return v[col]; }
		const float* operator[](int col) const { return v[col]; }

		float& operator()(int col, int row) { return v[col][row]; }
		float operator()(int col, int row) const { return v[col][row]; }

		signals<4> operator<<(const signals<4>& in) const {
			return { v[0][0] * in[0] + v[0][1] * in[1] + v[0][2] * in[2] + v[0][3] * in[3],
					 v[1][0] * in[0] + v[1][1] * in[1] + v[1][2] * in[2] + v[1][3] * in[3],
					 v[2][0] * in[0] + v[2][1] * in[1] + v[2][2] * in[2] + v[2][3] * in[3],
					 v[3][0] * in[0] + v[3][1] * in[1] + v[3][2] * in[2] + v[3][3] * in[3] };
		}
	};

	inline signals<4> operator*(const signals<4>& in, const Matrix& m) {
		return { m[0][0] * in[0] + m[0][1] * in[1] + m[0][2] * in[2] + m[0][3] * in[3],
				 m[1][0] * in[0] + m[1][1] * in[1] + m[1][2] * in[2] + m[1][3] * in[3],
				 m[2][0] * in[0] + m[2][1] * in[1] + m[2][2] * in[2] + m[2][3] * in[3],
				 m[3][0] * in[0] + m[3][1] * in[1] + m[3][2] * in[2] + m[3][3] * in[3] };
	}

	inline signals<4> operator>>(const signals<4>& in, const Matrix& m) { return operator*(in, m); }


	template<typename TYPE, typename _TYPE>
	struct phase {
		static constexpr TYPE twoPi = TYPE(2.0 * 3.1415926535897932384626433832795);

		// represent phase using full range of uint32
		// (integer math, no conditionals, free oversampling)
		_TYPE i = 0;

		// convert float [0, 2pi) to uint32
		phase(TYPE phase = 0) {
			if constexpr (std::is_same<TYPE, double>()) {
				constexpr TYPE FUINTMAX = TYPE(ULLONG_MAX + 1.0) / twoPi; // (float)INT_MAX + 1.f;
				phase *= FUINTMAX;
				i = (_TYPE)phase;
			} else if constexpr (std::is_same<TYPE, float>()) {
				constexpr TYPE FUINTMAX = TYPE(UINT_MAX + 1.0) / twoPi; // (float)INT_MAX + 1.f;
				phase *= FUINTMAX;
				i = (_TYPE)phase;
			}
		}

		// convert uint32 to float [0, 1)
		operator TYPE() const {
			const _TYPE phase = (i >> 12ULL) | 0x3FF0000000000000ULL;
			return (*(const TYPE*)&phase - TYPE(1.0)) * twoPi;
		}

		//static void test() {
		//	phase p;
		//	assert((float)(p = 0.f).radians() == 0.f);
		//	assert((float)(p = pi / 2).radians() == pi / 2);
		//	assert((float)(p = pi).radians() == pi);
		//	assert((float)(p = 3 * pi / 2).radians() == 3 * pi / 2);
		//	assert((float)(p = 2 * pi).radians() == 0.f);
		//}
	};

	struct Phase : public param {
		//INFO("Phase", 0.f, 0.f, 1.0f)
		using param::param;
		/*Phase(const float p = 0.f) : param(p) { };*/

		param& operator+=(float increment) {
			if (increment >= (2 * pi))
				return *this;
			value += increment;
			if (value > (2 * pi))
				value -= (2 * pi);
			return *this;
		}

		param& operator+=(const increment& increment) {
			if (increment.amount >= increment.size)
				return *this;
			value += increment.amount;
			if (value > increment.size)
				value -= increment.size;
			return *this;
		}

		Phase operator+(const increment& increment) const {
			if (increment.amount >= increment.size)
				return value;
			Phase value = Phase::value + increment.amount;
			if (value > increment.size)
				value -= increment.size;
			return value;
		}

		Phase operator%(float modulus) {
			return fast_mod(value, modulus);
		}
	};

	struct Frequency;
	struct Pitch : public param {
		//INFO("Pitch", 60.f, 0.f, 127.f)
		using param::param;
		
		//Pitch(float p = 60.f) : param(p) { };
		//Pitch(int p) : param((float)p) { };

		// convert note number to pitch class and octave (e.g. C#5)
		const char* text() const {
			thread_local static char buffer[32] = { 0 };
			const char* const notes[12] = { "C", "C#/Db", "D", "D#/Eb", "E", "F", "F#/Gb", "G", "G#/Ab", "A", "A#/Bb", "B" };
			snprintf(buffer, 32, "%s%d", notes[(int)value % 12], (int)value / 12);
			return buffer;
		}

		const Pitch* operator->() const {
			Frequency.pitch = value;
			return this;
		}

		thread_local static struct Convert {
			float pitch;
			operator param() {
				return 440.f * power(2.f, (pitch - 69.f) / 12.f);
			}
		} Frequency;

		template<typename TYPE>	Pitch operator+(TYPE in) { return value + in; }
		template<typename TYPE>	Pitch operator-(TYPE in) { return value - in; }
		template<typename TYPE>	Pitch operator*(TYPE in) { return value * in; }
		template<typename TYPE>	Pitch operator/(TYPE in) { return value / in; }
	};

	inline thread_local Pitch::Convert Pitch::Frequency;

	struct Frequency : public param { 
		//INFO("Frequency", 1000.f, -FLT_MAX, FLT_MAX)
		using param::param;
		Frequency(float f = 1000.f) : param(f) { };
	};

	static struct SampleRate {
		float f;
		int i;
		double d;
		float inv;
		float w;
		float nyquist;

		SampleRate(float sr) : f(sr), i(int(sr+0.001f)), d((double)sr), inv(1.f / sr), w(2.0f * pi * inv), nyquist(sr / 2.f) { }

		operator float() { return f; }
	} fs(44100); // sample rate

	struct Conversion : public signal {
		using signal::signal;
//		operator klang::signal& () { return signal; };
	};

	struct Amplitude;
	// gain (decibels)
	struct dB : public param {
		//INFO("dB", 0.f, -FLT_MAX, FLT_MAX)
		using param::param;

		dB(float gain = 0.f) : param(gain) { };

		const dB* operator->() {
			Amplitude = power(10, value * 0.05f);
			return this;
		}

		thread_local static Conversion Amplitude;
	};

	// amplitude (linear gain)
	struct Amplitude : public param {
		//INFO("Gain", 1.f, -FLT_MAX, FLT_MAX)
		using param::param;

		Amplitude(float a = 1.f) : param(a) { };
		Amplitude(const dB& db) {
			value = power(10, db.value * 0.05f); // 10 ^ (db * 0.05f);
		};

		//dB operator >> (dB) const {
		//	return 20.f * log10f(value);
		//}
		//operator dB() const {
		//	return 20.f * log10f(value);
		//}

		const Amplitude* operator->() const {
			dB = 20.f * log10f(value);
			return this;
		}

		thread_local static Conversion dB;
	};

	thread_local inline Conversion dB::Amplitude;
	thread_local inline Conversion Amplitude::dB;
	typedef Amplitude Velocity;

	struct Control
	{
		enum Type
		{
			NONE,	// no control (list terminator)
			ROTARY,	// rotary knob (dial/pot)
			BUTTON, // push button (trigger)
			TOGGLE, // on/off switch (toggle)
			SLIDER, // linear slider (fader)
			MENU,   // drop-down list (menu; up to 128 items)
			METER,  // level meter (read-only: use setParameter() to set value)
			WHEEL,  // MIDI control (Pitch Bend / Mod Wheel only)
		};

		struct Size
		{
			Size(int x = -1, int y = -1, int width = -1, int height = -1)
			: x(x), y(y), width(width), height(height) { }
        
			int x;
			int y;
			int width;
			int height;

			bool isAuto() const { return x == -1 && y == -1 && width == -1 && height == -1;  }
		};

		typedef Array<Caption, 128> Options;

		Caption name;           // name for control label / saved parameter
		Type type = NONE;       // control type (see above)
    
		float min;              // minimum control value (e.g. 0.0)
		float max;              // maximum control value (e.g. 1.0)
		float initial;          // initial value for control (e.g. 0.0)
    
		Size size;              // position (x,y) and size (height, width) of the control (use AUTO_SIZE for automatic layout)
		Options options;        // text options for menus and group buttons

		signal value;           // current control value
		signal smoothed;		// smoothed control value (filtered)

		operator signal& () { return value; }
		operator const signal&() const { return value; }
		operator param() const { return value; }
		operator float() const { return value.value; }

		static constexpr float smoothing = 0.999f;
		signal smooth() { return smoothed = smoothed * smoothing + (1.f - smoothing) * value; }

		operator Control*() { return this; }

		Control& set(float x) {
			value = std::clamp(x, min, max);
			return *this;
		}

		Control& operator+=(float x) { value += x; return *this; }
		Control& operator*=(float x) { value *= x; return *this; }
		Control& operator-=(float x) { value -= x; return *this; }
		Control& operator/=(float x) { value /= x; return *this; }

		template<typename TYPE> signal operator+(const Control& x) const { return value + x; }
		template<typename TYPE> signal operator*(const Control& x) const { return value * x; }
		template<typename TYPE> signal operator-(const Control& x) const { return value - x; }
		template<typename TYPE> signal operator/(const Control& x) const { return value / x; }

		template<typename TYPE> TYPE operator+(TYPE x) const { return value + x; }
		template<typename TYPE> TYPE operator*(TYPE x) const { return value * x; }
		template<typename TYPE> TYPE operator-(TYPE x) const { return value - x; }
		template<typename TYPE> TYPE operator/(TYPE x) const { return value / x; }

		template<typename TYPE> Control& operator<<(TYPE& in) { value = in; return *this;  }		// assign to control with processing
		template<typename TYPE> Control& operator<<(const TYPE& in) { value = in; return *this;  }	// assign to control without/after processing

		template<typename TYPE> TYPE& operator>>(TYPE& in) { return value >> in; }					// stream control to signal/object (allows processing)
		template<typename TYPE> const TYPE& operator>>(const TYPE& in) { return value >> in; }		// stream control to signal/object (no processing)
	};

	const Control::Size Automatic = { -1, -1, -1, -1 };
	const Control::Options NoOptions;

	struct ControlMap {
		Control* control;

		ControlMap() : control(nullptr) { };
		ControlMap(Control& control) : control(&control) { };

		operator Control&() { return *control; }
		operator signal& () { return control->value; }
		operator const signal&() const { return control->value; }
		operator param() const { return control->value; }
		operator float() const { return control->value; }
		signal smooth() { return control->smooth(); }

		template<typename TYPE> Control& operator<<(TYPE& in) { control->value = in; return *control; }		// assign to control with processing
		template<typename TYPE> Control& operator<<(const TYPE& in) { control->value = in; return *control; }	// assign to control without/after processing
	};

	IS_SIMPLE_TYPE(Control);

	inline param::param(Control& in) : signal(in.value) { }

	inline static Control Dial(const char* name, float min = 0.f, float max = 1.f, float initial = 0.f)
	{	return { Caption::from(name), Control::ROTARY, min, max, initial, Automatic, NoOptions, initial };	}

	inline static Control Button(const char* name)
	{	return { Caption::from(name), Control::BUTTON, 0, 1, 0.f, Automatic, NoOptions, 0.f };	}

	inline static Control Toggle(const char* name, bool initial = false)
	{	return { Caption::from(name), Control::TOGGLE, 0, 1, initial ? 1.f : 0.f, Automatic, NoOptions, initial ? 1.f : 0.f};	}

	inline static Control Slider(const char* name, float min = 0.f, float max = 1.f, float initial = 0.f)
	{	return { Caption::from(name), Control::SLIDER, min, max, initial, Automatic, NoOptions, initial }; }

	template<typename... Options>
	static Control Menu(const char* name, const Options... options)
	{	Control::Options menu;
		const char* strings[] = { options... };
		int nbValues = sizeof...(options);
		for(int p=0; p<nbValues; p++)
			menu.add(Caption::from(strings[p]));
		return { Caption::from(name), Control::MENU, 0, menu.size() - 1.f, 0, Automatic, menu, 0 }; 
	}

	inline static Control Meter(const char* name, float min = 0.f, float max = 1.f, float initial = 0.f)
	{	return { Caption::from(name), Control::METER, min, max, initial, Automatic, NoOptions, initial }; }

	inline static Control PitchBend()
	{	return { { "PITCH\nBEND" }, Control::WHEEL, 0.f, 16384.f, 8192.f, Automatic, NoOptions, 8192.f }; }

	inline static Control ModWheel()
	{	return { { "MOD\nWHEEL" }, Control::WHEEL, 0.f, 127.f, 0.f, Automatic, NoOptions, 0.f }; }

	struct Controls : Array<Control, 128>
	{
		float value[128] = { 0 };

		void operator+= (const Control& control) {
			items[count++] = control;
		}

		void operator= (const Controls& controls) {
			for (int c = 0; c < 128 && controls[c].type != Control::NONE; c++)
				operator+=(controls[c]);
		}

		void operator=(std::initializer_list<Control> controls) {
			for(auto control : controls)
				operator+=(control);
		}

		void add(const char* name, Control::Type type = Control::ROTARY, float min = 0.f, float max = 1.f, float initial = 0.f, Control::Size size = Automatic) {
			items[count].name = name;
			items[count].type = type;
			items[count].min = min;
			items[count].max = max;
			items[count].initial = initial;
			items[count].size = size;
			items[count++].value = initial;
		}	

		bool changed() {
			bool changed = false;
			for (unsigned int c = 0; c < count; c++) {
				if (items[c].value != value[c]) {
					value[c] = items[c].value;
					changed = true;
				}
			}
			return changed;
		}

		//float& operator[](int index) { return items[index].value; }
		//signal operator[](int index) const { return items[index].value; }

		//Control& operator()(int index) { return items[index]; }
		//Control operator()(int index) const { return items[index]; }
	};

	typedef Array<float, 128> Values;

	struct Preset {
		Caption name = { 0 };
		Values values;
	};

	//template<typename... Settings>
	//static Program Preset(const char* name, const Settings... settings)
	//{	Values values;
	//	float preset[] = { (float)settings... };
	//	int nbSettings = sizeof...(settings);
	//	for(int s=0; s<nbSettings; s++)
	//		values.add(preset[s]);
	//	return { Caption::from(name), values };
	//}

	struct Presets : Array<Preset, 128> {
		void operator += (const Preset& preset) {
			items[count++] = preset;
		}

		void operator= (const Presets& presets) {
			for (int p = 0; p < 128 && presets[p].name[0]; p++)
				operator+=(presets[p]);
		}

		void operator=(std::initializer_list<Preset> presets) {
			for(auto preset : presets)
				operator+=(preset);
		}

		template<typename... Values>
		void add(const char* name, const Values... values) {
			items[count].name = name;

			const float preset[] = { values... };
			int nbValues = sizeof...(values);
			for(int p=0; p<nbValues; p++)
				items[count].values.add(preset[p]);
			count++;
		}
	};
	
	class buffer {
	protected:
		constexpr unsigned int capacity(unsigned int n) {
			// Decrement n to handle the case where n is already a power of 2
			n--;
			n |= n >> 1;
			n |= n >> 2;
			n |= n >> 4;
			n |= n >> 8;
			n |= n >> 16;
			// Increment n to get the next power of 2
			n++;
			return n;
		}
		const unsigned int mask = 0xFFFFFFFF;
		const bool owned = false;
		float* samples;
		signal* ptr;
		signal* end;	
	public:
		typedef signal signal;
		const int size;

		buffer(float* buffer, int size)
		: samples(buffer), size(size), owned(false) { 
			rewind();
		}

		buffer(float* buffer, int size, float initial)
		: samples(buffer), size(size), owned(false) {
			rewind();
			set(initial);
		}

		buffer(int size, float initial = 0)
		: mask(capacity(size) - 1), owned(true), samples(new float[capacity(size)]), size(size) {
			rewind();
			set(initial);
		}

		virtual ~buffer() {
			if (owned)
				delete[] samples;
		}

		void rewind(int offset = 0) {
#ifdef _MSC_VER
			_controlfp_s(nullptr, _DN_FLUSH, _MCW_DN);
#else
			_controlfp(_DN_FLUSH, _MCW_DN); // flush denormals to zero
#endif
			ptr = (signal*)&samples[offset];
			end = (signal*)&samples[size];
		}

		void clear() {
			memset(samples, 0, sizeof(float) * size);
		}

		void clear(int size) {
			memset(samples, 0, sizeof(float) * (size < buffer::size ? size : buffer::size));
		}

		int offset() const {
			return int((signal*)&samples[0] - ptr);
		}

		void set(float value = 0) {
			if (value == 0)
				clear();
			else for (int s = 0; s < size; s++)
				samples[s] = value;
		}

		signal& operator[](int offset) {
			return *(signal*)&samples[offset & mask]; // owned array can't write beyond allocation
		}

		signal operator[](float offset) {
			return ((const buffer*)this)->operator[](offset);
		}

		signal operator[](float offset) const {
			const float f = floorf(offset);
			const float frac = offset - f;

			const int i = (int)offset;
			const int j = (i == (size - 1)) ? 0 : (i + 1);
			
			return samples[i] * (1.f - frac) + samples[j] * frac;
		}

		const signal& operator[](int index) const {
			return *(const signal*)&samples[index];
		}

		operator signal& () {
			return *ptr;
		}

		operator const signal& () const {
			return *ptr;
		}

		explicit operator double() const {
			return *ptr;
		}

		bool finished() const {
			return ptr == end;
		}

		signal& operator++(int) {
			return *ptr++;
		}

		signal& operator=(const signal& in) {
			return *ptr = in;
		}

		signal& operator+=(const signal& in) {
			*ptr += in;
			return *this;
		}

		signal& operator*=(const signal& in) {
			*ptr *= in;
			return *this;
		}

		buffer& operator=(const buffer& in) {
			assert(size == in.size);
			memcpy(samples, in.samples, size * sizeof(float));
			return *this;
		}

		buffer& operator<<(const signal& in) {
			*ptr = in;
			return *this;
		}

		const float* data() const { return samples; }
	};

	namespace Generic {
		template<typename SIGNAL>
		struct Input {
			SIGNAL in = { 0.f };

			// retrieve current input
			virtual const SIGNAL& input() const { return in; }

			// feedback input (include pre-processing, if any)
			virtual void operator<<(const SIGNAL& source) { in = source; input(); }
			virtual void input(const SIGNAL& source) { in = source; input(); }

		protected:
			// preprocess input (default: none)
			virtual void input() { }
		};

		//// support source >> destination
		//template<typename SIGNAL>
		//inline Input<SIGNAL>& operator>>(SIGNAL& source, Input<SIGNAL>& input) {
		//	input << source;
		//	return input;
		//}

		// an object that produces a processed output
		template<typename SIGNAL>
		struct Output {
			SIGNAL out = { 0.f };

			// returns previous output (without processing)
			virtual const SIGNAL& output() const { return out; }
			
			// pass output to destination (with processing)
			template<typename TYPE>
			TYPE& operator>>(TYPE& destination) { process(); return destination = out; }

			// returns output (with processing)
			virtual operator const SIGNAL&() { process(); return out; } // return processed output
			virtual operator const SIGNAL&() const { return out; } // return last output

			// arithmetic operations produce copies
			template<typename TYPE> SIGNAL operator+(TYPE& other) { process(); return out + (other); }
			template<typename TYPE> SIGNAL operator*(TYPE& other) { process(); return out * (other); }
			template<typename TYPE> SIGNAL operator-(TYPE& other) { process(); return out - (other); }
			template<typename TYPE> SIGNAL operator/(TYPE& other) { process(); return out / (other); }

		protected:
			// signal processing
			virtual void process() = 0;
		};

		//template<typename TYPE> inline signal operator+(TYPE other, Output& output) { return signal(output) + other; }
		//template<typename TYPE> inline signal operator*(TYPE other, Output& output) { return signal(output) * other; }
		//template<typename TYPE> inline signal operator-(TYPE other, Output& output) { return signal(other) - signal(output); }
		//template<typename TYPE> inline signal operator/(TYPE other, Output& output) { return signal(other) / signal(output); }

		// in-place arithmetic operations (CHECK: can these just be SIGNAL?)
		template<typename SIGNAL> inline SIGNAL operator+(float other, Output<SIGNAL>& output) { return SIGNAL(output) + other; }
		template<typename SIGNAL> inline SIGNAL operator*(float other, Output<SIGNAL>& output) { return SIGNAL(output) * other; }
		template<typename SIGNAL> inline SIGNAL operator-(float other, Output<SIGNAL>& output) { return SIGNAL(other) - SIGNAL(output); }
		template<typename SIGNAL> inline SIGNAL operator/(float other, Output<SIGNAL>& output) { return SIGNAL(other) / SIGNAL(output); }

		template<typename SIGNAL> inline SIGNAL operator+(Output<SIGNAL>& output, float other) { return SIGNAL(output) + other; }
		template<typename SIGNAL> inline SIGNAL operator*(Output<SIGNAL>& output, float other) { return SIGNAL(output) * other; }
		template<typename SIGNAL> inline SIGNAL operator-(Output<SIGNAL>& output, float other) { return SIGNAL(output) - other; }
		template<typename SIGNAL> inline SIGNAL operator/(Output<SIGNAL>& output, float other) { return SIGNAL(output) / other; }

		//inline signal operator+(Output& other, Output& output) { return signal(output) + signal(other); }
		//inline signal operator*(Output& other, Output& output) { return signal(output) * signal(other); }
		//inline signal operator-(Output& other, Output& output) { return signal(other) - signal(output); }
		//inline signal operator/(Output& other, Output& output) { return signal(other) / signal(output); }

		template<typename SIGNAL> 
		struct Generator : public Output<SIGNAL> { 
			using Output<SIGNAL>::out;

			// inline parameter(s) support
			template<typename... params>
			Output<SIGNAL>& operator()(params... p) {
				set(p...); return *this;
			}

			using Output<SIGNAL>::operator>>;
			using Output<SIGNAL>::process;
			operator const SIGNAL& () override { process(); return out; } // return processed output		
			operator const SIGNAL& () const override { return out; } // return last output		

		protected:
			// overrideable parameter setting (up to 8 parameters)
			virtual void set(param) { };
			virtual void set(relative) { }; // relative alternative
			virtual void set(param, param) { };
			virtual void set(param, relative) { }; // relative alternative
			virtual void set(param, param, param) { };
			virtual void set(param, param, relative) { }; // relative alternative
			virtual void set(param, param, param, param) { };
			virtual void set(param, param, param, relative) { }; // relative alternative
			virtual void set(param, param, param, param, param) { };
			virtual void set(param, param, param, param, relative) { }; // relative alternative
			virtual void set(param, param, param, param, param, param) { };
			virtual void set(param, param, param, param, param, relative) { }; // relative alternative
			virtual void set(param, param, param, param, param, param, param) { };
			virtual void set(param, param, param, param, param, param, relative) { }; // relative alternative
			virtual void set(param, param, param, param, param, param, param, param) { };
			virtual void set(param, param, param, param, param, param, param, relative) { }; // relative alternative
		};

		template<typename SIGNAL>
		struct Modifier : public Input<SIGNAL>, public Output<SIGNAL> { 
			using Input<SIGNAL>::in;
			using Output<SIGNAL>::out;

			// signal processing (input-output)
			operator const SIGNAL&() override { process(); return out; } // return processed output
			operator const SIGNAL&() const override { return out; } // return last output

			using Input<SIGNAL>::input;
			virtual void process() override { out = in; } // default to pass-through

			// inline parameter(s) support
			template<typename... params>
			Modifier<SIGNAL>& operator()(params... p) {
				set(p...); return *this;
			}
		protected:
			// overrideable parameter setting (up to 8 parameters)
			virtual void set(param) { };
			virtual void set(param, param) { };
			virtual void set(param, param, param) { };
			virtual void set(param, param, param, param) { };
			virtual void set(param, param, param, param, param) { };
			virtual void set(param, param, param, param, param, param) { };
			virtual void set(param, param, param, param, param, param, param) { };
			virtual void set(param, param, param, param, param, param, param, param) { };
		};

		//// pass signal as input to modifier (allows processing)
		//template<typename SIGNAL>
		//inline Modifier<SIGNAL>& operator>>(SIGNAL& input, Modifier<SIGNAL>& modifier) {
		//	modifier << input;
		//	return modifier;
		//}

		//// pass signal as input to modifier (prevents processing)
		//template<typename SIGNAL>
		//inline Modifier<SIGNAL>& operator>>(const SIGNAL& input, Modifier<SIGNAL>& modifier) {
		//	modifier << input;
		//	return modifier;
		//}

		//// pass literal / scalar as input to modifier (prevents processing)
		//template<typename SIGNAL>
		//inline Modifier<SIGNAL>& operator>>(float input, Modifier<SIGNAL>& modifier) {
		//	modifier << input;
		//	return modifier;
		//}

		// applies a mathematical function to the input signal
		template<typename SIGNAL, typename... Args>
		struct Function : public Generic::Modifier<SIGNAL> {
			using Modifier<SIGNAL>::in;
			using Modifier<SIGNAL>::out;

			std::function<float(Args...)> function;

			template <typename FunctionType>
			Function(FunctionType&& function)
				: function(std::forward<FunctionType>(function)) {}

			// Function call operator to invoke the stored callable
			inline SIGNAL operator()(Args... args) const {
				return function(std::forward<Args>(args)...);
			}

			void process() override {
				out = function(in);
			}
		};

		template<typename SIGNAL>
		struct Oscillator : public Generator<SIGNAL> {
		protected:
			Phase increment;				// phase increment (per sample, in seconds or samples)
			Phase position = 0;				// phase position (in radians or wavetable size)
		public:
			Frequency frequency = 1000.f;	// fundamental frequency of oscillator (in Hz)
			Phase offset = 0;				// phase offset (in radians - e.g. for modulation)

			virtual void reset() { position = 0; }

			using Generator<SIGNAL>::set;
			virtual void set(param frequency) {
				Oscillator::frequency = frequency;
				increment = frequency * 2.f * pi.f / fs;
			}

			virtual void set(param frequency, param phase) {
				position = phase;
				set(frequency);
			}

			virtual void set(param frequency, relative phase) {
				set(frequency);
				set(phase);
			}

			virtual void set(relative phase) {
				offset = phase * (2 * pi);
			}
		};
	}

	struct Input : Generic::Input<signal> { };
	struct Output : Generic::Output<signal> { };
	struct Generator : Generic::Generator<signal> { };
	struct Modifier : public Generic::Modifier<signal> { };
	struct Oscillator : Generic::Oscillator<signal> { };

	template <typename TYPE, typename SIGNAL>
	using GeneratorOrModifier = typename std::conditional<std::is_base_of<Generator, TYPE>::value, Generic::Generator<SIGNAL>,
								typename std::conditional<std::is_base_of<Modifier, TYPE>::value, Generic::Modifier<SIGNAL>, void>::type>::type;

	template<typename TYPE, int COUNT>
	struct Bank : public GeneratorOrModifier<TYPE, signals<COUNT>> {
		using GeneratorOrModifier<TYPE, signals<COUNT>>::in;
		using GeneratorOrModifier<TYPE, signals<COUNT>>::out;

		TYPE items[COUNT];

		TYPE& operator[](int index) { return items[index]; }
		const TYPE& operator[](int index) const { return items[index]; }

		template<typename... Args>
		void set(Args... args) {
			for (int n = 0; n < COUNT; n++)
				items[n].set(args...);
		}

		void input() override {
			for (int n = 0; n < COUNT; n++)
				in[n] >> items[n];
		}

		void process() override {
			for (int n = 0; n < COUNT; n++)
				items[n] >> out[n];
		}
	};
	
	template<typename... Args>
	struct Function : public Generic::Function<signal, Args...> { 
		Function(std::function<float(Args...)> function) : Generic::Function<signal, Args...>(function) { }
	};

	// syntax equivalence: a = b is the same as b >> a
	inline signal& signal::operator=(Output& b) { 
		b >> *this;
		return *this;
	}

	// supports summing of multiple outputs
	inline signal& signal::operator+=(Output& in) { // e.g. out += osc;
		value += signal(in);
		return *this;
	}

	inline static Function<float> sqrt(::sqrtf);
	inline static Function<float> sqr([](float x) -> float { return x * x; });
	inline static Function<float> cube([](float x) -> float { return x * x * x; });
	inline static Function<float> abs(::fabsf);

	#define sqrt klang::sqrt // avoid conflict with std::sqrt
	#define abs klang::abs   // avoid conflict with std::abs

	struct Console : public Text<16384> {
		static std::mutex _lock;
		thread_local static Text<16384> last;
		int length = 0;

		void clear() {
			length = 0;
			string[0] = 0;
		}

		Console& operator=(const char* in) {
			std::lock_guard<std::mutex> lock(_lock);
			length = std::min(capacity(), (int)strlen(in));
			memcpy(string, in, length);
			string[length] = 0;
			return *this;
		}

		Console& operator=(const Console& in) {
			std::lock_guard<std::mutex> lock(_lock);
			length = std::min(capacity(), in.length);
			memcpy(string, in.string, length);
			string[length] = 0;
			return *this;
		}

		Console& operator+=(const char* in) {
			std::lock_guard<std::mutex> lock(_lock);
			const int len = std::max(0, std::min(capacity() - length, (int)strlen(in)));
			memcpy(&string[length], in, len);
			length += len;
			string[length] = 0;
			memcpy(&last, in, len);
			last.string[len] = 0;
			return *this;
		}

		bool hasText() const {
			return length != 0;
		}

		int getText(char* buffer) {
			//if (!length) return 0;
			std::lock_guard<std::mutex> lock(_lock);
			const int _length = length;
			memcpy(buffer, string, length);
			buffer[length] = 0; // null terminator
			clear();
			return _length;
		}
	};

	inline thread_local Text<16384> Console::last;

#define PROFILE(func, ...) debug.print("%-16s = %fns\n", #func "(" #__VA_ARGS__ ")", debug.profile(1000, func, __VA_ARGS__));

	struct Debug : Input {

		struct Buffer : private buffer {
			using buffer::clear;
			using buffer::rewind;
			using buffer::operator++;
			using buffer::operator signal&;

			Buffer() : buffer(16384) { }

			enum Content {
				Empty = 0,
				Notes = 1,
				Effect = 2,
				Synth = 3,
			} content = Empty;

			/*		void attach(float* buffer, int size) {
						Debug::buffer = new klang::buffer(buffer, size);
					}

					void detach() {
						klang::buffer* tmp = buffer;
						buffer = nullptr;
						delete tmp;
					}*/

			bool active = false;
			int used = 0;

			const float* get() {
				if (active) {
					active = false;
					return data();
				}
				else {
					return nullptr;
				}
			}

			Buffer& operator+=(const signal in) {
				active = true;
				buffer::operator+=(in);
				return *this;
			}

			template<typename TYPE>
			Buffer& operator+=(TYPE& in) {
				active = true;
				buffer::operator+=(in);
				return *this;
			}

			signal& operator>>(signal& destination) const {
				destination << *ptr;
				return destination;
			}

			operator const signal& () const {
				return *ptr;
			}
		};

		thread_local static Buffer buffer; // support multiple threads/instances

		Console console;

		template <typename Func, typename... Args>
		inline static double profile(Func func, Args... args) {
			using namespace std::chrono;

			auto start = high_resolution_clock::now();
			func(args...);
			auto end = high_resolution_clock::now();

			auto duration = duration_cast<nanoseconds>(end - start).count();
			return (double)duration;
		}

		template <typename Func, typename... Args>
		inline static double profile(int times, Func func, Args... args) {
			double sum = 0;
			while (times--) {
				sum += profile(func, args...);
			}
			return sum / 1000000.0;
		}

		struct Session {
			Session(float*, int size, Buffer::Content content) {
				//if (buffer) {
					//debug.attach(buffer, size);
				if (buffer.content != Buffer::Notes)
					buffer.clear(size);
				buffer.content = content;
				buffer.rewind();
				//}
			}
			~Session() {
				//debug.detach();
			}

			bool hasAudio() const {
				return buffer.active;
			}
			const float* getAudio() const {
				return buffer.get();
			}
		};

		void print(const char* format, ...) {
			if (console.length < 10000) {
				thread_local static char string[1024] = { 0 };
				string[0] = 0;
				va_list args;                     // Initialize the variadic argument list
				va_start(args, format);           // Start variadic argument processing
				vsnprintf(string, 1024, format, args);  // Safely format the string into the buffer
				va_end(args);                     // Clean up the variadic argument list
				console += string;
			}
		}

		void printOnce(const char* format, ...) {
			if (console.length < 1024) {
				thread_local static char string[1024] = { 0 };
				string[0] = 0;
				va_list args;                     // Initialize the variadic argument list
				va_start(args, format);           // Start variadic argument processing
				vsnprintf(string, 1024, format, args);  // Safely format the string into the buffer
				va_end(args);                     // Clean up the variadic argument list

				if (console.last != string)
					console += string;
			}
		}

		bool hasText() const {
			return console.hasText();
		}

		int getText(char* buffer) {
			return console.getText(buffer);
		}

		operator const signal& () const {
			return buffer.operator const klang::signal & ();
		}

		using Input::input;
		void input() override {
			buffer += in;
		}
	};

	//inline static Debug::Buffer& operator>>(const signal source, Debug& debug) {
	//	return debug.buffer += source;
	//}

	//template<typename TYPE>
	//inline static Debug::Buffer& operator>>(TYPE& source, Debug& debug) {
	//	return debug.buffer += source;
	//}

	static Debug debug;
	inline thread_local Debug::Buffer Debug::buffer; //
	inline std::mutex Console::_lock;

#ifndef GRAPH_SIZE
#define GRAPH_SIZE 44100
#endif

#define GRAPH_SERIES 4

#define FUNCTION(type) (void(*)(type, Result<type>&))[](type x, Result<type>& y)

	template <typename TYPE>
	struct Result {
		TYPE* y = nullptr;
		int i = 0;
		TYPE sum = 0;

		Result(TYPE* array, int index) : y(&array[index]), i(index) { }

		TYPE& operator[](int index) {
			return *(y + index);
		}

		operator TYPE const () { return *y; }

		Result& operator=(const TYPE& in) {
			*y = in;
			return *this;
		}

		TYPE& operator++(int) { i++; return *++y; }
	};

	template<typename TYPE, int SIZE>
	struct Table : public Array<TYPE, SIZE> {
		typedef Array<TYPE, SIZE> Array;
		typedef Result<TYPE> Result;

		using Array::add;

		// single argument
		Table(TYPE(*function)(TYPE)) {
			for (int x = 0; x < SIZE; x++)
				add(function(x));
		}

		// double argument
		Table(void(*function)(TYPE, TYPE), TYPE arg) {
			Array::count = SIZE;
			for (int x = 0; x < SIZE; x++)
				Array::items[x] = function(x, arg);
		}

		// single argument (enhanced)
		Table(void(*function)(TYPE x, Result& y)) {
			Array::count = SIZE;
			Result y(Array::items, 0);
			for (int x = 0; x < SIZE; x++) {
				function((TYPE)x, y);
				y.sum += Array::items[x];
				y++;
			}
		}

		Table(std::initializer_list<TYPE> values) {
			for (TYPE value : values)
				add(value);
		}

		using Array::operator[];
		TYPE operator[](float index) {
			if (index < 0) return Array::items[0];
			else if (index >= (SIZE - 1)) return Array::items[SIZE - 1];
			else {
				const float x = std::floor(index);
				const int i = int(x);
				const float dx = index - x;
				const float dy = Array::items[i + 1] - Array::items[i];
				return Array::items[i] + dx * dy;
			}
		}
	};

	struct Graph : Input {
		struct Point {
			double x, y;
			bool valid() const { return !isnan(x) && !isinf(x) && !isnan(y); } // NB: y can be +/- inf
		};

		struct Axis;

		struct Series : public Array<Point, GRAPH_SIZE + 1>, Input {
			void* function = nullptr;
			using Array::add;
			void add(double y) {
				add({ (double)Array::size(), y });
			}

			template<class TYPE>
			void plot(TYPE f, const Axis& x_axis) {
				if (function != (void*)f) {
					clear();
					function = (void*)f;
					double x = 0;
					const double dx = x_axis.range() / GRAPH_SIZE;
					for (int i = 0; i <= GRAPH_SIZE; i++) {
						x = x_axis.min + i * dx;
						add({ x, (double)f(x) });
					}
				}
			}

			bool operator==(const Series& in) const {
				if (count != in.count) return false;
				for (unsigned int i = 0; i < count; i++)
					if (items[i].x != in.items[i].x || items[i].y != in.items[i].y)
						return false;
				return true;
			}

			bool operator!=(const Series& in) const {
				return !operator==(in);
			}

			using Input::input;
			void input() override {
				add(in);
			}
		};

		struct Axis {
			double min = 0, max = 0;
			bool valid() const { return max != min; }
			double range() const { return max - min; }
			bool contains(double value) const { return value >= min && value <= max; }
			void clear() { min = max = 0; }

			void from(const Series& series, double Point::* axis) {
				if (!series.count) return;
				int points = 0;
				for (unsigned int p = 0; p < series.count; p++) {
					const Point& pt = series[p];
					if (pt.valid() && !isinf(pt.*axis)) {
						if (!points || pt.*axis < min) min = pt.*axis;
						if (!points || pt.*axis > max) max = pt.*axis;
						points++;
					}
				}
				if (std::abs(max) < 0.0000000001) max = 0;
				if (std::abs(min) < 0.0000000001) min = 0;
				if (std::abs(max) > 1000000000.0) max = 0;
				if (std::abs(min) > 1000000000.0) min = 0;
				if (min > max) max = min = 0;
			}
		};

		struct Axes {
			Axis x, y;
			bool valid() const { return x.valid() && y.valid(); }
			void clear() { x = { 0,0 }; y = { 0,0 }; }
			bool contains(const Point& pt) const { return x.contains(pt.x) && y.contains(pt.y); }
		};

		void clear() {
			dirty = true;
			axes.clear();
			for (int s = 0; s < GRAPH_SERIES; s++)
				data[s].clear();
			data.clear();
		}

		bool isActive() const {
			if (data.count)
				return true;
			for (int s = 0; s < GRAPH_SERIES; s++)
				if (data[s].count)
					return true;
			return false;
		}

		struct Data : public Array<Series, GRAPH_SERIES> {
			Series* find(void* function) {
				for (int s = 0; s < GRAPH_SERIES; s++)
					if (operator[](s).function == function)
						return &items[s];
				return nullptr;
			}
		};

		Graph& operator()(double min, double max) {
			dirty = true;
			axes.x.min = min; axes.x.max = max;
			return *this;
		}

		Graph::Series& operator[](int index) {
			dirty = true;
			return data[index];
		}

		const Graph::Series& operator[](int index) const {
			return data[index];
		}

		Graph& operator()(double x_min, double x_max, double y_min, double y_max) {
			dirty = true;
			axes.x.min = x_min; axes.x.max = x_max;
			axes.y.min = y_min; axes.y.max = y_max;
			return *this;
		}

		operator Series& () {
			dirty = true;
			return data[0];
		}

		template<class TYPE>
		void plot(TYPE function) {
			Graph::Series* series = Graph::data.find((void*)function);
			if (!series)
				series = Graph::data.add();
			if (series) {
				dirty = true;
				if (!axes.x.valid())
					axes.x = { -1, 1 };
				series->plot(function, axes.x);
			}
		}

		template<typename TYPE>
		void add(TYPE y) { data[0].add(y); }
		void add(const Point pt) { data[0].add(pt); }

		template<typename TYPE>
		Graph& operator+=(TYPE y) { data[0].add(y); return *this; }
		Graph& operator+=(const Point pt) { data[0].add(pt); return *this; }

		template<typename TYPE>
		Graph& operator=(TYPE(*function)(TYPE)) {
			plot(function);
			return *this;
		}

		Graph& operator=(std::initializer_list<Point> values) {
			clear(); return operator+=(values);
		}

		Graph& operator+=(std::initializer_list<Point> values) {
			for (const auto& value : values)
				add(value);
			return *this;
		}

		using Input::input;
		void input() override {
			add(in);
		}

		// returns the user-defined axes
		const Axes& getAxes() const { return axes; }

		// calculates axes based on data
		void getAxes(Axes& axes) const {
			if (!axes.x.valid()) {
				axes.x.clear();
				for (int s = 0; s < GRAPH_SERIES; s++)
					axes.x.from(data[s], &Point::x);
				if (!axes.y.valid() && axes.y.max != 0)
					axes.y.min = 0;
			}
			if (!axes.y.valid()) {
				axes.y.clear();
				for (int s = 0; s < GRAPH_SERIES; s++)
					axes.y.from(data[s], &Point::y);
				if (!axes.y.valid() && axes.y.max != 0)
					axes.y.min = 0;
			}
		}

		const Data& getData() const { return data; }
		bool isDirty() const { return dirty; }
		void setDirty(bool dirty) { Graph::dirty = dirty; }

		void truncate(unsigned int count) {
			if (count < data.count)
				data.count = count;

			for (int s = 0; s < GRAPH_SERIES; s++)
				if (count < data[s].count)
					data[s].count = count;
		}

	protected:
		Axes axes;
		Data data;
		bool dirty = false;
	};

	template<typename TYPE>
	static Graph& operator>>(TYPE(*function)(TYPE), Graph& graph) {
		graph.plot(function);
		return graph;
	}

	//template<typename TYPE>
	//inline static Graph::Series& operator>>(TYPE y, Graph::Series& series) {
	//	series.add(y);
	//	return series;
	//}

	inline static Graph::Series& operator>>(Graph::Point pt, Graph::Series& series) {
		series.add(pt);
		return series;
	}

	static Graph graph;

	template<int SIZE>
	struct Delay : public Modifier {
		using Modifier::in;
		using Modifier::out;

		buffer buffer;
		float time = 1;
		int position = 0;

		Delay() : buffer(SIZE + 1, 0) { clear(); }

		void clear() {
			buffer.clear();
		}

		//void operator<<(const signal& input) override {
		void input() override { 
			buffer++ = in;
			position++;
			if (buffer.finished()) {
				buffer.rewind();
				position = 0;
			}
		}

		signal tap(int delay) const {
			int read = (position - 1) - delay;
			if (read < 0)
				read += SIZE;
			return buffer[read];
		}

		signal tap(float delay) const {
			float read = (float)(position - 1) - delay;
			if (read < 0.f)
				read += SIZE;

			const float f = floor(read);
			delay = read - f;

			const int i = (int)read;
			const int j = (i == (SIZE - 1)) ? 0 : (i + 1);
			
			return buffer[i] * (1.f - delay) + buffer[j] * delay;
		}

		virtual void process() override {
			out = tap(time);
		}

		virtual void set(param delay) override {
			Delay::time = delay <= SIZE ? (float)delay : SIZE;
		}

		template<typename TIME>
		signal operator()(TIME& delay) {
			if constexpr (std::is_integral_v<TIME>)
				return tap((int)delay);
			else if constexpr (std::is_floating_point_v<TIME>)
				return tap((float)delay);
			else
				return tap((signal)delay);
		}

		template<typename TIME>
		signal operator()(const TIME& delay) {
			if constexpr (std::is_integral_v<TIME>) 
				return tap((int)delay);
			else if constexpr (std::is_floating_point_v<TIME>)
				return tap((float)delay);
			else
				return tap((signal)delay);
		}

		unsigned int max() const { return SIZE; }
	};

	class Wavetable : public Oscillator {
		using Oscillator::set;
	protected:
		buffer buffer;
		const int size;
	public:
		Wavetable(int size = 2048) : buffer(size), size(size) { }

		template<typename TYPE>
		Wavetable(TYPE oscillator, int size = 2048) : buffer(size), size(size) {
			operator=(oscillator);
		}
		
		signal& operator[](int index) {
			return buffer[index];
		}

		template<typename TYPE>
		Wavetable& operator=(TYPE& oscillator) {
			oscillator.set(fs / size);
			for (int s = 0; s < size; s++)
				buffer[s] = oscillator;
			return *this;
		}

		virtual void set(param frequency) override {
			Oscillator::frequency = frequency;
			increment = frequency * (size / fs);
		}

		virtual void set(param frequency, param phase) override {
			position = phase * float(size);
			set(frequency);
		}

		virtual void set(relative phase) override {
			
			offset = phase * float(size);
		}

		virtual void set(param frequency, relative phase) override {
			set(frequency);
			set(phase);
		}

		void process() override {
			position += { increment, size };
			out = buffer[position + offset /*klang::increment(offset, size)*/];
		}
	};

	// Models a changing value (e.g. amplitude) over time (in seconds) using breakpoints (time, value)
	class Envelope : public Generator {
		using Generator::set;

	public:

		struct Follower;

		// Abstract envelope ramp type (override to support different point-to-point trajectories)
		struct Ramp : public Generator {
			float target;
			float rate;
			bool active = false;
		public:

			// Create a null ramp (full signal)
			Ramp(float value = 1.f) {
				setValue(value);
			}

			// Create a ramp from start to target over time (in seconds)
			Ramp(float start, float target, float time) {
				setValue(start);
				setTarget(target);
				setTime(time);
			}

			virtual ~Ramp() { }

			// Is ramp currently processing (ramping)?
			bool isActive() const {
				return active;
			}

			// Set a new target (retains rate)
			virtual void setTarget(float target) {
				Ramp::target = target;
				active = (out != target);
			}

			// Immediately jump to value (disables ramp)
			virtual void setValue(float value) {
				out = value;
				Ramp::target = value;
				active = false;
			}

			// Set rate of change (per sample)
			virtual void setRate(float rate) { Ramp::rate = rate; }

			// Set rate of change (by duration)
			virtual void setTime(float time) { Ramp::rate = time ? 1.f / (time * fs) : 0; }

			// Return the current output and advanced the ramp
			virtual signal operator++(int) = 0;

			void process() override { /* do nothing -> only process on ++ */ }
		};

		// Default (linear) ramp implementation
		struct Linear : public Ramp {

			// Return the current output and process the next
			signal operator++(int) override {
				const signal output = out;

				if (active) {
					if (target > out) {
						out += rate;
						if (out >= target) {
							out = target;
							active = false;
						}
					}
					else {
						out -= rate;
						if (out <= target) {
							out = target;
							active = false;
						}
					}
				}

				return output;
			}
		};

		enum Stage { Sustain, Release, Off };
		enum Mode { Time, Rate };

		// Envelope point type
		struct Point {
			float x, y;

			Point() : x(0), y(0) { }
			
			template<typename T1, typename T2>
			Point(T1 x, T2 y) : x(float(x)), y(float(y)) { }
		};

		// Linked-list of points (for inline initialisation)
		struct Points : public Point {
			Points(float x, float y) {
				Point::x = x;
				Point::y = y;
				next = NULL;
			}
			~Points() {
				delete next;
			}

			Points& operator()(float x, float y) {
				last().next = new Points(x, y);
				return *this;
			}

			Points& last() {
				return next ? next->last() : *this;
			}

			int count() const {
				return next ? 1 + next->count() : 1;
			}

			Points* next;
		};

		// Envelope loop (between two points)
		struct Loop {
			Loop(int from = -1, int to = -1) : start(from), end(to) {}

			void set(int from, int to) { start = from; end = to; }
			void reset() { start = end = -1; }

			bool isActive() const { return start != -1 && end != -1; }

			int start;
			int end;
		};

		// Default Envelope (full signal)
		Envelope() : ramp(new Linear()) {										set(Points(0.f, 1.f));	}

		// Creates a new envelope from a list of points, e.g. Envelope env = Envelope::Points(0,1)(1,0);
		Envelope(const Points& points) : ramp(new Linear()) {					set(points);			}

		// Creates a new envelope from a list of points, e.g. Envelope env = { { 0,1 }, { 1,0 } };
		Envelope(std::initializer_list<Point> points) : ramp(new Linear()) {	set(points);			}

		// Creates a copy of an envelope from another envelope
		Envelope(const Envelope& in) : ramp(new Linear()) {						set(in.points);			}

		virtual ~Envelope() { }

		// Checks if the envelope is at a specified stage (Sustain, Release, Off)
		bool operator==(Stage stage) const { return Envelope::stage == stage; }
		bool operator!=(Stage stage) const { return Envelope::stage != stage; }

		operator float() const { return out; }

		// assign points (without recreating envelope)
		Envelope& operator=(std::initializer_list<Point> points) {
			set(points);
			return *this;
		}

		// Sets the envelope based on an array of points
		void set(const std::vector<Point>& points) {
			Envelope::points = points;
			initialise();
		}

		// Sets the envelope from a list of points, e.g. env.set( Envelope::Points(0,1)(1,0) );
		void set(const Points& point){
			points.clear();
            
			const Points* pPoint = &point;
			while(pPoint){
				points.push_back(*pPoint);
				pPoint = pPoint->next;
			}
            
			initialise();
		}

		// Converts envelope points based on relative time to absolute time
		void sequence() {
			float time = 0.f;
			for(Point& point : points) {
				const float delta = point.x;
				time += delta + 0.00001f;
				point.x = time;
			}
			initialise();
		}

		// Sets an envelope loop between two points
		void setLoop(int startPoint, int endPoint){
			if(startPoint >= 0 && endPoint < points.size())
				loop.set(startPoint, endPoint);
		}

		// Retrieve value at time (in seconds)
		signal at(param time) const {
			if (points.empty()) return 0;
			Point last = { 0, points[0].y };
			for (const Point& point : points) {
				if (point.x >= time) {
					const float dx = point.x - last.x;
					const float dy = point.y - last.y;
					const float x = time - last.x;
					return dx == 0 ? last.y : (last.y + x * dy / dx);
				}
				last = point;
			}
			return points.back().y;
		}
        
		// Resets the envelope loop
		void resetLoop(){
			loop.reset();
			if(stage == Sustain && (point+1) < points.size())
				setTarget(points[point+1], points[point].x);
		}
        
		// Sets the current stage of the envelope
		void setStage(Stage stage){ this->stage = stage; }

		// Returns the current stage of the envelope
		const Stage getStage() const { return stage; }
        
		// Returns the total length of the envelope (ignoring loops)
		float getLength() const { return points.size() ? points[points.size() - 1].x : 0.f; }
        
		// Trigger the release of the envelope
		virtual void release(float time, float level = 0.f){
			stage = Release;
			setTarget({ time, level }, 0);
			//ramp->setTime(time);
			//ramp->setTarget(0.f);
		}

		// Returns true if the envelope has finished (is off)
		bool finished() const {
			return getStage() == Stage::Off;
		}
        
		// Prepare envelope to (re)start
		void initialise(){
			point = 0;
			timeInc = 1.0f / fs;
			loop.reset();
			stage = Sustain;
			if(points.size()){
				out = points[0].y;
				ramp->setValue(points[0].y);
				if(points.size() > 1)
					setTarget(points[1], points[0].x);
			}else{
				out = 1.0f;
				ramp->setValue(1.0f);
			}
		}
        
		// Scales the envelope duration to specified length 
		void resize(float length){
			const float old_length = getLength();
			if(old_length == 0.0)
				return;
            
			const float multiplier = length / (fs * old_length);
			std::vector<Point>::iterator point = points.begin();
			while(point != points.end()){
				point->x *= multiplier;
				point++;
			}
            
			initialise();
		}
        
		// Set the current envelope target
		void setTarget(const Point& point, float time = 0.0){
			(this->*setTargetFunction)(point, time);
		}
        
		// Returns the output of the envelope and advances the envelope.
		signal& operator++(int){ 
			out = (*ramp)++;
            
			switch(stage){
			case Sustain:
				time += timeInc;
				if (!ramp->isActive()) { // envelop segment end reached
					if (loop.isActive() && (point + 1) >= loop.end) {
						point = loop.start;
						ramp->setValue(points[point].y);
						if (loop.start != loop.end)
							setTarget(points[point + 1], points[point].x);
					} else if ((point + 1) < points.size()) {
						if (mode() == Rate || time >= points[point + 1].x) { // reached target point
							point++;
							ramp->setValue(points[point].y); // make sure exact value is set

							if ((point + 1) < points.size()) // new target point?
								setTarget(points[point + 1], points[point].x);
						}
					} else {
						stage = Off;
					}
				} break;
			case Release:
				if (!ramp->isActive()) //if(out == 0.0)
					stage = Off;
				break;
			case Off:
				break;
			}
            
			return out;
		}

		void process() override { /* do nothing -> only process on ++ */
			out = *ramp;
		}
        
		// Retrieve a specified envelope point (read-only)
		const Point& operator[](int point) const {
			return points[point];
		}

		// Set the Ramp class (default: Envelope::Linear)
		void set(Ramp* ramp) {
			Envelope::ramp = std::shared_ptr<Ramp>(ramp);
			initialise();
		}

		void setMode(Mode mode) {
			if (mode == Time)
				setTargetFunction = &Envelope::setTargetTime;
			else
				setTargetFunction = &Envelope::setTargetRate;
		}

		Mode mode() const { return setTargetFunction == &Envelope::setTargetTime ? Time : Rate; }
        
	protected:

		void (Envelope::* setTargetFunction)(const Point& point, float time) = &Envelope::setTargetTime;

		void setTargetTime(const Point& point, float time = 0.0) {
			this->time = time;
			ramp->setTarget(point.y);
			ramp->setRate(fabsf(point.y - ramp->out) / ((point.x - time) * fs));
		}

		void setTargetRate(const Point& point, float rate = 0.0) {
			this->time = 0;
			if (point.x == 0) {
				ramp->setValue(point.y);
			} else {
				ramp->setTarget(point.y);
				ramp->setRate(point.x);
			}
		}

		std::vector<Point> points;
		Loop loop;
        
		int point;
		float time, timeInc;
		Stage stage;

		std::shared_ptr<Ramp> ramp;
	};

	struct ADSR : public Envelope {
	private:
		using Envelope::Follower;
	public:
		param A, D, S, R;

		enum Mode { Time, Rate } mode = Time;

		ADSR() { set(0.5, 0.5, 1, 0.5); }

		void set(param attack, param decay, param sustain, param release) override {
			A = attack + 0.005f;
			D = decay + 0.005f;
			S = sustain;
			R = release + 0.005f;

			points.resize(3);
			points[0] = { 0, 0 };
			points[1] = { A, 1 };
			points[2] = { A + D, S };
			
			initialise();
			setLoop(2, 2);
		}

		void release(float time = 0.f, float level = 0.f) override {
			Envelope::release(time ? time : float(R), level);
		}

		bool operator==(Envelope::Stage stage) const {
			return getStage() == stage;
		}
	};

	template<class OSCILLATOR>
	struct Operator : public OSCILLATOR, public Input {
		Envelope env;
		Amplitude amp = 1.f;

		Operator& operator()(param f) { OSCILLATOR::set(f); return *this; }
		Operator& operator()(param f, relative phase) { OSCILLATOR::set(f, phase); return *this; }
		Operator& operator()(relative phase) { OSCILLATOR::set(phase); return *this; }

		virtual Operator& operator=(const Envelope::Points& points) {
			env = points;
			return *this;
		}

		virtual Operator& operator=(const Envelope& points) {
			env = points;
			return *this;
		}

		Operator& operator*(float amp) {
			Operator::amp = amp;
			return *this;
		}

		virtual void process() override {
			OSCILLATOR::set(+in);
			OSCILLATOR::process();
			OSCILLATOR::out *= env++ * amp;
		}

		inline Operator& operator>>(Operator& carrier) {
			carrier << *this;
			return carrier;
		}
	};

	template<class OSCILLATOR>
	inline const signal& operator>>(klang::signal modulator, Operator<OSCILLATOR>& carrier) {
		carrier << modulator;
		return carrier;
	}

	struct Plugin {
		virtual ~Plugin() { }

		virtual void onParameter(int index, float value) { };
		virtual void onPreset(int index) { };

		Controls controls;
		Presets presets;
	};

	struct Effect : public Plugin, public Modifier {
		virtual ~Effect() { }

		virtual void prepare() { };
		virtual void process() { out = in; }
		virtual void process(buffer buffer) {
			prepare();
			while (!buffer.finished()) {
				input(buffer);
				process();
				buffer++ = out;
				debug.buffer++;
			}
		}
	};

	template<class SYNTH>
	class NoteBase {
		SYNTH* synth;

		class Controls {
			klang::Controls* controls = nullptr;
		public:
			Controls& operator=(klang::Controls& controls) { Controls::controls = &controls; return *this; }
			signal& operator[](int index) { return controls->operator[](index).operator signal&(); }
			const signal& operator[](int index) const { return controls->operator[](index).operator const signal&(); }
			unsigned int size() { return controls ? controls->size() : 0; }
		};

	protected:
		virtual event on(Pitch p, Velocity v) { }
		virtual event off(Velocity v = 0) { stage = Off; }
		virtual event control(int controller, int value) { };

		SYNTH* getSynth() { return synth; }
	public:
		Pitch pitch;
		Velocity velocity;
		Controls controls;

		NoteBase() : synth(nullptr) { }
		virtual ~NoteBase() { }

		void attach(SYNTH* synth) {
			NoteBase::synth = synth;
			controls = synth->controls;
			init();
		}

		virtual void init() { }

		virtual void start(Pitch p, Velocity v) {
			stage = Onset;
			pitch = p;
			velocity = v;
			on(pitch, velocity);
			stage = Sustain;
		}

		virtual bool release(Velocity v = 0) {
			if (stage == Off)
				return true;

			if (stage != Release) {
				stage = Release;
				off(v);
			}
			
			return stage == Off;
		}

		virtual bool stop(Velocity v = 0) {
			stage = Off;
			return true;
		}

		bool finished() const { return stage == Off; }

		enum Stage { Onset, Sustain, Release, Off } stage = Off;

		virtual void controlChange(int controller, int value) { control(controller, value); };
	};

	struct Synth;

	// base class for individual synthesiser notes/voices
	struct Note : public NoteBase<Synth>, public Generator {
		virtual void prepare() { }
		virtual void process() override = 0;
		virtual bool process(buffer buffer) {
			prepare();
			while (!buffer.finished()) {
				process();
				buffer++ = out;
				debug.buffer++;
			}
			return !finished();
		}
		virtual bool process(buffer* buffer) {
			return process(buffer[0]);
		}
	};

	template<class SYNTH, class NOTE = Note>
	struct Notes : Array<NOTE*, 128> {
		SYNTH* synth;
		typedef Array<NOTE*, 128> Array;
		using Array::items;
		using Array::count;

		Notes(SYNTH* synth) : synth(synth) {
			for (int n = 0; n < 128; n++)
				items[n] = nullptr;
		}
		virtual ~Notes();

		template<class TYPE>
		void add(int count) {
			for (int n = 0; n < count; n++) {
				TYPE* note = new TYPE();
				note->attach(synth);
				Array::add(note);
			}
		}

		// returns index of a 'free' note (stealing oldest, if required)
		unsigned int noteOns = 0;				// number of NoteOn events processed
		unsigned int noteStart[128] = { 0 };	// track age of notes (for note stealing)

		int assign(){
			// favour unused voices
			for (int i = 0; i < count; i++) {
				if (items[i]->stage == NOTE::Off){
					noteStart[i] = noteOns++;
					return i;
				}
			}

			// no free notes => steal oldest released note?
			int oldest = -1;
			unsigned int oldest_start = 0;
			for (int i = 0; i < count; i++) {
				if (items[i]->stage == NOTE::Release) {
					if(oldest == -1 || noteStart[i] < oldest_start){
						oldest = i;
						oldest_start = noteStart[i];
					}
				}
			}
			if(oldest != -1){
				noteStart[oldest] = noteOns++;
				return oldest;
			}

			// no available released notes => steal oldest playing note
			oldest = -1;
			oldest_start = 0;
			for (int i = 0; i < count; i++) {
				if(oldest == -1 || noteStart[i] < oldest_start){
					oldest = i;
					oldest_start = noteStart[i];
				}
			}
			noteStart[oldest] = noteOns++;
			return oldest;
		}
	};

	// base class for synthesiser mini-plugins
	struct Synth : public Effect {
		typedef Note Note;

		Notes<Synth, Note> notes;

		Synth() : notes(this) { }
		virtual ~Synth() { }

		virtual void presetLoaded(int preset) { }
		virtual void optionChanged(int param, int item) { }
		virtual void buttonPressed(int param) { };

		int indexOf(Note* note) const {
			int index = 0;
			for (const auto* n : notes.items) {
				if (note == n) return 
					index;
				index++;
			}
			return -1; // not found
		}
	};

	template<class SYNTH, class NOTE>
	inline Notes<SYNTH, NOTE>::~Notes() {
		for (unsigned int n = 0; n < count; n++) {
			NOTE* tmp = items[n];
			items[n] = nullptr;
			delete tmp;
		}
	}

	namespace Mono { using namespace klang; }
	namespace mono { using namespace klang; }

	namespace Stereo {
		typedef signals<2> signal;

		struct frame {
			mono::signal& l;
			mono::signal& r;

			frame(mono::signal& left, mono::signal& right) : l(left), r(right) { }
			frame(signal& signal) : l(signal.l), r(signal.r) { }

			frame& operator+=(const frame& x) { l += x.l; r += x.r; return *this; }
			frame& operator-=(const frame& x) { l -= x.l; r -= x.r; return *this; }
			frame& operator*=(const frame& x) { l *= x.l; r *= x.r; return *this; }
			frame& operator/=(const frame& x) { l /= x.l; r /= x.r; return *this; }

			frame& operator+=(const signal x) { l += x.l; r += x.r; return *this; }
			frame& operator-=(const signal x) { l -= x.l; r -= x.r; return *this; }
			frame& operator*=(const signal x) { l *= x.l; r *= x.r; return *this; }
			frame& operator/=(const signal x) { l /= x.l; r /= x.r; return *this; }

			frame& operator+=(const mono::signal x) { l += x; r += x; return *this; }
			frame& operator-=(const mono::signal x) { l -= x; r -= x; return *this; }
			frame& operator*=(const mono::signal x) { l *= x; r *= x; return *this; }
			frame& operator/=(const mono::signal x) { l /= x; r /= x; return *this; }

			frame& operator+=(float x) { l += x; r += x; return *this; }
			frame& operator-=(float x) { l -= x; r -= x; return *this; }
			frame& operator*=(float x) { l *= x; r *= x; return *this; }
			frame& operator/=(float x) { l /= x; r /= x; return *this; }

			frame& operator+=(double x) { l += (float)x; r += (float)x; return *this; }
			frame& operator-=(double x) { l -= (float)x; r -= (float)x; return *this; }
			frame& operator*=(double x) { l *= (float)x; r *= (float)x; return *this; }
			frame& operator/=(double x) { l /= (float)x; r /= (float)x; return *this; }

			frame& operator+=(int x) { l += (float)x; r += (float)x; return *this; }
			frame& operator-=(int x) { l -= (float)x; r -= (float)x; return *this; }
			frame& operator*=(int x) { l *= (float)x; r *= (float)x; return *this; }
			frame& operator/=(int x) { l /= (float)x; r /= (float)x; return *this; }

			signal operator+(const signal x) const { return { l + x.l, r + x.r }; }
			signal operator-(const signal x) const { return { l - x.l, r - x.r }; }
			signal operator*(const signal x) const { return { l * x.l, r * x.r }; }
			signal operator/(const signal x) const { return { l / x.l, r / x.r }; }

			signal operator+(const mono::signal x) const { return { l + x, r + x }; }
			signal operator-(const mono::signal x) const { return { l - x, r - x }; }
			signal operator*(const mono::signal x) const { return { l * x, r * x }; }
			signal operator/(const mono::signal x) const { return { l / x, r / x }; }

			signal operator+(float x) const { return { l + x, r + x }; }
			signal operator-(float x) const { return { l - x, r - x }; }
			signal operator*(float x) const { return { l * x, r * x }; }
			signal operator/(float x) const { return { l / x, r / x }; }

			signal operator+(double x) const { return { l + (float)x, r + (float)x }; }
			signal operator-(double x) const { return { l - (float)x, r - (float)x }; }
			signal operator*(double x) const { return { l * (float)x, r * (float)x }; }
			signal operator/(double x) const { return { l / (float)x, r / (float)x }; }

			signal operator+(int x) const { return { l + (float)x, r + (float)x }; }
			signal operator-(int x) const { return { l - (float)x, r - (float)x }; }
			signal operator*(int x) const { return { l * (float)x, r * (float)x }; }
			signal operator/(int x) const { return { l / (float)x, r / (float)x }; }

			frame& operator=(const signal& signal) {
				l = signal.l;
				r = signal.r;
				return *this;
			}

			mono::signal mono() const {
				return (l + r) * 0.5f;
			}
		};

		struct Input : Generic::Input<signal> { };
		struct Output : Generic::Output<signal> { };
		struct Generator : Generic::Generator<signal> { };
		struct Modifier : Generic::Modifier<signal> { };
		struct Oscillator : Generic::Oscillator<signal> { };

		//inline Modifier& operator>>(signal input, Modifier& modifier) {
		//	modifier << input;
		//	return modifier;
		//}

		// interleaved access to non-interleaved stereo buffers
		struct buffer {
			typedef Stereo::signal signal;
			mono::buffer& left, right;

			buffer(const buffer& buffer) : left(buffer.left), right(buffer.right) { rewind(); }
			buffer(mono::buffer& left, mono::buffer& right) : left(left), right(right) { rewind(); }

			operator const signal() const { return { left, right }; }
			operator frame () {				return { left, right }; }
			bool finished() const {			return left.finished() && right.finished(); }
			frame operator++(int) {			return { left++, right++ };	}

			frame operator=(const signal& in) {
				return { left = in.l, right = in.r };
			}

			buffer& operator=(const buffer& in) {
				left = in.left;
				right = in.right;
				return *this;
			}

			buffer& operator+=(const signal& in) {
				left += in.l;
				right += in.r;
				return *this;
			}

			buffer& operator=(const frame& in) { 		left = in.l;  right = in.r;		return *this;	}
			buffer& operator+=(const frame& in) {		left += in.l; right += in.r;	return *this;	}
			buffer& operator*=(const frame& in) {		left *= in.l; right *= in.r;	return *this;	}

			buffer& operator=(const mono::signal in) {		left = in;  right = in;	return *this;	}
			buffer& operator+=(const mono::signal in) {	left += in; right += in;return *this;	}
			buffer& operator*=(const mono::signal in) {	left *= in; right *= in;return *this;	}

			frame operator[](int index) {
				return { left[index], right[index] };
			}

			signal operator[](int index) const {
				return { left[index], right[index] };
			}

			mono::buffer& channel(int index) {
				return index == 1 ? right : left;
			}

			void clear() {
				left.clear();
				right.clear();
			}

			void clear(int size) {
				left.clear(size);
				right.clear(size);
			}

			void rewind() {
				left.rewind();
				right.rewind();
			}
		};

		template<class TYPE>
		struct Bank : klang::Bank<TYPE, 2> { };

		template<int SIZE>
		struct Delay : Bank<klang::Delay<SIZE>> { 
			using Bank<klang::Delay<SIZE>>::items;
			using Bank<klang::Delay<SIZE>>::in;
			using Bank<klang::Delay<SIZE>>::out;

			void clear() {
				items[0].clear();
				items[1].clear();
			}

			signal tap(int delay) const {
				int read = (items[0].position - 1) - delay;
				if (read < 0)
					read += SIZE;
				return { items[0].buffer[read], items[1].buffer[read] };
			}

			signal tap(float delay) const {
				float read = (float)(items[0].position - 1) - delay;
				if (read < 0.f)
					read += SIZE;

				const float f = floor(read);
				delay = read - f;

				const int i = (int)read;
				const int j = (i == (SIZE - 1)) ? 0 : (i + 1);

				return { items[0].buffer[i] * (1.f - delay) + items[0].buffer[j] * delay,
						 items[1].buffer[i] * (1.f - delay) + items[1].buffer[j] * delay };
			}

			virtual void process() override {
				out = tap(items[0].time);
			}

			template<typename TIME>
			signal operator()(const TIME& delay) {
				if constexpr (std::is_integral_v<TIME>)
					return tap((int)delay);
				else if constexpr (std::is_floating_point_v<TIME>)
					return tap((float)delay);
				else if constexpr (std::is_same_v<TIME, signal>) // stereo signal (use for l/r delay times)
					return { items[0].tap(delay.l), items[1].tap(delay.r) };
				else
					return tap((klang::signal)delay); // else treat as single signal
			}

			unsigned int max() const { return SIZE; }
		};

		struct Effect : public Plugin, public Modifier {
			virtual ~Effect() { }

			virtual void prepare() { };
			virtual void process() { out = in; };
			virtual void process(Stereo::buffer buffer) {
				prepare();
				while (!buffer.finished()) {
					input(buffer);
					process();
					buffer++ = out;
					debug.buffer++;
				}
			}
		};

		struct Synth;

		// stereo note object
		struct Note : public NoteBase<Synth>, public Generator { 
			//virtual signal output() { return 0; };

			virtual void prepare() { }
			virtual void process() override = 0;
			virtual bool process(Stereo::buffer buffer) {
				prepare();
				while (!buffer.finished()) {
					process();
					buffer++ = out;
				}
				return !finished();
			}
			virtual bool process(mono::buffer* buffers) {
				buffer buffer = { buffers[0], buffers[1] };
				return process(buffer);
			}
		};

		// base class for stereo synthesiser mini-plugins
		struct Synth : public Effect {
			typedef Stereo::Note Note;

			struct Notes : klang::Notes<Synth, Note> {
				using klang::Notes<Synth, Note>::Notes;
			} notes;

			Synth() : notes(this) { }
			virtual ~Synth() { }

			virtual void presetLoaded(int preset) { }
			virtual void optionChanged(int param, int item) { }
			virtual void buttonPressed(int param) { };

			int indexOf(Note* note) const {
				int index = 0;
				for (const auto* n : notes.items) {
					if (note == n) return
						index;
					index++;
				}
				return -1; // not found
			}
		};
	}

	namespace stereo {
		using namespace Stereo;
	}

	// pass source to destination (with source processing)
	template<typename SOURCE, typename DESTINATION>
	inline DESTINATION& operator>>(SOURCE& source, DESTINATION& destination) {
		if constexpr (is_derived_from<Input, DESTINATION>())
			destination.input(source); // input to destination (enables overriding of <<)
		else if constexpr (is_derived_from<Stereo::Input, DESTINATION>())
			destination.input(source); // input to destination (enables overriding of <<)
		else
			destination << source; // copy to destination
		return destination;
	}

	// pass source to destination (no source processing)
	template<typename SOURCE, typename DESTINATION>
	inline DESTINATION& operator>>(const SOURCE& source, DESTINATION& destination) {
		if constexpr (is_derived_from<Input, DESTINATION>())
			destination.input(source); // input to destination (enables overriding of <<)
		else if constexpr (is_derived_from<Stereo::Input, DESTINATION>())
			destination.input(source); // input to destination (enables overriding of <<)
		else
			destination << source; // copy to destination
		return destination;
	}

	namespace Generators {
		using namespace klang;

		namespace Basic {
			struct Sine : public Oscillator {
				void process() {
					out = sin(position + offset);
					position += increment;
				}
			};

			struct Saw : public Oscillator {
				void process() {
					out = position * pi.inv - 1.f;
					position += increment;
				}
			};

			struct Triangle : public Oscillator {
				void process() {
					out = abs(2.f * position * pi.inv - 2) - 1.f;
					position += increment;
				}
			};

			struct Square : public Oscillator {
				void process() {
					out = position > pi ? 1.f : -1.f;
					position += increment;
				}
			};

			struct Pulse : public Oscillator {
				param duty = 0.5f;

				using Oscillator::set;
				void set(param frequency, param phase, param duty) {
					set(frequency, phase);
					Pulse::duty = duty;
				}

				void process() {
					out = position > (duty * pi) ? 1.f : -1.f;
					position += increment;
				}
			};

			struct Noise : public Generator {
				void process() {
					out = rand() * 2.f/(const float)RAND_MAX - 1.f;
				}
			};
		};

		namespace Fast {
//			constexpr float pi = 3.1415926535897932384626433832795f;
			constexpr float twoPi = float(2.0 * 3.1415926535897932384626433832795);
			constexpr float twoPiInv = float(1.0 / twoPi);

			struct Increment {
				// represent phase delta using full range of int32
				// (integer math, no conditionals, free oversampling)
				signed int amount = 0;

				void reset() { amount = 0; }

				// set increment given frequency (in Hz)
				void set(Frequency f) {
					constexpr float FC4 = float(261.62556530059862);			// Reference Freq (C4 = 261Hz)
					constexpr float FC4_FINTMAX = float(261.62556530059862 * 2147483648.0);
					const float FBASE = (FC4_FINTMAX) / klang::fs;
					amount = 2u * (signed int)(FBASE / FC4 * f);
				}

				// convert int32 to float [0, 2pi)
				operator float() const {
					const unsigned int i = (amount >> 9) | 0x3f800000;
					return *(const float*)&i - 1.f;
				}

				Increment operator+(const Increment& in) const {
					return { in.amount + amount };
				}
			};

			struct Phase {
				// represent phase using full range of uint32
				// (integer math, no conditionals, free oversampling)
				unsigned int position = 0;

				// convert float [0, 2pi) to uint32
				Phase& operator=(klang::Phase phase) {
					constexpr float FINTMAX = 2147483648.0f; // (float)INT_MAX + 1.f;
					phase = phase * FINTMAX / (2.f * pi);
					position = (unsigned int)phase;
					return *this;

					//position = 2u * (signed int)(phase * (float(INT_MAX) + 1.f) * twoPiInv);
					//return *this;
				}

				// convert uint32 to float [0, 1)
				operator float() const {
					const unsigned int i = (position >> 9) | 0x3f800000;
					return (*(const float*)&i - 1.f);

					//// = quarter-turn (pi/2) phase offset
					//unsigned int phase = position + 0x40000000;

					//// range reduce to [0,pi]
					//if (phase & 0x80000000) // cos(x) = cos(-x)
					//	phase = ~phase; 

					//// convert to float in range [-pi/2,pi/2)
					//phase = (phase >> 8) | 0x3f800000;
					//return *(float*)&phase * pi - 3.f/2.f * pi; // 1.0f + (phase / (2^31))
				}

				// returns offset phase
				float operator+(const Phase& offset) const {
					
					unsigned int phase = (position + offset.position) + 0x40000000; // = quarter-turn (pi/2) phase offset
					// range reduce to [0,pi]
					if (phase & 0x80000000) // cos(x) = cos(-x)
						phase = ~phase;

					// convert to float in range [-pi/2,pi/2)
					phase = (phase >> 8) | 0x3f800000;
					return *(float*)&phase * pi - 3.f / 2.f * pi; // 1.0f + (phase / (2^31))
				}

				// convert uint32 to float [0, 2pi)
				float radians() const {
					return operator float();
				}

				// applies increment and returns float [0, 2pi)
				//float operator+=(Increment i) {
				//	unsigned int phase = position + 0x40000000; // = quarter-turn (pi/2) phase offset
				//	position += i.amount;						// apply increment

				//	// range reduce to [0,pi]
				//	if (phase & 0x80000000) // cos(x) = cos(-x)
				//		phase = ~phase;

				//	// convert to float in range [-pi/2,pi/2)
				//	phase = (phase >> 8) | 0x3f800000;
				//	return *(float*)&phase * pi - 3 / 2.f * pi; // 1.0f + (phase / (2^31))
				//}

				// applies increment and returns float [0, 2pi)
				Phase& operator+=(const Increment i) {
					position += i.amount;						// apply increment
					return *this;
				}

				// applies increment and returns float [0, 2pi)
				//Phase operator+(const Phase& i) const {
				//	return { position + i.position };				// apply increment
				//}

				//// applies increment and returns float [0, 2pi)
				//float operator+=(Increment i) {
				//	unsigned int phase = position + 0x40000000; // = quarter-turn (pi/2) phase offset
				//	position += i.amount;						// apply increment

				//	// range reduce to [0,pi]
				//	if (phase & 0x80000000) // cos(x) = cos(-x)
				//		phase = ~phase; 

				//	// convert to float in range [-pi/2,pi/2)
				//	phase = (phase >> 8) | 0x3f800000;
				//	return *(float*)&phase * pi - 3/2.f * pi; // 1.0f + (phase / (2^31))
				//}

				//Phase& operator+(Phase offset) const {
				//	offset.position += position;
				//	return offset;
				//}

				//static void test() {
				//	Phase p;
				//	assert((float)(p = 0.f).radians() == 0.f);
				//	assert((float)(p = pi/2).radians() == pi/2);
				//	assert((float)(p = pi).radians() == pi);
				//	assert((float)(p = 3*pi/2).radians() == 3*pi/2);
				//	assert((float)(p = 2*pi).radians() == 0.f);
				//}
			};

			// sin approximation [-pi/2, pi/2] using odd minimax polynomial (Robin Green)
			inline static float polysin(float x) {
				const float x2 = x * x;
				return (((-0.00018542f * x2 + 0.0083143f) * x2 - 0.16666f) * x2 + 1.0f) * x;
			}

			// fast sine (based on V2/Farbrausch; using polysin)
			inline static float fastsin(float x)
			{
				// Range reduction to [0, 2pi]
				//x = fmodf(x, twoPi);
				//if (x < 0)			// support -ve phase (e.g. for FM)
				//	x += twoPi;		//
				x = fast_mod2pi(x);

				// Range reduction to [-pi/2, pi/2]
				if (x > 3/2.f * pi)	// 3/2pi ... 2pi
					x -= twoPi;	// (= translated)
				else if (x > pi/2)	// pi/2 ... 3pi/2
					x = pi - x;		// (= mirrored)
  
				return polysin(x);
			}

			// fast sine (using polysin and integer math)
			inline static float fastsinp(unsigned int p)
			{
				// Range reduction to [0, 2pi]
				//x = fmodf(x, twoPi);
				//if (x < 0)			// support -ve phase (e.g. for FM)
				//	x += twoPi;		//
				float x = fast_modp(p);

				// Range reduction to [-pi/2, pi/2]
				if (x > 3.f/2.f * pi)	// 3/2pi ... 2pi
					x -= twoPi;			// (= translated)
				else if (x > pi/2.f)	// pi/2 ... 3pi/2
					x = pi - x;			// (= mirrored)

				return polysin(x);
			}

			struct Sine : public Oscillator {
				void reset() override {
					Sine::position = Oscillator::position = 0;
					Sine::offset = Oscillator::offset = 0;
					set(Oscillator::frequency, 0.f);
				}

				// set the frequency
				void set(param frequency) override {
					if (frequency != Oscillator::frequency) {
						Oscillator::frequency = frequency;
						increment.set(frequency);
					}
				}

				void set(param frequency, param phase) override { // set frequency and phase
					Sine::position = Oscillator::position = phase;
					Sine::offset = Oscillator::offset = 0;
					set(frequency);
				}

				void set(param frequency, relative phase) override { // set frequency and phase
					set(frequency);
					set(phase);
				}

				void set(relative phase) override { // set frequency and phase
					Sine::offset = Oscillator::offset = phase * twoPi;
				}

				void process() override {
					out = fastsinp(position.position + offset.position);
					position += increment;
				}

			protected:
				Fast::Increment increment;
				Fast::Phase position, offset;
			};

			struct OSM {
				using Waveform = float(OSM::*)();
				const Waveform waveform;

				OSM(Waveform waveform) : waveform(waveform) { }

				enum State { // carry:old_up:new_up
					NewUp = 0b001, NewDown = 0b000,
					OldUp = 0b010, OldDown = 0b000,
					Carry = 0b100, NoCarry = 0b000,

					Down		= OldDown|NewDown|NoCarry,  
					UpDown		= OldUp|NewDown|NoCarry,    
					Up			= OldUp|NewUp|NoCarry,		
					DownUpDown	= OldDown|NewDown|Carry,	
					DownUp		= OldDown|NewUp|Carry,		
					UpDownUp	= OldUp|NewUp|Carry			
				};

				Fast::Increment increment;
				Fast::Phase offset;
				Fast::Phase duty;
				State state;
				float delta;

				// coeffeficients
				float f, omf, rcpf, rcpf2, col;
				float c1,c2;

				param frequency = 0; // cached to optimise updates

				void init() {
					state = (offset.position - increment.amount) < duty.position ? Up : Down;
					f = delta;
					omf = 1.f - f;
					rcpf2 = 2.f * (rcpf = 1.f / f);
					col = duty;

					c1 = 1.f / col;
					c2 = -1.f / (1.0f - col);
				}
				
				void set(param frequency) {
					if (OSM::frequency != frequency) {
						OSM::frequency = frequency;
						increment.set(frequency);
						delta = increment;
						init();
					}
				}

				void set(param frequency, param phase) {
					if (OSM::frequency != frequency) {
						OSM::frequency = frequency;
						increment.set(frequency);
						delta = increment;
					}
					offset = phase;
					init();
				}

				void set(param frequency, param phase, param duty) {
					if (OSM::frequency != frequency) {
						OSM::frequency = frequency;
						increment.set(frequency);
						delta = increment;
					}
					offset = phase;
					setDuty(duty);
				}

				void setDuty(param duty) {
					OSM::duty = duty * (2.f * pi);
					init();
				}

				State tick() {
					// old_up = new_up, new_up = (cnt < brpt)
					state = (State)(((state << 1) | (offset.position < duty.position)) & (NewUp|OldUp));

					// we added freq to cnt going from the previous sample to the current one.
					// so if cnt is less than freq, we carried.
					const State transition = (State)(state | (offset.position < (unsigned int)increment.amount ? DownUpDown : Down)); 

					// finally, tick the oscillator
					offset += increment;

					return transition;
				}

				inline static float sqr(float x) { return x * x; }
				inline float output() { return (this->*waveform)(); }

				//inline float saw() {
				//	const float f = delta;
				//	const float omf = 1.f - f;
				//	const float rcpf = 1.f / f;
				//	const float col = duty;
				//	const float c1 = 1.f / col;
				//	const float c2 = -1.f / (1.0f - col);
				//	const float p = offset - col;
				//	float y = 0.0f;
				//	// state machine action
				//	switch (tick()) {
				//	case Up:		 y = c1 * (p + p - f); break; // average of linear function = just sample in the middle
				//	case Down:		 y = c2 * (p + p - f); break; // again, average of a linear function
				//	case UpDown:	 y = rcpf * (c2 * sqr(p) - c1 * sqr(p - f));		  break;
				//	case DownUp:	 y = -rcpf * (1.f + c2 * sqr(p + omf) - c1 * sqr(p)); break;
				//	case UpDownUp:	 y = -rcpf * (1.f + c1 * omf * (p + p + omf));		  break;
				//	case DownUpDown: y = -rcpf * (1.f + c2 * omf * (p + p + omf));		  break;
				//	default:		 y = -1; // should never happen
				//	}
				//	return y + 1.f;
				//}

				float saw() { return saw(offset - col, tick()); }
				inline float saw(const float p, const State state) const {
					// state machine action
					switch (state) {
					case Up:		 return c1 * (p + p - f) + 1.f; break; // average of linear function = just sample in the middle
					case Down:		 return c2 * (p + p - f) + 1.f; break; // again, average of a linear function
					case UpDown:	 return rcpf * (c2 * sqr(p) - c1 * sqr(p - f)) + 1.f;		   break;
					case DownUp:	 return -rcpf * (1.f + c2 * sqr(p + omf) - c1 * sqr(p)) + 1.f; break;
					case UpDownUp:	 return -rcpf * (1.f + c1 * omf * (p + p + omf)) + 1.f;		   break;
					case DownUpDown: return -rcpf * (1.f + c2 * omf * (p + p + omf)) + 1.f;		   break;
					default:		 return 0.f; // should never happen
					}
				}

				float pulse() { return pulse(offset, tick()); }
				inline float pulse(const float p, const State state) const {
					// state machine action
					switch (state) {
					case Up:		 return 1.f;							break;
					case Down:		 return -1.f;							break;
					case UpDown:	 return rcpf2 * (col - p) + 1.f;		break;
					case DownUp:	 return rcpf2 * p - 1.f;				break;
					case UpDownUp:	 return rcpf2 * (col - 1.0f) + 1.f;		break;
					case DownUpDown: return rcpf2 * col - 1.f;				break;
					default:		 return 0; // should never happen
					}
				}
			};

			struct Osm : public Oscillator {
				const float _Duty;

				Osm(OSM::Waveform waveform, float duty = 0.f) : _Duty(duty), osm(waveform) { osm.setDuty(_Duty); }

				void set(param frequency) {
					osm.set(frequency);
				}

				void set(param frequency, param phase) {
					osm.set(frequency, phase);
				}

				void set(param frequency, param phase, param duty) {
					osm.set(frequency, phase, duty);
				}

				void process() {
					out = osm.output();
				}

			protected:
				using Oscillator::set;
				OSM osm;
			};

			struct Saw :	  public Osm {	Saw()	   : Osm(&OSM::saw, 0.f) {}	};
			struct Triangle : public Osm {	Triangle() : Osm(&OSM::saw, 1.f) {}	};
			struct Square :	  public Osm {	Square()   : Osm(&OSM::pulse, 1.0f) {} };
			struct Pulse :	  public Osm {	Pulse()	   : Osm(&OSM::pulse, 0.5f) {} };

			struct Noise : public Generator {
				static constexpr unsigned int bias = 0b1000011100000000000000000000000;
				union { unsigned int i; float f; };
				void process() {
					i = ((rand() & 0b111111111111111UL) << 1) | bias;
					out = f - 257.f;
				}
			};
		};

		namespace Wavetables {
			struct Sine : public Wavetable {
				Sine() : Wavetable(Basic::Sine()) { }
			};

			struct Saw : public Wavetable {
				Saw() : Wavetable(Basic::Saw()) { }
			};
		}
	};

	namespace Filters {
		namespace Basic {
			struct IIR : public Modifier {
				float a, b;

				void set(param coeff) {
					a = coeff;
					b = 1 - a;
				}

				void process() {
					out = (a * in) + (b * out);
				}
			};

			// Single-pole (one-pole, one-zero) First Order Filters
			namespace OnePole 
			{
				struct Filter : Modifier {
					float f = 0; 		// cutoff f
					//float shelf = 1;	// shelving gain

					float /*a0 = 1*/ a1 = 0, b0 = 1, b1 = 0; //coefficients

					float exp0 = 0;
					//float tan0 = 0;
					float z = 0;	// filter state

					void reset() {
						a1 = 0;
						b0 = 1;
						b1 = 0;
						f = 0;
						z = 0;
					}

					void set(param f) {
						if (Filter::f != f) {
							Filter::f = f;						
							//tan0 = tanf(0.5f * f * fs.w);
							init();
						}
					}

					virtual void init() = 0;

					void process() {
						out = b0 * in + b1 * z + a1 * out + DENORMALISE;
					}
				};

				// One-pole, low-pass filter
				struct LPF : Filter {
					void init() {
						const float exp0 = expf(-f * fs.w);
						b0 = 1 - exp0;
						a1 = exp0;
					}
					void process() {
						out = b0 * in + a1 * out + DENORMALISE;
					}
				};

				// One-pole, high-pass filter
				struct HPF : Filter {
					void init() {
						const float exp0 = expf(-f * fs.w);
						b0 = 0.5f * (1.f + exp0);
						b1 = -b0;
						a1 = exp0;
					}
				};
			};

			namespace Butterworth {
				struct LPF : OnePole::Filter {
					void init() {
						const float c = 1.f / tanf(pi * f * fs.inv);
						constant a0 = { 1.f + c };
						b0 = a0.inv;  // b0 == b1
						a1 = (1.f - c) * a0.inv; // = (1-c) / a0
					}
					void process() {
						out = b0 * (in + z) - a1 * out;// +DENORMALISE;
						z = in;
					}
				};
			};

			// Transposed Direct Form II Biquadratic Filter
			namespace Biquad {

				struct Filter : Modifier
				{
					float f = 0; 	// cutoff/centre f
					float Q = 0;	// Q (resonance)

					float /*a0 = 1*/ a1 = 0, a2 = 0, b0 = 1, b1 = 0, b2 = 0; // coefficients

					float a = 0;		// alpha
					float cos0 = 1;		// cos(omega)
					float sin0 = 0;		// sin(omega)
					float z[2] = { 0 };	// filter state

					void reset() {
						f = 0;
						Q = 0;
						b0 = 1;
						a1 = a2 = b1 = b2 = 0;
						a = 0;
						z[0] = z[1] = 0;
					}

					void set(param f, param Q = root2.inv) {
						if (Filter::f != f || Filter::Q != Q) {
							Filter::f = f;
							Filter::Q = Q;

							const float w = f * fs.w;
							cos0 = cosf(w);
							sin0 = sinf(w);

							if (Q < 0.5) Q = 0.5;
							a = sin0 / (2.f * Q);
							init();
						}
					}

					virtual void init() = 0;

					void process() noexcept {
						const float z0 = z[0];
						const float z1 = z[1];
						const float y = b0 * in + z0;
						z[0] = b1 * in - a1 * y + z1;
						z[1] = b2 * in - a2 * y;
						out = y;
					}
				};

				// Low-pass filter (RBJ)
				struct LPF : Filter {
					void init() {
						constant a0 = { 1.f + a };
						a1 = a0.inv * (-2.f * cos0);
						a2 = a0.inv * (1.f - a);

						b2 = b0 = a0.inv * (1.f - cos0) * 0.5f;
						b1 = a0.inv * (1.f - cos0);
					}
				};

				typedef LPF HCF; // = High-cut Filter
				typedef LPF HRF; // = High-reject Filter

				// High-pass filter (RBJ)
				struct HPF : Filter {
					void init() {
						constant a0 = { 1.f + a };
						a1 = a0.inv * (-2.f * cos0);
						a2 = a0.inv * (1.f - a);

						b2 = b0 = a0.inv * (1.f + cos0) * 0.5f;
						b1 = a0.inv * -(1.f + cos0);
					}
				};

				typedef HPF LCF; // = Low-cut Filter
				typedef HPF LRF; // = Low-reject Filter

				// Biquad Band-pass Filter (RBJ)
				struct BPF : Filter {
					enum Gain {
						ConstantSkirtGain,
						ConstantPeakGain
					};

					BPF& operator=(Gain gain) {
						init_gain = gain == ConstantSkirtGain ? &BPF::init_skirt : &BPF::init_peak;
						init();
						return *this;
					}

					using Init = void(BPF::*)(void);
					Init init_gain = &BPF::init_peak;
					void init() { (this->*init_gain)(); }

					// constant skirt gain
					void init_skirt() {
						constant a0 = { 1.f + a };
						a1 = a0.inv * (-2.f * cos0);
						a2 = a0.inv * (1.f - a);

						b0 = a0.inv * sin0 * 0.5f;
						b1 = 0;
						b2 = -b0;
					}

					void init_peak() {
						// constant peak gain
						constant a0 = { 1.f + a };
						a1 = a0.inv * (-2.f * cos0);
						a2 = a0.inv * (1.f - a);

						b0 = a0.inv * a;
						b1 = 0;
						b2 = a0.inv * -a;
					}
				};

				// Band-reject (notch) Filter 
				struct BRF : Filter {
					void init() {
						constant a0 = { 1.f + a };
						b1 = a1 = a0.inv * (-2.f * cos0);
						a2 = a0.inv * (1.f - a);
						b0 = b2 = a0.inv;
					}
				};

				typedef BRF BSF; // = Band-stop Filter

				// All-pass filter (APF)
				struct APF : Filter {
					void init() {
						constant a0 = { 1.f + a };
						b1 = a1 = a0.inv * (-2.f * cos0);
						b0 = a2 = a0.inv * (1.f - a);
						b2 = 1.f;
					}
				};
			}
		}
	}

	// Envelope Followers
	struct Envelope::Follower : Modifier {

		// Attack / Release IIR Filter (~Butterworth when attack == release)
		struct AR : Modifier {
			param attack = 0;
			param release = 0;

			param A = 1, R = 1;

			void set(param attack, param release) {
				if (AR::attack != attack || AR::release != release) {
					AR::attack = attack;
					AR::release = release;
					A = 1.f - (attack == 0.f ? 0.f : expf(-1.0f / (fs * attack)));
					R = 1.f - (release == 0.f ? 0.f : expf(-1.0f / (fs * release)));
				}
			}

			void process() {
				const float smoothing = in > out ? A : R;
				(out + smoothing * (in - out)) >> out;
			}
		} ar;
			 	
		// Peak / RMS Envelope Follower (default; filter-based)
		Follower() { set(0.01f, 0.1f); }
		void set(param attack, param release) {
			ar.set(attack, release);
		}

		Follower& operator=(klang::Mode mode) {
			_process = (mode == RMS) ? &Follower::rms : &Follower::peak;
			return *this;
		}

		using Process = void(Follower::*)();
		Process _process = &Follower::rms;

		void process() {	(this->*_process)();	}

		void peak() {	abs(in) >> ar >> out;					}
		void rms() {	(in * in) >> ar >> sqrt >> out;		}

		// Average/RMS Envelope Follower (windowed/moving average)
		template<int WINDOW>
		struct Window : Modifier
		{
			AR ar;
			buffer buffer;
			double sum = 0; // NB: must be 64-bit to avoid rounding errors
			static constexpr constant window = { WINDOW };

			Window() : buffer(WINDOW, 0) {
				set(0.01f, 0.1f);
			}

			void set(param attack, param release) {
				ar.set(attack, release);
			}

			Window& operator=(klang::Mode mode) {
				_process = (mode == RMS) ? &Window::rms : &Window::mean;
				return *this;
			}

			using Process = void(Window::*)();
			Process _process = &Window::rms;

			void process() { (this->*_process)(); }

			void mean() { 
				sum -= double(buffer);
				abs(in) >> buffer;
				sum += double(buffer++);
				if (buffer.finished())
					buffer.rewind();
				sum * window.inv >> ar >> out;
			}
			void rms() {  
				sum -= double(buffer);
				(in * in) >> buffer;
				sum += double(buffer++);
				if (buffer.finished())
					buffer.rewind();
				sum* window.inv >> sqrt >> ar >> out;
			}
		};
	};

	namespace basic {
		using namespace klang;

		using namespace Generators::Basic;
		using namespace Filters::Basic;
		using namespace Filters::Basic::Biquad;		
	};

	namespace optimised {
		using namespace klang;

		using namespace Generators::Fast;
		using namespace Filters::Basic;
		using namespace Filters::Basic::Biquad;
	};

	namespace minimal {
		using namespace klang;
	};

	//using namespace optimised;
};

//using namespace klang;