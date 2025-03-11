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

#ifdef __wasm__
#define THREAD_LOCAL 
static inline float _sqrt(float x) { return __builtin_sqrtf(x); }
static inline float _abs(float x) { return __builtin_fabsf(x); }
#define SQRT _sqrt
#define SQRTF _sqrt
#define ABS _abs
#define FABS _abs
#endif
#ifdef __APPLE__
#define THREAD_LOCAL
#define SQRT ::sqrt
#define SQRTF ::sqrtf
#define ABS ::abs
#define FABS ::fabsf
#endif
#if defined(__WIN32__) || defined(WIN32)
#define THREAD_LOCAL thread_local
#define SQRT ::sqrt
#define SQRTF ::sqrtf
#define ABS ::abs
#define FABS ::fabsf
#endif

#if defined(DEBUG) || defined(_DEBUG)
#define KLANG_DEBUG 1
#else
#define KLANG_DEBUG 0
#endif

#ifndef GRAPH_SIZE
#define GRAPH_SIZE 44100
#endif

#define GRAPH_SERIES 4

// provide access to original math functions through std:: prefix
namespace std {
	namespace klang {
		template<typename TYPE> TYPE sqrt(TYPE x) { return SQRT(x); }
		template<typename TYPE> TYPE abs(TYPE x) { return ABS(x); }
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

	/// Klang language version (major.minor.build.debug)
	struct Version {
		unsigned char major, minor, build, extra;
		bool isDebug() const { return extra & 1; }
		bool atLeast(unsigned char M, unsigned char m, unsigned char b) const {
			return M < major || (M == major && m < minor) || (M == major && m == minor && b <= build);
		}
		bool below(unsigned char M, unsigned char m, unsigned char b) const { return !atLeast(M, m, b); }
	}
	static constexpr version = { 0, 7, 8, KLANG_DEBUG };

	/// Klang mode identifiers (e.g. averages, level following)
	enum Mode { Peak, RMS, Mean };

#define DENORMALISE 1.175494e-38f

	/// Constant scalar (pre-converted to double, float and int).
	struct constant {

		/// Create a constant from the given value.
		constexpr constant(double value) noexcept
			: d(value), f(static_cast<float>(value)), i(static_cast<int>(value)), inv(value == 0.0f ? 0.0f : static_cast<float>(1.0 / value)) {
		}

		const double d; //!< Constant as double
		const float f; //!< Constant as float
		const int i; //!< Constant as integer
		const float inv; //!< Inverse of constant

		constexpr operator float() const noexcept { return f; }

		/// Constant raised to the power, x.
		float operator^(float x) const { return std::pow(f, x); }
		float operator^(int x) const { return static_cast<float>(std::pow(d, x)); }
		float operator^(double x) const { return static_cast<float>(std::pow(d, x)); }
	};

#define CONSTANT constexpr constant

#if __cplusplus == 201703L
#pragma warning(disable:4996) // disable deprecated warning
	/// @internal
	template <typename T>
	inline constexpr bool is_literal = std::is_literal_type_v<T>;
#else
	/// @internal
	template <typename T>
	inline constexpr bool is_literal = std::is_trivially_constructible_v<T> && std::is_trivially_copyable_v<T> && std::is_trivially_destructible_v<T>;
#endif
	/// @internal
	template<typename Base, typename Derived>
	constexpr bool is_derived_from() {
		return std::is_base_of_v<Base, Derived>;
	}
	/// @internal
	template<typename Head, typename... Tail>
	constexpr bool are_scalars(Head&& head, Tail&&... tail) {
		using T = std::decay_t<decltype(head)>;
		return std::is_scalar_v<T> && ((sizeof... (Tail)) == 0 || are_scalars(std::forward<decltype(tail)>(tail)...));
	}
	/// @internal
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
	/// @internal
	template <typename BASE, typename EXP>
	using power_t = typename std::conditional_t<std::is_integral_v<BASE>, float, BASE>;

	/// Raise @a base to the power @a exp.
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
		}
		else if constexpr (std::is_floating_point_v<EXP>) {
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

	/// Raise @a base to the power @a exp.
	template <typename BASE, typename EXP>
	inline constexpr power_t<BASE, EXP> power(BASE base, EXP exp) {
		if constexpr (std::is_integral_v<BASE>) {
			return power(float(base), exp);
		}
		else if constexpr (std::is_integral_v<EXP>) {
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
		}
		else if constexpr (std::is_floating_point_v<EXP>) {
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

	/// Return the minimum of two values.
	template<typename TYPE1, typename TYPE2> inline TYPE1 min(TYPE1 a, TYPE2 b) { return a < b ? a : (TYPE1)b; };

	/// Return the minimum of two values.
	template<typename TYPE1, typename TYPE2> inline TYPE1 max(TYPE1 a, TYPE2 b) { return a > b ? a : (TYPE1)b; };

	/// The mathematical constant, pi (and it's inverse).
	constexpr constant pi = { 3.1415926535897932384626433832795 };

	/// The natural logorithm of 2 (and it's inverse).
	constexpr constant ln2 = { 0.6931471805599453094172321214581 };

	/// The square root of 2 (and it's inverse).
	constexpr constant root2 = { 1.4142135623730950488016887242097 };

	/// Generates a random number between min and max. Use an integer types for whole numbers.
	template<typename TYPE>
	inline static TYPE random(const TYPE min, const TYPE max) { return rand() * ((max - min) / (TYPE)RAND_MAX) + min; }

	/// Set the random seed (to allow repeatable random generation).
	inline static void random(const unsigned int seed) { srand(seed); }

	/// A function that handles an event.
	typedef void event;

	/// Variable-sized array, pre-allocated to a max. capacity.
	template<typename TYPE, int CAPACITY>
	struct Array {
		unsigned int count = 0;
		TYPE items[CAPACITY];

		/// The maximum capacity of the array.
		static int capacity() { return CAPACITY; }

		/// The current number of items in the array.
		unsigned int size() const { return count; }

		/// Add the specified item to the end of the array.
		void add(const TYPE& item) {
			if (count < CAPACITY)
				items[count++] = item;
		}

		/// Add a blank item to the end of the array, returning a pointer that allows the item to be modified.
		TYPE* add() {
			if (count < CAPACITY)
				return &items[count++];
			return nullptr;
		}

		/// Clear the array contents. Only resets the item count, without wiping memory.
		void clear() { count = 0; }

		/// Find the maximum value in the array.
		float max() const {
			float max = 0.f;
			for (unsigned int i = 0; i < count; i++)
				if (abs(items[i]) > max)
					max = items[i];
			return max;
		}

		/// Find the mean average of values in the array.
		float mean() const {
			float sum = 0.f;
			for (unsigned int i = 0; i < count; i++)
				sum += abs(items[i]);
			return sum / count;
		}

		/// Find the root mean squared (RMS) average of values in the array.
		float rms() const {
			float sum = 0.f;
			for (unsigned int i = 0; i < count; i++)
				sum += items[i] * items[i];
			return sqrt(sum / count);
		}

		/// Normalises values in the array to the specified @a target, based on peak, mean, or RMS value;
		void normalise(float target = 1.f, int mode = Peak) {
			if (!count) return;

			const float current = (mode == Peak) ? max() : (mode == Mean) ? mean() : rms();
			const float scale = current == 0.f ? 0.f : target / current;
			for (int i = 0; i < count; i++)
				items[i] *= scale;
		}

		/// Returns a reference to the array item at the given index.
		TYPE& operator[](int index) { return items[index]; }

		/// Returns a read-only reference to the array item at the given index.
		const TYPE& operator[](int index) const { return items[index]; }

		/// Construct an array given the specified values.
		Array(std::initializer_list<TYPE> init_list) {
			count = std::min(static_cast<unsigned int>(init_list.size()), static_cast<unsigned int>(CAPACITY));
			std::copy_n(init_list.begin(), count, items);
		}

		/// Construct an empty array.
		Array() = default; // Default constructor to allow empty initialization
	};

	struct Memory
	{
		typedef unsigned char Byte;
		typedef Byte* Pointer;

		Pointer start, ptr, end;
		size_t size;
		bool owned;

		Memory() {
			detach();
		}

		Memory(size_t size) {
			create(size);
		}

		~Memory() {
			free();
		}

		// attach to an existing buffer
		void attach(unsigned char* data, size_t size, bool own = false) {
			Memory::size = size;
			start = ptr = data;
			end = start + size;
			owned = own;
		}

		// detach from the buffer (without freeing it)
		void detach() {
			start = ptr = end = nullptr;
			size = 0;
			owned = false;
		}

		// allocate memory
		bool create(size_t size) {
			free();
			start = ptr = new Byte[size];
			if (start) {
				Memory::size = size;
				end = start + size;
				owned = true;
				return true;
			}
			return false;
		}

		// clear memory
		void clear() {
			memset(start, 0, size);
		}

		// rewind memory to start
		void rewind() {
			ptr = start;
		}

		// rewind memory
		void rewind(int bytes) {
			ptr -= bytes;
		}

		// skip / fastforward memory
		void skip(int bytes) {
			ptr += bytes;
		}

		// release memory
		void free() {
			if (owned && start) {
				ptr = end = nullptr;
				delete[] start;
				start = nullptr;
				size = 0;
			}
		}

		// check if pointer is at end of memory
		bool finished() const {
			return ptr >= end;
		}

		// return memory-mapped file pointer
		FILE* file() {
			FILE* f = fopen("/dev/null", "wt");
			if (!f) f = fopen("nul", "wt");
			setvbuf(f, (char*)start, _IOFBF, size);
			return f;
		}

		bool load(const char* path) {
			FILE* f = fopen(path, "rb");
			if (!f) return false;

			// find out length of file
			fseek(f, 0, SEEK_END);
			size_t length = ftell(f);
			fseek(f, 0, SEEK_SET);

			// allocate memory and read file
			if (!create(length)) {
				fclose(f);
				return false;
			}
			fread(start, 1, length, f);
			fclose(f);

			return true;
		}

		// retrieve the next value from memory
		template<typename TYPE>
		bool get(TYPE& value) {
			if (ptr + sizeof(TYPE) > end) {
				ptr = end;
				return false;
			}
			value = *(TYPE*)ptr;
			ptr += sizeof(TYPE);
			return true;
		}

		// retrieve the next value from memory
		template<typename TYPE>
		operator TYPE() {
			TYPE value;
			get(value);
			return value;
		}

		// add a value to the memory
		template<typename TYPE>
		bool add(const TYPE& value) {
			if (ptr + sizeof(TYPE) > end)
				return false;
			*(TYPE*)ptr = value;
			ptr += sizeof(TYPE);
			return true;
		}

		// add a value to the memory
		template<typename TYPE>
		Memory& operator+=(const TYPE& value) {
			add(value);
			return *this;
		}

		// check if memory is equal to a value
		template<typename TYPE>
		bool operator==(const TYPE& value) const {
			if (ptr + sizeof(TYPE) > end)
				return false;
			return value == *(TYPE*)ptr;
		}

		// copy memory from another memory object (with resize)
		Memory& operator=(const Memory& memory) {
			if (ptr + memory.size > end)
				return *this;
			create(memory.size);
			memcpy(start, memory.start, memory.size);
			return *this;
		}

		// attach memory to another memory object
		Memory& operator=(Memory* memory) {
			free();
			attach(memory->start, memory->size);
			return *this;
		}

		// add memory data from another memory object
		Memory& operator+=(const Memory& memory) {
			if (ptr + memory.size > end)
				return *this;
			memcpy(ptr, memory.start, memory.size);
			ptr += memory.size;
			return *this;
		}

		//operator int() {
		//	if (ptr + sizeof(int) > end) {
		//		ptr = end;
		//		return 0;
		//	}
		//	ptr += sizeof(int);
		//	return *(int*)(ptr - sizeof(int));
		//}

		//operator float() {
		//	if (ptr + sizeof(float) > end) {
		//		ptr = end;
		//		return 0;
		//	}
		//	ptr += sizeof(float);
		//	return *(float*)(ptr - sizeof(float));
		//}

	//	int geti1() {
	//		if ((++_ptr) > _end) {
	//			if (_ptr == _end)
	//				return (*(BYTE*)(_ptr - 1));
	//			_ptr = _end;
	//			return 0;
	//		}
	//		return (*(int*)(_ptr - 1)) & 0xFF;
	//	}

	//	int geti2() {
	//		if ((_ptr += 2) >/*=*/ _end) {
	//			//            if(_ptr == _end)
	//			//                return (*(short*)(_ptr - 2));
	//			_ptr = _end;
	//			return 0;
	//		}
	//		return (*(int*)(_ptr - 2)) & 0xFFFF;
	//	}

	//	//	int geti3() {
	//	//		if ((_ptr += 3) > _end) {
	//	//			_ptr = _end;
	//	//			return 0;
	//	//		}
	//	//		return (*(int*)(_ptr - 3)) & 0xFFFFFF;
	//	//	}

	//	int geti4() {
	//		if ((_ptr += 4) > _end) {
	//			_ptr = _end;
	//			return 0;
	//		}
	//		return (*(int*)(_ptr - 4)) & 0xFFFFFFFF;
	//	}

	//	int geti(const int size) {
	//		switch (size) {
	//		case 1: return geti1();
	//		case 2: return geti2();
	//			//		case 3: return geti3();
	//		case 4: return geti4();
	//		default:
	//			int number = getc();
	//			for (int i = 1; i < size; i++)           // (collate bytes into multi-byte integer)
	//				number += (1 << (i * 8)) * getc();  // pow(256.0,  i) * getc();		//
	//			return number;							// (return results)
	//		}
	//	}

	//	int getbe(int size) {
	//		int number = getc();
	//		while (--size) {
	//			number <<= 8;
	//			number += getc();
	//		}
	//		return number;
	//	}

	//	bool putc(char c) {
	//		if (_ptr < _end) {
	//			*_ptr++ = c;
	//			return true;
	//		}
	//		return false;
	//	}

	//	size_t write(const void* src, const size_t size, const size_t count) {
	//		if ((_ptr + size * count) <= _end) {
	//			memcpy(_ptr, src, size * count);
	//			_ptr += size * count;
	//			return size;
	//		}
	//		else {
	//			_ptr = _end;
	//		}
	//		return 0;
	//	}

	//	/*int getshort(){
	//		_ptr+=sizeof(short);
	//		if(_ptr > _end){
	//			_ptr = _end;
	//			return 0;
	//		}
	//		return *(int*)(_ptr-sizeof(short));
	//	}

	//	int getint(){
	//		_ptr+=sizeof(int);
	//		if(_ptr > _end){
	//			_ptr = _end;
	//			return 0;
	//		}
	//		return *(int*)(_ptr-sizeof(int));
	//	}*/

	//	bool eof() const {
	//		return _ptr >= _end;
	//	}

	//	int getvari(void)
	//	{
	//		int    value;
	//		short	c;

	//		if ((value = getc()) & 0x80) {
	//			value &= 0x7f;
	//			do {
	//				value = (value << 7) + ((c = getc()) & 0x7f);
	//			} while (c & 0x80);
	//		}
	//		return value;
	//	}

	//	char* gets(int length) {
	//		if ((_ptr + length) >= _end) {
	//			_ptr = _end;
	//			_string[0] = 0;
	//		}
	//		else {
	//			memcpy(_string, _ptr, MIN(length, 255));
	//			_string[MIN(length, 255)] = 0;
	//			_ptr += length;
	//		}
	//		return _string;
	//	}

	//	char* gets0(int length) {
	//		if ((_ptr + length) >= _end) {
	//			_ptr = _end;
	//			_string[0] = 0;
	//		}
	//		else {
	//			memcpy(_string, _ptr, MIN(length, 255));
	//			_string[MIN(length, 255)] = 0;
	//			if (!_string[0])
	//				_string[0] = ' ';
	//			_ptr += length;
	//		}
	//		return _string;
	//	}

	//	char* gets_any(const char* const of) {
	//		_string[0] = 0;

	//		int length = 0;
	//		while (!eof() && length < 255) {
	//			unsigned char ch = *_ptr;
	//			if (strchr(of, ch)) { // valid character
	//				_string[length++] = ch;
	//				_ptr++;
	//			}
	//			else {
	//				_string[length] = 0;
	//				return _string;
	//			}
	//		}
	//		_string[length] = 0;
	//		return _string;
	//	}

	//	static bool isNumeric(char x) {
	//		return (x >= '0' && x <= '9');
	//	}

	//	char* gets_numeric() {
	//		_string[0] = 0;

	//		int length = 0;
	//		while (!eof() && length < 255) {
	//			unsigned char ch = *_ptr;
	//			if (isNumeric(ch)) { // valid character
	//				_string[length++] = ch;
	//				_ptr++;
	//			}
	//			else {
	//				_string[length] = 0;
	//				return _string;
	//			}
	//		}
	//		_string[length] = 0;
	//		return _string;
	//	}

	//	static bool isHexadecimal(char x) {
	//		return (x >= 'A' && x <= 'F') || (x >= '0' && x <= '9');
	//	}

	//	char* gets_hex() {
	//		_string[0] = 0;

	//		int length = 0;
	//		while (!eof() && length < 255) {
	//			unsigned char ch = *_ptr;
	//			if (isHexadecimal(ch)) { // valid character
	//				_string[length++] = ch;
	//				_ptr++;
	//			}
	//			else {
	//				_string[length] = 0;
	//				return _string;
	//			}
	//		}
	//		_string[length] = 0;
	//		return _string;
	//	}

	//	void gets(char* string, int length) {
	//		if ((_ptr + length) >= _end) {
	//			_ptr = _end;
	//			return;
	//		}
	//		memcpy(string, _ptr, length);
	//		string[length] = 0;
	//		_ptr += length;
	//	}

	//	/*	void gets(std::string& string, int length){
	//			if((_ptr + length) >= _end){
	//				_ptr = _end;
	//				return;
	//			}

	//	//		memcpy(string, _ptr, length);
	//	//		string[length] = 0;
	//			string.copy((char*)_ptr, length);
	//			string.append(0);

	//			_ptr += length;
	//		}*/

	//	void gets(char* string) {
	//		if (eof()) return;
	//		gets(string, getc());
	//	}

	//	/*void gets0(char* string, int length){
	//		if((_ptr + length) >= _end){
	//			_ptr = _end;
	//			return;
	//		}
	//		memcpy(string, _ptr, length);
	//		string[length] = 0;
	//		if(string[0] == 0)
	//			string[0] = ' ';
	//		_ptr += length;
	//	}*/

	//	void gets(char* string, char terminator, unsigned int length) {
	//		const BYTE_PTR _end = std::min(_ptr + length, this->_end);
	//		BYTE_PTR _offset;
	//		for (_offset = _ptr; _offset < _end; _offset++) {
	//			if (*_offset == terminator)
	//				break;
	//		}
	//		//		if(_offset != _end){
	//		const unsigned int _len = (unsigned int)(_offset - _ptr);
	//		memcpy(string, _ptr, _len);
	//		string[std::min(length - 1, _len)] = 0;
	//		//		}
	//		_ptr = _offset;
	//	}

	//	static bool isAlphaNumeric(char x) {
	//		return (x >= 'a' && x <= 'z') || (x >= 'A' && x <= 'Z') || (x >= '0' && x <= '9');
	//	}

	//	void gettag(char* string) {
	//		while (isAlphaNumeric(*_ptr)) {
	//			*string++ = *_ptr++;
	//		}
	//		*string = 0;
	//	}

	//	void getcs(char* string) {
	//		if (_ptr > _end) {
	//			string[0] = 0;
	//			return;
	//		}
	//		const int length = *_ptr;
	//		if ((_ptr + length) > _end) {
	//			string[0] = 0;
	//			return;
	//		}
	//		_ptr++;
	//		memcpy(string, _ptr, length);
	//		_ptr += length;
	//	}

	//	int read(void* to, const int size) {
	//		if ((_ptr + size) > _end) {
	//			_ptr = _end;
	//			return 0;
	//		}
	//		memcpy(to, _ptr, size);
	//		_ptr += size;
	//		return size;
	//	}

	//	void skip(const int size) {
	//		if ((_ptr + size) > _end) {
	//			_ptr = _end;
	//			return;
	//		}
	//		_ptr += size;
	//	}

	//	void skip_any(char token) {
	//		while (!eof()) {
	//			if (*_ptr != token)
	//				return;
	//			_ptr++;
	//		}
	//	}

	//	void skip_any(const char* tokens) {
	//		while (!eof()) {
	//			const char* pTokens = tokens;
	//			while (*pTokens != '\0') {
	//				if (*_ptr == *pTokens)
	//					break;
	//				pTokens++;
	//			}
	//			if (*pTokens)
	//				_ptr++;
	//			else
	//				return;
	//		}
	//	}

	//	bool skip_to(char token) {
	//		while (!eof()) {
	//			if (*_ptr == token)
	//				return true;
	//			_ptr++;
	//		}
	//		return false;
	//	}

	//	bool skip_to(const char* token) {
	//		size_t length = strlen(token);
	//		while (!eof()) {
	//			if (!memcmp(token, _ptr, length)) {
	//				_ptr += length;
	//				return true;
	//			}
	//			_ptr++;
	//		}
	//		return false;
	//	}

	//	void skip_to_any(const char* tokens) {
	//		while (!eof()) {
	//			const char* pTokens = tokens;
	//			while (*pTokens != '\0') {
	//				if (*_ptr == *pTokens++)
	//					return;
	//			}
	//			_ptr++;
	//		}
	//	}

	//	bool match_i(const char* stuff) {													// matches given text with file
	//		size_t length = strlen(stuff);																	// (reset expectation)
	//		if (!_strnicmp(stuff, (const char*)_ptr, length)) {
	//			_ptr += length;
	//			return true;
	//		}
	//		return false;
	//	}

	//	bool match(const char* stuff) {													// matches given text with file
	//		size_t length = strlen(stuff);																	// (reset expectation)
	//		if (!memcmp(stuff, _ptr, length)) {
	//			_ptr += length;
	//			return true;
	//		}
	//		return false;
	//	}																																				//

	//	bool match(char stuff) {													// matches given text with file
	//		if (*_ptr == stuff) {
	//			_ptr++;
	//			return true;
	//		}
	//		return false;
	//	}

	//	// in-place character replacement
	//	int replace(BYTE find, BYTE replace) {
	//		int nbReplaced = 0;
	//		BYTE_PTR ptr = _buf;
	//		while (ptr < _end) {
	//			if (*ptr == find)
	//				*ptr = replace;
	//			nbReplaced++;
	//			ptr++;
	//		}
	//		return nbReplaced;
	//	}

	//	// in-place phrase replacement
	//	int replace_fast(const char* find, const char* replace) {
	//		// build new string
	//		const char* str = (const char*)_buf;
	//		const char* end = (const char*)_end;
	//		const int length = (int)strlen(replace);
	//		int nbReplaced = 0;

	//		while (str < end) {
	//			if (!memcmp(str, find, length)) { // match
	//				memcpy((void*)str, replace, length);
	//				str += length;
	//				nbReplaced++;
	//			}
	//			else {
	//				str++;
	//			}
	//		}
	//		return nbReplaced;
	//	}

	//	int replace(const char* find, const char* replace) {
	//		const int cbFind = (int)strlen(find);
	//		const int cbReplace = (int)strlen(replace);
	//		if (cbFind == cbReplace) // no memory re-allocation required
	//			return replace_fast(find, replace);

	//		// build new string
	//		std::string new_str;
	//		const char* str = (const char*)_buf;
	//		const char* last = str;
	//		const char* end = (const char*)_end;
	//		int nbReplaced = 0;

	//		while (str < end) {
	//			if (!memcmp(str, find, cbFind)) { // match
	//				new_str.append(last, str - last); // catch-up
	//				new_str += replace;
	//				str += cbFind;
	//				last = str;
	//				nbReplaced++;
	//			}
	//			else {
	//				str++;
	//			}
	//		}
	//		if (last < end)
	//			new_str.append(last, end - last);

	//		// replace buffer with new string
	//		create((int)new_str.length());
	//		memcpy(_buf, new_str.data(), new_str.length());
	//		return nbReplaced;
	//	}

	//	bool match_any(const char* stuffs) {
	//		while (*stuffs != '\0') {
	//			if (*_ptr == *stuffs++) {
	//				_ptr++;
	//				return true;
	//			}
	//		}
	//		return false;
	//	}

	//	unsigned int length() const { return _len; }
	//	const BYTE_PTR buffer() const { return _buf; }
	//	BYTE_PTR buffer() { return _buf; }
	//	BYTE_PTR pointer() { return _ptr; }
	//	BYTE_PTR* ppointer() { return &_ptr; }

	//	unsigned int offset() const { return (unsigned int)(_ptr - _buf); }
	//	bool seek(unsigned int offset) {
	//		_ptr = _buf + offset;
	//		if (_ptr > _end) {
	//			_ptr = _end;
	//			return false;
	//		}
	//		else return true;
	//	}
	//protected:
	//	BYTE_PTR _buf;
	//	unsigned int _len;

	//	BYTE_PTR _end;
	//	BYTE_PTR _ptr;

	//	char _string[256];
	};

	/// String of characters representing text.
	template<int SIZE>
	struct Text {
		/// The character buffer.
		char string[SIZE + 1] = { 0 };

		/// Maximum size of the string.
		int capacity() const { return SIZE; }

		/// Automatic cast to constant C-string.
		operator const char* () const { return string; }

		/// Return constant C-string.
		const char* c_str() const { return string; }

		/// Create a new Text object from a C-string.
		static Text from(const char* in) {
			Text text;
			text = in;
			return text;
		}

		/// Assign C-String to Text object.
		void operator=(const char* in) {
			memcpy(string, in, SIZE);
			string[SIZE] = 0;
		}

		///  Returns true if Text matches the given C-string.
		bool operator==(const char* in) const {
			return strcmp(string, in) == 0;
		}

		///  Returns true if Text does not matches the given C-string.
		bool operator!=(const char* in) const {
			return !operator==(in);
		}
	};

	/// A short Text object used to label controls.
	typedef Text<32> Caption;

	struct Output;

	struct relative;

	/// A mono audio signal (equivalent to a float).
	struct signal {
		float value;

		/// Create signal from a constant.
		signal(constant initial) : value(initial.f) {}

		/// Create signal from a 32-bit float..
		signal(const float initial = 0.f) : value(initial) {}

		/// Create signal from a 64-bit double.
		signal(const double initial) : value((const float)initial) {}

		/// Create signal from an 32-bit signed integer.
		signal(const int value) : value((const float)value) {}

		/// Feedback operator (prevents further processing of returned value).
		const signal& operator<<(const signal& input) {
			value = input;
			return *this;
		}

		/// Stream operator (feedforward; allows further processing)
		signal& operator>>(signal& destination) const {
			destination.value = value;
			return destination;
		}

		/// Assign processed output of \a in to signal.
		signal& operator=(Output& in);
		/// Adds processed output of \a in to signal.
		signal& operator+=(Output& in);

		/// Add (mix) another signal to the signal.
		signal& operator+=(const signal& x) { value += x.value; return *this; }
		/// Subtract another signal from the signal.
		signal& operator-=(const signal& x) { value -= x.value; return *this; }
		/// Multiply (modulate) signal by another signal.
		signal& operator*=(const signal& x) { value *= x.value; return *this; }
		/// Divide signal by another signal.
		signal& operator/=(const signal& x) { value /= x.value; return *this; }

		/// Add the specified amount to the signal.
		signal& operator+=(float x) { value += x; return *this; }
		/// Subtract the specified amount from the signal.
		signal& operator-=(float x) { value -= x; return *this; }
		/// Multiply signal by the specified amount.
		signal& operator*=(float x) { value *= x; return *this; }
		/// Divide signal by the specified amount.
		signal& operator/=(float x) { value /= x; return *this; }

		/// Add the specified amount to the signal.
		signal& operator+=(double x) { value += (float)x; return *this; }
		/// Subtract the specified amount from the signal.
		signal& operator-=(double x) { value -= (float)x; return *this; }
		/// Multiply signal by the specified amount.
		signal& operator*=(double x) { value *= (float)x; return *this; }
		/// Divide signal by the specified amount.
		signal& operator/=(double x) { value /= (float)x; return *this; }

		/// Add the specified amount to the signal.
		signal& operator+=(int x) { value += (float)x; return *this; }
		/// Subtract the specified amount from the signal.
		signal& operator-=(int x) { value -= (float)x; return *this; }
		/// Multiply signal by the specified amount.
		signal& operator*=(int x) { value *= (float)x; return *this; }
		/// Divide signal by the specified amount.
		signal& operator/=(int x) { value /= (float)x; return *this; }

		/// Add two signals together.
		signal operator+(float x) const { return value + x; }
		/// Subtract one signal from another.
		signal operator-(float x) const { return value - x; }
		/// Multiply (modulate) two signals.
		signal operator*(float x) const { return value * x; }
		/// Divide one signal by another.
		signal operator/(float x) const { return value / x; }

		/// Return a copy of the signal offset by x.
		signal operator+(double x) const { return value + (float)x; }
		/// Return a copy of the signal offset by -x.
		signal operator-(double x) const { return value - (float)x; }
		/// Return a copy of the signal scaled by x.
		signal operator*(double x) const { return value * (float)x; }
		/// Return a copy of the signal divided by.
		signal operator/(double x) const { return value / (float)x; }

		/// Return a copy of the signal offset by x.
		signal operator+(int x) const { return value + (float)x; }
		/// Return a copy of the signal offset by -x.
		signal operator-(int x) const { return value - (float)x; }
		/// Return a copy of the signal scaled by x.
		signal operator*(int x) const { return value * (float)x; }
		/// Return a copy of the signal divided by x.
		signal operator/(int x) const { return value / (float)x; }

		/// Return a copy of the signal raised to the power of x.
		signal operator^(float x) const { return power(value, x); }
		/// Return a copy of the signal raised to the power of x.
		signal operator^(double x) const { return power(value, x); }
		/// Return a copy of the signal raised to the power of x.
		signal operator^(int x) const { return power(value, x); }

		operator const float() const { return value; }
		operator float& () { return value; }

		/// Check if the signal contains a denormal value.
		bool isDenormal() const {
			const unsigned int bits = *(const unsigned int*)&value;
			return !(bits & 0x7F800000) && (bits & 0x007FFFFF);
		}

		/// Returns the number of channels (1 = mono).
		int channels() const { return 1; }

		/// Returns a copy of the signal to treat as a relative offset (e.g. for phase modulation).
		relative operator+() const;	// unary + operator produces relative signal
		/// Returns a copy of the signal to treat as a relative offset (e.g. for phase modulation).
		relative relative() const;	// 
	};

	/// @internal signal MUST compile as a float
	static_assert(sizeof(signal) == sizeof(float), "signal != float");
	IS_SIMPLE_TYPE(signal)

		/// A signal used as an offset relative to another signal.
		struct relative : public signal {};

	/// Returns a copy of the signal to treat as a relative offset (e.g. for phase modulation).
	inline relative signal::operator+() const { return { value }; }
	/// Returns a copy of the signal to treat as a relative offset (e.g. for phase modulation).
	inline relative signal::relative() const { return { value }; }

	/// Stream a literal / constant / scalar type into a signal.
	inline static signal& operator>>(float input, signal& destination) { // CHECK: should this be signal, rather than float?
		destination << signal(input);
		return destination;
	}

	/// A multi-channel audio signal (e.g. stereo).
	template<int CHANNELS = 2>
	struct signals {
		/// @cond
		union {
			signal value[CHANNELS]; ///< Array of channel values.
			struct {
				signal l; ///< Left channel
				signal r; ///< Right channel
			};
		};
		/// @endcond

		/// Return the mono mix of a stereo channel.
		signal mono() const { return (l + r) * 0.5f; }

		/// Return a reference to the signal at the specified index (0 = left, 1 = right).
		signal& operator[](int index) { return value[index]; }
		/// Return a read-only reference to the signal  at the specified index (0 = left, 1 = right).
		const signal& operator[](int index) const { return value[index]; }

		/// Create a stereo signal with the given value.
		signals(float initial = 0.f) : l(initial), r(initial) {}
		/// Create a stereo signal with the given value.
		signals(double initial) : l((float)initial), r((float)initial) {}
		/// Create a stereo signal with the given value.
		signals(int initial) : l((float)initial), r((float)initial) {}

		/// Create a stereo signal with the given left and right value.
		signals(float left, float right) : l(left), r(right) {}
		/// Create a stereo signal with the given left and right value.
		signals(double left, double right) : l((float)left), r((float)right) {}
		/// Create a stereo signal with the given left and right value.
		signals(int left, int right) : l((float)left), r((float)right) {}

		/// Create a multi-channel signal with the given channel values.
		template <typename... Args, typename = std::enable_if_t<(std::is_convertible_v<Args, signal> && ...)>>
		signals(Args&... initial) : value{ initial... } {}
		/// Create a multi-channel signal with the given channel values.
		template <typename... Args, typename = std::enable_if_t<(std::is_scalar_v<Args> && ...)>>
		signals(Args... initial) : value{ initial... } {}

		/// Returns the number of channels in the signal.
		int channels() const { return CHANNELS; }

		/// Feedback operator (prevents further processing of returned value).
		const signals& operator<<(const signals& input) {
			return this->operator=(input);
		}

		/// Stream operator (feedforward; allows further processing).
		signals& operator>>(signals& destination) const {
			destination = *this;
			return destination;
		}

		/// Add (mix) another signal to the signal.
		signals& operator+=(const signals x) { for (int v = 0; v < CHANNELS; v++) value[v] += x[v]; return *this; }
		/// Subtract another signal from the signal.
		signals& operator-=(const signals x) { for (int v = 0; v < CHANNELS; v++) value[v] -= x[v]; return *this; }
		/// Multiply (modulate) signal by another signal.
		signals& operator*=(const signals x) { for (int v = 0; v < CHANNELS; v++) value[v] *= x[v]; return *this; }
		/// Divide signal by another signal.
		signals& operator/=(const signals x) { for (int v = 0; v < CHANNELS; v++) value[v] /= x[v]; return *this; }

		/// Add two multi-channel signals together.
		signals operator+(const signals x) const { signals s = *this; for (int v = 0; v < CHANNELS; v++) s[v] += x[v]; return s; }
		/// Subtract one multi-channel signal from another.
		signals operator-(const signals x) const { signals s = *this; for (int v = 0; v < CHANNELS; v++) s[v] -= x[v]; return s; }
		/// Multiply (modulate) two multi-channel signals.
		signals operator*(const signals x) const { signals s = *this; for (int v = 0; v < CHANNELS; v++) s[v] *= x[v]; return s; }
		/// Divide one multi-channel signal by another.
		signals operator/(const signals x) const { signals s = *this; for (int v = 0; v < CHANNELS; v++) s[v] /= x[v]; return s; }

		//signals& operator+=(const signal x) { for (int v = 0; v < CHANNELS; v++) value[v] += x; return *this; }
		//signals& operator-=(const signal x) { for (int v = 0; v < CHANNELS; v++) value[v] -= x; return *this; }
		//signals& operator*=(const signal x) { for (int v = 0; v < CHANNELS; v++) value[v] *= x; return *this; }
		//signals& operator/=(const signal x) { for (int v = 0; v < CHANNELS; v++) value[v] /= x; return *this; }

		/// Return a copy of the multi-channel signal, adding a mono signal to each channel.
		signals operator+(const signal x) const { signals s = *this; for (int v = 0; v < CHANNELS; v++) s[v] += x; return s; }
		/// Return a copy of the multi-channel signal, subtracting a mono signal from each channel.
		signals operator-(const signal x) const { signals s = *this; for (int v = 0; v < CHANNELS; v++) s[v] -= x; return s; }
		/// Return a copy of the multi-channel signal, multiplying (modulating) each channel by a mono signal.
		signals operator*(const signal x) const { signals s = *this; for (int v = 0; v < CHANNELS; v++) s[v] *= x; return s; }
		/// Return a copy of the multi-channel signal, dividing each channel by a mono signal.
		signals operator/(const signal x) const { signals s = *this; for (int v = 0; v < CHANNELS; v++) s[v] /= x; return s; }

		/// Return a copy of the signal with each channel offset by x.
		signals operator+(float x) const { signals s = *this; for (int v = 0; v < CHANNELS; v++) s[v] += x; return s; }
		/// Return a copy of the signal with each channel offset by -x.
		signals operator-(float x) const { signals s = *this; for (int v = 0; v < CHANNELS; v++) s[v] -= x; return s; }
		/// Return a copy of the signal  with each channel scaled by x.
		signals operator*(float x) const { signals s = *this; for (int v = 0; v < CHANNELS; v++) s[v] *= x; return s; }
		/// Return a copy of the signal with each channel divided by x.
		signals operator/(float x) const { signals s = *this; for (int v = 0; v < CHANNELS; v++) s[v] /= x; return s; }

		/// Return a copy of the signal with each channel offset by x.
		signals operator+(double x) const { signals s = *this; for (int v = 0; v < CHANNELS; v++) s[v] += x; return s; }
		/// Return a copy of the signal with each channel offset by -x.
		signals operator-(double x) const { signals s = *this; for (int v = 0; v < CHANNELS; v++) s[v] -= x; return s; }
		/// Return a copy of the signal  with each channel scaled by x.
		signals operator*(double x) const { signals s = *this; for (int v = 0; v < CHANNELS; v++) s[v] *= x; return s; }
		/// Return a copy of the signal with each channel divided by x.
		signals operator/(double x) const { signals s = *this; for (int v = 0; v < CHANNELS; v++) s[v] /= x; return s; }

		/// Return a copy of the signal with each channel offset by x.
		signals operator+(int x) const { signals s = *this; for (int v = 0; v < CHANNELS; v++) s[v] += x; return s; }
		/// Return a copy of the signal with each channel offset by -x.
		signals operator-(int x) const { signals s = *this; for (int v = 0; v < CHANNELS; v++) s[v] -= x; return s; }
		/// Return a copy of the signal  with each channel scaled by x.
		signals operator*(int x) const { signals s = *this; for (int v = 0; v < CHANNELS; v++) s[v] *= x; return s; }
		/// Return a copy of the signal with each channel divided by x.
		signals operator/(int x) const { signals s = *this; for (int v = 0; v < CHANNELS; v++) s[v] /= x; return s; }
	};

	/// Return a copy of the signal with each channel offset by x.
	template<int CHANNELS = 2> inline signals<CHANNELS> operator+(float x, const signals<CHANNELS>& y) { return y + x; }
	/// Return a copy of the signal with each channel subtracted from  x.
	template<int CHANNELS = 2> inline signals<CHANNELS> operator-(float x, const signals<CHANNELS>& y) { return -y + x; }
	/// Return a copy of the signal  with each channel scaled by x.
	template<int CHANNELS = 2> inline signals<CHANNELS> operator*(float x, const signals<CHANNELS>& y) { return y * x; }
	/// Return a copy of the signal with each channel divided into x.
	template<int CHANNELS = 2> inline signals<CHANNELS> operator/(float x, const signals<CHANNELS>& y) {
		signals<CHANNELS> s = { 0.f };
		for (int c = 0; c < CHANNELS; c++)
			s[c] = x / y[c];
		return std::move(s);
	}

	/// @cond
	/// Helper class for converting units / quantities
	struct Conversion : public signal {
		using signal::signal;
		//        operator klang::signal& () { return signal; };
	};
	/// @endcond

	/// A phase or wavetable increment.
	struct increment {
		float amount; ///< The current phase.
		const float size; ///< The length of a a full cycle (e.g. 2 pi or wavetable size)

		/// Create a phase increment (default to radians).
		increment(float amount, const float size = 2 * pi) : amount(amount), size(size) {}
		/// Create a phase increment (wavetable size).
		increment(float amount, int size) : amount(amount), size((float)size) {}
		/// Create a phase increment.
		increment(float amount, double size) : amount(amount), size((float)size) {}

		/// Set current phase.
		increment& operator=(float in) { amount = in; return *this; }
	};

	struct Control;

	/// A signal used as a control parameter.
	struct param : public signal {
		param(constant in) : signal(in.f) {}
		param(const float initial = 0.f) : signal(initial) {}
		param(const signal& in) : signal(in) {}
		param(signal& in) : signal(in) {}
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

	/// @cond
	struct params {
		param* parameters;
		const int size;

		params(param p) : parameters(new param[1]), size(1) { parameters[0] = p; }
		params(std::initializer_list<param> params) : parameters(new param[params.size()]), size((int)params.size()) {
			int index = 0;
			for (param p : params)
				parameters[index++] = p;
		}

		param& operator[](int index) { return parameters[index]; }
	};
	/// @endcond

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

	/// Matrix processor
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

	/// @cond
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
			}
			else if constexpr (std::is_same<TYPE, float>()) {
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
	/// @endcond

	/// Control parameter (phase)
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

	/// Control parameter (pitch)
	struct Pitch : public param {
		//INFO("Pitch", 60.f, 0.f, 127.f)
		using param::param;

		//Pitch(float p = 60.f) : param(p) { };
		//Pitch(int p) : param((float)p) { };

		// convert note number to pitch class and octave (e.g. C#5)
		const char* text() const {
			THREAD_LOCAL static char buffer[32] = { 0 };
			const char* const notes[12] = { "C", "C#/Db", "D", "D#/Eb", "E", "F", "F#/Gb", "G", "G#/Ab", "A", "A#/Bb", "B" };
			snprintf(buffer, 32, "%s%d", notes[(int)value % 12], (int)value / 12);
			return buffer;
		}

		const Pitch* operator->() {
			Frequency = 440.f * power(2.f, (value - 69.f) / 12.f);
			return this;
		}

		THREAD_LOCAL static Conversion Frequency;

		template<typename TYPE>	Pitch operator+(TYPE in) { return value + in; }
		template<typename TYPE>	Pitch operator-(TYPE in) { return value - in; }
		template<typename TYPE>	Pitch operator*(TYPE in) { return value * in; }
		template<typename TYPE>	Pitch operator/(TYPE in) { return value / in; }
	};

	THREAD_LOCAL inline Conversion Pitch::Frequency;

	//	inline THREAD_LOCAL Pitch::Convert Pitch::Frequency;

		/// Control parameter (frequency)
	struct Frequency : public param {
		//INFO("Frequency", 1000.f, -FLT_MAX, FLT_MAX)
		using param::param;
		Frequency(float f = 1000.f) : param(f) {};
	};

	/// Sample rate constants
	static struct SampleRate {
		float f;        ///< sample rate (float)
		int i;          ///< sample rate (integer)
		double d;       ///< sample rate (double)
		float inv;      ///< 1 / sample rate (inverse)
		float w;        ///< angular frequency (omega)
		float nyquist;  ///< nyquist frequency (f / 2)

		SampleRate(float sr) : f(sr), i(int(sr + 0.001f)), d((double)sr), inv(1.f / sr), w(2.0f * pi * inv), nyquist(sr / 2.f) {}

		operator float() { return f; }
	} fs(44100); // sample rate

	struct Amplitude;

	/// Control parameter (idecibels)
	struct dB : public param {
		//INFO("dB", 0.f, -FLT_MAX, FLT_MAX)
		using param::param;

		dB(float gain = 0.f) : param(gain) {};

		const dB* operator->() const {
			Amplitude = power(10, value * 0.05f);
			return this;
		}

		THREAD_LOCAL static Conversion Amplitude;
	};

	/// Control parameter (linear amplitude)
	struct Amplitude : public param {
		//INFO("Gain", 1.f, -FLT_MAX, FLT_MAX)
		using param::param;

		Amplitude(float a = 1.f) : param(a) {};
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

		THREAD_LOCAL static Conversion dB;
	};

	THREAD_LOCAL inline Conversion dB::Amplitude;
	THREAD_LOCAL inline Conversion Amplitude::dB;

	/// Control parameter (velocity)
	typedef Amplitude Velocity;

	/// UI control / parameter
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

		/// Control size
		struct Size
		{
			Size(int x = -1, int y = -1, int width = -1, int height = -1)
				: x(x), y(y), width(width), height(height) {
			}

			int x;
			int y;
			int width;
			int height;

			bool isAuto() const { return x == -1 && y == -1 && width == -1 && height == -1; }
			bool isRelative() const { return x != -1 && y != -1; }
		};

		// Control Group
		struct Group {					// 48 bytes
			Caption name;				// 36 bytes (32+1 aligned)
			Control::Size size;			// 4 bytes
			unsigned int start = 0;		// 4 bytes
			unsigned int length = 0;	// 4 bytes

			bool contains(unsigned int c) const { return c >= start && c < (start + length); }
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
		operator signal () const { return value; }
		operator param() const { return value; }
		operator float() const { return value.value; }

		static constexpr float smoothing = 0.999f;
		signal smooth() { return smoothed = smoothed * smoothing + (1.f - smoothing) * value; }

		float range() const { return max - min; }
		float normalise(float value) const { return range() ? (value - min) / range() : std::clamp(value, 0.f, 1.f); }
		float normalised() const { return range() ? (value.value - min) / range() : std::clamp(value.value, 0.f, 1.f); }
		void setNormalised(float norm) { value.value = norm * range() + min; }

		operator Control* () { return this; }

		Control& set(float x) {
			value = std::clamp(x, min, max);
			return *this;
		}

		Control& operator+=(float x) { value += x; return *this; }
		Control& operator*=(float x) { value *= x; return *this; }
		Control& operator-=(float x) { value -= x; return *this; }
		Control& operator/=(float x) { value /= x; return *this; }

		//float operator+(float x) const { return value + x; }
		//float operator*(float x) const { return value * x; }
		//float operator-(float x) const { return value - x; }
		//float operator/(float x) const { return value / x; }

		template<typename TYPE> signal operator+(const Control& x) const { return value + (signal)x; }
		template<typename TYPE> signal operator*(const Control& x) const { return value * (signal)x; }
		template<typename TYPE> signal operator-(const Control& x) const { return value - (signal)x; }
		template<typename TYPE> signal operator/(const Control& x) const { return value / (signal)x; }

		template<typename TYPE> float operator+(TYPE& x) const { return value + (signal)x; }
		template<typename TYPE> float operator*(TYPE& x) const { return value * (signal)x; }
		template<typename TYPE> float operator-(TYPE& x) const { return value - (signal)x; }
		template<typename TYPE> float operator/(TYPE& x) const { return value / (signal)x; }

		template<typename TYPE> Control& operator<<(TYPE& in) { value = in; return *this; }		// assign to control with processing
		template<typename TYPE> Control& operator<<(const TYPE& in) { value = in; return *this; }	// assign to control without/after processing

		template<typename TYPE> TYPE& operator>>(TYPE& in) { return value >> in; }					// stream control to signal/object (allows processing)
		template<typename TYPE> const TYPE& operator>>(const TYPE& in) { return value >> in; }		// stream control to signal/object (no processing)
	};

	const Control::Size Automatic = { -1, -1, -1, -1 };
	const Control::Options NoOptions;

	/// Return a copy of the signal with each channel offset by x.
	template<int CHANNELS = 2> inline signals<CHANNELS> operator+(const Control& x, const signals<CHANNELS>& y) { return y + x.value; }
	/// Return a copy of the signal with each channel subtracted from  x.
	template<int CHANNELS = 2> inline signals<CHANNELS> operator-(const Control& x, const signals<CHANNELS>& y) { return -y + x.value; }
	/// Return a copy of the signal  with each channel scaled by x.
	template<int CHANNELS = 2> inline signals<CHANNELS> operator*(const Control& x, const signals<CHANNELS>& y) { return y * x.value; }
	/// Return a copy of the signal with each channel divided into x.
	template<int CHANNELS = 2> inline signals<CHANNELS> operator/(const Control& x, const signals<CHANNELS>& y) {
		signals<CHANNELS> s = { 0.f };
		for (int c = 0; c < CHANNELS; c++)
			s[c] = x.value / y[c];
		return std::move(s);
	}

	/// Mapped UI control
	struct ControlMap {
		Control* control;

		ControlMap() : control(nullptr) {};
		ControlMap(Control& control) : control(&control) {};

		operator Control& () { return *control; }
		operator signal& () { return control->value; }
		operator const signal& () const { return control->value; }
		operator param() const { return control->value; }
		operator float() const { return control->value; }
		signal smooth() { return control->smooth(); }

		template<typename TYPE> Control& operator<<(TYPE& in) { control->value = in; return *control; }		// assign to control with processing
		template<typename TYPE> Control& operator<<(const TYPE& in) { control->value = in; return *control; }	// assign to control without/after processing
	};

	IS_SIMPLE_TYPE(Control);

	inline param::param(Control& in) : signal(in.value) {}

	inline static Control Dial(const char* name, float min = 0.f, float max = 1.f, float initial = 0.f, Control::Size size = Automatic)
	{
		return { Caption::from(name), Control::ROTARY, min, max, initial, size, NoOptions, initial };
	}

	inline static Control Button(const char* name, Control::Size size = Automatic)
	{
		return { Caption::from(name), Control::BUTTON, 0, 1, 0.f, size, NoOptions, 0.f };
	}

	inline static Control Toggle(const char* name, bool initial = false, Control::Size size = Automatic)
	{
		return { Caption::from(name), Control::TOGGLE, 0, 1, initial ? 1.f : 0.f, size, NoOptions, initial ? 1.f : 0.f };
	}

	inline static Control Slider(const char* name, float min = 0.f, float max = 1.f, float initial = 0.f, Control::Size size = Automatic)
	{
		return { Caption::from(name), Control::SLIDER, min, max, initial, size, NoOptions, initial };
	}

	template<typename... Options>
	static Control Menu(const char* name, const Options... options)
	{
		Control::Options menu;
		const char* strings[] = { options... };
		int nbValues = sizeof...(options);
		for (int p = 0; p < nbValues; p++)
			menu.add(Caption::from(strings[p]));
		return { Caption::from(name), Control::MENU, 0, menu.size() - 1.f, 0, Automatic, menu, 0 };
	}

	template<typename... Options>
	static Control Menu(const char* name, Control::Size size, const Options... options)
	{
		Control::Options menu;
		const char* strings[] = { options... };
		int nbValues = sizeof...(options);
		for (int p = 0; p < nbValues; p++)
			menu.add(Caption::from(strings[p]));
		return { Caption::from(name), Control::MENU, 0, menu.size() - 1.f, 0, size, menu, 0 };
	}

	inline static Control Meter(const char* name, float min = 0.f, float max = 1.f, float initial = 0.f, Control::Size size = Automatic)
	{
		return { Caption::from(name), Control::METER, min, max, initial, size, NoOptions, initial };
	}

	inline static Control PitchBend(Control::Size size = Automatic)
	{
		return { { "PITCH\nBEND" }, Control::WHEEL, 0.f, 16384.f, 8192.f, size, NoOptions, 8192.f };
	}

	inline static Control ModWheel(Control::Size size = Automatic)
	{
		return { { "MOD\nWHEEL" }, Control::WHEEL, 0.f, 127.f, 0.f, size, NoOptions, 0.f };
	}

	struct Group {
		const char* name;
		Control::Size size;
		std::vector<Control> controls;

		Group(const char* name, std::initializer_list<Control> controls) : name(name), size(Automatic), controls{ controls } {}

		template <typename... Controls>
		Group(const char* name, Controls... ctrls) : name(name), size(Automatic), controls{ std::forward<Controls>(ctrls)... } {}

		template <typename... Controls>
		Group(Controls... ctrls) : name(""), size(Automatic), controls{ std::forward<Controls>(ctrls)... } {}

		Group(const char* name, Control::Size size, std::initializer_list<Control> controls) : name(name), size(size), controls{ controls } {}

		template <typename... Controls>
		Group(const char* name, Control::Size size, Controls... ctrls) : name(name), size(size), controls{ std::forward<Controls>(ctrls)... } {}

		template <typename... Controls>
		Group(Control::Size size, Controls... ctrls) : name(""), size(size), controls{ std::forward<Controls>(ctrls)... } {}
	};

	/// Plugin UI controls
	struct Controls : Array<Control, 128>
	{
		float value[128] = { 0 };
		Array<Control::Group, 10> groups;

		void operator+= (const Control& control) {
			items[count++] = control;
		}

		void operator= (const Controls& controls) {
			for (int c = 0; c < 128 && controls[c].type != Control::NONE; c++)
				operator+=(controls[c]);
		}

		//void operator=(std::initializer_list<Control> controls) {
		//	for(auto control : controls)
		//		operator+=(control);
		//}

		void operator=(std::initializer_list<klang::Group> controls) {
			for (auto group : controls) {
				if (group.name && group.name[0])
					groups.add({ Caption::from(group.name), group.size, count, (unsigned int)group.controls.size() });
				for (auto control : group.controls)
					operator+=(control);
			}
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

		void group(const char* name, unsigned int start, unsigned int length, Control::Size size = Automatic) {
			const Control::Group g = { Caption::from(name), size, start, length };
			groups.add(g);
		}

		//float& operator[](int index) { return items[index].value; }
		//signal operator[](int index) const { return items[index].value; }

		//Control& operator()(int index) { return items[index]; }
		//Control operator()(int index) const { return items[index]; }
	};

	typedef Array<float, 128> Values;

	/// Factory preset
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

	/// Factory presets
	struct Presets : Array<Preset, 128> {
		void operator += (const Preset& preset) {
			items[count++] = preset;
		}

		void operator= (const Presets& presets) {
			for (int p = 0; p < 128 && presets[p].name[0]; p++)
				operator+=(presets[p]);
		}

		void operator=(std::initializer_list<Preset> presets) {
			for (auto preset : presets)
				operator+=(preset);
		}

		template<typename... Values>
		void add(const char* name, const Values... values) {
			items[count].name = name;

			const float preset[] = { values... };
			int nbValues = sizeof...(values);
			for (int p = 0; p < nbValues; p++)
				items[count].values.add(preset[p]);
			count++;
		}
	};

	/// Audio buffer (mono)
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
			: owned(false), samples(buffer), size(size) {
			rewind();
		}

		buffer(float* buffer, int size, float initial)
			: owned(false), samples(buffer), size(size) {
			rewind();
			set(initial);
		}

		buffer(int size = 1, float initial = 0)
			: mask(capacity(size) - 1), owned(true), samples(new float[capacity(size)]), size(size) {
			rewind();
			set(initial);
		}

		virtual ~buffer() {
			if (owned)
				delete[] samples;
		}

		void attach(const buffer& buffer, int size = 0) {
			samples = buffer.samples;
			rewind();
			end = (signal*)&samples[size];
		}

		void rewind(int offset = 0) {
#ifdef _MSC_VER
			_controlfp_s(nullptr, _DN_FLUSH, _MCW_DN);
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
			memcpy(samples, in.samples, min(size, in.size) * sizeof(float));
			return *this;
		}

		buffer& operator<<(const signal& in) {
			*ptr = in;
			return *this;
		}

		float* data() { return samples; }
		const float* data() const { return samples; }
	};

	namespace variable {
		class buffer {
			std::unique_ptr<klang::buffer> ptr;
		public:
			int size = 0;

			buffer(int size = 1, float initial = 0.f) : size(size), ptr(new klang::buffer(size, initial)) {}
			buffer(const klang::buffer& buffer) : size(buffer.size), ptr(new klang::buffer(buffer.size)) { *ptr = buffer; };
			//buffer(klang::buffer* buffer) : size(buffer->size), ptr(new klang::buffer(buffer->data(), buffer->size)) { };
			virtual ~buffer() { ptr.reset(); };

			operator klang::buffer& () { return *ptr; }
			operator const klang::buffer& () const { return *ptr; }
			void resize(int size) {
				if (size != ptr->size)
					ptr = std::unique_ptr<klang::buffer>(new klang::buffer(buffer::size = size));
			}

			void rewind(int offset = 0) { ptr->rewind(offset); }
			void clear() { ptr->clear(); }
			void clear(int size) { ptr->clear(size); }	
			int offset() const { return ptr->offset(); }
			void set(float value = 0) { ptr->set(value); }
			signal& operator[](int offset) { return ptr->operator[](offset); }
			signal operator[](float offset) { return ptr->operator[](offset); }
			signal operator[](float offset) const { return ptr->operator[](offset); }
			const signal& operator[](int index) const { return ptr->operator[](index); }
			operator signal& () { return ptr->operator signal & (); }
			operator const signal& () const { return ptr->operator const signal & (); }
			explicit operator double() const { return ptr->operator double(); }
			bool finished() const { return ptr->finished(); }
			signal& operator++(int) { return ptr->operator++(1); }
			//signal& operator=(const signal& in) { return ptr->operator=(in); }
			//signal& operator+=(const signal& in) { return ptr->operator+=(in); }
			//signal& operator*=(const signal& in) { return ptr->operator*=(in); }
			
			variable::buffer& operator=(const klang::buffer& in) {
				resize(in.size); *ptr = in; return *this;
			}

			float* data() { return ptr->data(); }
			const float* data() const { return ptr->data(); }
		};
	}

	struct Graph;
	struct GraphPtr;

	/// Templates supporting common audio functionality
	namespace Generic {

		/// Audio input object
		template<typename SIGNAL>
		struct Input {
			virtual ~Input() {}
			SIGNAL in = { 0.f };

			// retrieve current input
			virtual const SIGNAL& input() const { return in; }

			// feedback input (include pre-processing, if any)
			virtual void operator<<(const SIGNAL& source) { in = source; this->input(); }
			virtual void input(const SIGNAL& source) { in = source; this->input(); }

		protected:
			// preprocess input (default: none)
			virtual void input() {}
		};

		/// Audio output object
		template<typename SIGNAL>
		struct Output {
			SIGNAL out = { 0.f };

			// returns previous output (without processing)
			virtual const SIGNAL& output() const { return out; }

			// pass output to destination (with processing)
			template<typename TYPE>
			TYPE& operator>>(TYPE& destination) { this->process(); return destination = out; }

			// returns output (with processing)
			virtual operator const SIGNAL& () { this->process(); return out; } // return processed output
			virtual operator const SIGNAL& () const { return out; } // return last output

			// arithmetic operations produce copies
			template<typename TYPE> SIGNAL operator+(TYPE& other) { this->process(); return out + SIGNAL(other); }
			template<typename TYPE> SIGNAL operator*(TYPE& other) { this->process(); return out * SIGNAL(other); }
			template<typename TYPE> SIGNAL operator-(TYPE& other) { this->process(); return out - SIGNAL(other); }
			template<typename TYPE> SIGNAL operator/(TYPE& other) { this->process(); return out / SIGNAL(other); }

			void reset() { out = 0; }

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

		/// Signal generator object (output only)
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
			operator const SIGNAL& () override { this->process(); return out; } // return processed output		
			operator const SIGNAL& () const override { return out; } // return last output		

		protected:
			// overrideable parameter setting (up to 8 parameters)
			/// @cond
			virtual void set(param) {};
			virtual void set(relative) {}; // relative alternative
			virtual void set(param, param) {};
			virtual void set(param, relative) {}; // relative alternative
			virtual void set(param, param, param) {};
			virtual void set(param, param, relative) {}; // relative alternative
			virtual void set(param, param, param, param) {};
			virtual void set(param, param, param, relative) {}; // relative alternative
			virtual void set(param, param, param, param, param) {};
			virtual void set(param, param, param, param, relative) {}; // relative alternative
			virtual void set(param, param, param, param, param, param) {};
			virtual void set(param, param, param, param, param, relative) {}; // relative alternative
			virtual void set(param, param, param, param, param, param, param) {};
			virtual void set(param, param, param, param, param, param, relative) {}; // relative alternative
			virtual void set(param, param, param, param, param, param, param, param) {};
			virtual void set(param, param, param, param, param, param, param, relative) {}; // relative alternative
			/// @endcond
		};

		/// Signal modifier object (input-output)
		template<typename SIGNAL>
		struct Modifier : public Input<SIGNAL>, public Output<SIGNAL> {
			using Input<SIGNAL>::in;
			using Output<SIGNAL>::out;

			virtual ~Modifier() {}

			// signal processing (input-output)
			operator const SIGNAL& () override { this->process(); return out; } // return processed output
			operator const SIGNAL& () const override { return out; } // return last output

			using Input<SIGNAL>::input;
			virtual void process() override { out = in; } // default to pass-through

			// inline parameter(s) support
			template<typename... params>
			Modifier<SIGNAL>& operator()(params... p) {
				set(p...); return *this;
			}
		protected:
			// overrideable parameter setting (up to 8 parameters)
			/// @cond
			virtual void set(param) {};
			virtual void set(relative) {}; // relative alternative
			virtual void set(param, param) {};
			virtual void set(param, relative) {}; // relative alternative
			virtual void set(param, param, param) {};
			virtual void set(param, param, relative) {}; // relative alternative
			virtual void set(param, param, param, param) {};
			virtual void set(param, param, param, relative) {}; // relative alternative
			virtual void set(param, param, param, param, param) {};
			virtual void set(param, param, param, param, relative) {}; // relative alternative
			virtual void set(param, param, param, param, param, param) {};
			virtual void set(param, param, param, param, param, relative) {}; // relative alternative
			virtual void set(param, param, param, param, param, param, param) {};
			virtual void set(param, param, param, param, param, param, relative) {}; // relative alternative
			virtual void set(param, param, param, param, param, param, param, param) {};
			virtual void set(param, param, param, param, param, param, param, relative) {}; // relative alternative
			/// @endcond
		};

		/// Applies a function to a signal (input-output)
		template<typename SIGNAL, typename... Args>
		struct Function : public Generic::Modifier<SIGNAL> {
			virtual ~Function() {}

			using Modifier<SIGNAL>::in;
			using Modifier<SIGNAL>::out;

			operator SIGNAL() {
				if (function)
					return out = evaluate();
				this->process();
				return out;
			};
			operator param() {
				if (function)
					return out = evaluate();
				return out;
			}
			operator float() {
				if (function)
					return out = evaluate();
				return out;
			}

			/// @cond
			// Helper to combine hash values
			template <typename T>
			inline void hash_combine(std::size_t& seed, const T& value) const {
				std::hash<T> hasher;
				seed ^= hasher(value) + 0x9e3779b9 + (seed << 6) + (seed >> 2);
			}

			// Recursive function to hash each element of a tuple
			template <typename Tuple, std::size_t Index = 0>
			std::size_t hash_tuple(const Tuple& tuple) const {
				if constexpr (Index < std::tuple_size<Tuple>::value) {
					std::size_t seed = hash_tuple<Tuple, Index + 1>(tuple);
					hash_combine(seed, std::get<Index>(tuple));
					return seed;
				}
				else {
					return 0;
				}
			}

			operator void* () { return this; }
			uint64_t hash() const { return hash_tuple(tail(inputs)); }
			/// @endcond

			std::function<float(Args...)> function;

			static constexpr unsigned int ARGS = sizeof...(Args);
			unsigned int args() const { return ARGS; }

			std::tuple<Args...> inputs;

			// Helper function to extract the tail of a tuple
			template <typename First, typename... Rest>
			std::tuple<Rest...> tail(const std::tuple<First, Rest...>& t) const {
				return std::apply([](const First&, const Rest&... rest) { return std::make_tuple(rest...); }, t);
			}

			template<typename FunctionPtr>
			Function() : function(nullptr) {}

			template<typename FunctionPtr>
			Function(FunctionPtr function)
				: function(std::forward<FunctionPtr>(function)) {
			}

			template<typename FunctionPtr, typename... OtherArgs>
			Function(FunctionPtr function, OtherArgs... args)
				: function(std::forward<FunctionPtr>(function)) {
				with(args...);
			}

			void input() override {
				std::get<0>(inputs) = in.value;
			}

			// get the first argument
			template <typename First, typename... Rest>
			First& first(First& first, Rest...) { return first; }

			// get the first argument
			template <typename First, typename... Rest>
			const First& first(const First& first, Rest...) const { return first; }

			// Configure callable function's arguments
			template<typename... FuncArgs>
			Function<SIGNAL, Args...>& operator()(const FuncArgs&... args) {

				//// single argument supplied, more than one expected -> use cached inputs or use as input?
	//            if constexpr (ARGS > 1 && sizeof...(FuncArgs) == 1){
	//                //in = first(args...);
	//                std::get<1>(inputs) = in.value;
	//                return *this;
	//            } else

				// Calling: full set of arguments supplied (overwrite live input)
				if constexpr (ARGS == sizeof...(FuncArgs)) {
					in = first(args...);
					inputs = std::tuple<Args...>(args...);
					return *this;

					// Streaming: all but first argument supplied (use live input as x)
				}
				else if constexpr (sizeof...(FuncArgs) == (ARGS - 1)) {
					inputs = std::tuple<Args...>(in.value, args...);
					return *this;

					// Invalid: insufficents arguments
				}
				else {
					static_assert(sizeof...(FuncArgs) < (ARGS - 1), "Insufficient arguments: can only omit first argument");
					//in = first(args...);
					//std::get<0>(inputs) = in.value;
					return *this;
				}
			}

			// Configure and call function
			template<typename... FuncArgs>
			const float operator()(const FuncArgs&... args) const {
				signal in = this->in;
				std::tuple<Args...> inputs = this->inputs;

				// configure inputs
				if constexpr (ARGS > 1 && sizeof...(FuncArgs) == 1) {
					in = first(args...);
					std::get<0>(inputs) = in.value;
				}
				else if constexpr (ARGS == sizeof...(FuncArgs)) {
					in = first(args...);
					inputs = std::tuple<Args...>(args...);
				}
				else if constexpr (sizeof...(FuncArgs) == (ARGS - 1)) {
					inputs = std::tuple<Args...>(in.value, args...);
				}
				else {
					in = first(args...);
					std::get<0>(inputs) = in.value;
				}

				// return outputs
				if (function) {
					if constexpr (ARGS > 1)
						return std::apply(function, inputs);
					else
						return function(in);
				}
				else {
					// NOT YET IMPLEMENTED (needs non-const status)
					return out;
				}
			}

			template<typename... FuncArgs>
			Function<SIGNAL, Args...>& with(FuncArgs... args) {
				static_assert(sizeof...(FuncArgs) == (ARGS - 1), "with() arguments must have all but first argument.");
				inputs = std::tuple<Args...>(in.value, args...);

				return *this;
			}

			signal evaluate() const {
				if (!function)
					return 0.f;
				if constexpr (ARGS > 1)
					return std::apply(function, inputs);
				else
					return (signal)function(in);
			}

			virtual void process() override {
				out = evaluate();
			}

			klang::Graph& operator>>(klang::Graph& graph);
			klang::GraphPtr& operator>>(klang::GraphPtr& graph);

			using Generic::Output<SIGNAL>::operator+;
			using Generic::Output<SIGNAL>::operator-;
			using Generic::Output<SIGNAL>::operator*;
			using Generic::Output<SIGNAL>::operator/;
		};

		// deduction guide for functions
		template <typename... Args>
		Function(float(*)(Args...)) -> Function<signal, Args...>;

		template <typename... Args>
		Function() -> Function<signal, Args...>;

		template<typename TYPE, typename SIGNAL, typename... Args> SIGNAL operator+(TYPE& x, Function<SIGNAL, float>& func) { return func + (SIGNAL)x; }
		template<typename TYPE, typename SIGNAL, typename... Args> SIGNAL operator-(TYPE& x, Function<SIGNAL, float>& func) { return -func + (SIGNAL)x; }
		template<typename TYPE, typename SIGNAL, typename... Args> SIGNAL operator*(TYPE& x, Function<SIGNAL, float>& func) { return func * (SIGNAL)x; }
		template<typename TYPE, typename SIGNAL, typename... Args> SIGNAL operator/(TYPE& x, Function<SIGNAL, float>& func) {
			SIGNAL y = (SIGNAL)func;
			return (SIGNAL)x / y;
		}

		/// A line graph plotter
		template<int SIZE>
		struct Graph {
			virtual ~Graph() {};

			size_t capacity() const { return SIZE; }

			struct Series;

			/// A data point
			struct Point {
				double x, y;
				bool valid() const { return !::isnan(x) && !::isinf(x) && !::isnan(y); } // NB: y can be +/- inf

				Series& operator>>(Series& series);
			};

			struct Axis;

			/// Data series
			struct Series : public Array<Point, SIZE + 1>, Input<signal> {
				virtual ~Series() {}

				const void* function = nullptr;
				uint64_t hash = 0;

				using Array = Array<Point, SIZE + 1>;
				using Array::add;
				using Array::count;
				using Array::items;

				void add(double y) {
					add({ (double)Array::size(), y });
				}

				template<typename SIGNAL, typename... Args>
				void plot(const Generic::Function<SIGNAL, Args...>& f, const Axis& x_axis) {
					constexpr int size = SIZE > 1024 ? 1024 : SIZE;

					if (function != (const void*)&f || hash != f.hash()) {
						clear();
						function = (const void*)&f;
						hash = f.hash();
						double x = 0;
						const double dx = x_axis.range() / size;
						for (int i = 0; i <= size; i++) {
							x = x_axis.min + i * dx;
							add({ x, (double)(signal)f(x) });
						}
					}
				}

				template<typename RETURN, typename ARG>
				void plot(RETURN(*f)(ARG), const Axis& x_axis) {
					constexpr int size = SIZE > 1024 ? 1024 : SIZE;

					if (function != (void*)f) {
						clear();
						function = (void*)f;
						hash = 0;
						double x = 0;
						const double dx = x_axis.range() / size;
						for (int i = 0; i <= size; i++) {
							x = x_axis.min + i * dx;
							add({ x, (double)(signal)f((float)x) });
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

				void clear() {
					function = nullptr;
					Array::clear();
				}

				using Input::input;
				void input() override {
					add(in);
				}
			};

			/// Graph axis
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
						if (pt.valid() && !::isinf(pt.*axis)) {
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

			/// Graph axes (x/y)
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

			/// @cond
			bool isActive() const {
				if (data.count)
					return true;
				for (int s = 0; s < GRAPH_SERIES; s++)
					if (data[s].count)
						return true;
				return false;
			}
			/// @endcond

			/// Graph data
			struct Data : public Array<Series, GRAPH_SERIES> {
				using Array<Series, GRAPH_SERIES>::items;

				Series* find(void* function) {
					for (int s = 0; s < GRAPH_SERIES; s++)
						if (this->operator[](s).function == function)
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

			/// Plot the given Function
			template<typename SIGNAL, typename... Args>
			void plot(Generic::Function<SIGNAL, Args...>& function) {
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

			/// Plot the given function of x
			template<typename TYPE>
			void plot(TYPE(*function)(TYPE)) {
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

			/// Plot the given function for x plus additional arguments
			template<typename FUNCTION, typename... VALUES>
			void plot(FUNCTION f, VALUES... values) {
				thread_local static Function fun(f);
				Graph::Series* series = Graph::data.find((void*)fun);
				if (!series)
					series = Graph::data.add();
				if (series) {
					dirty = true;
					if (!axes.x.valid())
						axes.x = { -1, 1 };
					series->plot(fun.with(values...), axes.x);
				}
			}

			/// Add a data point (incrementing x)
			template<typename TYPE>
			void add(TYPE y) { data[0].add(y); dirty = true; }
			/// Add a data point
			void add(const Point pt) { data[0].add(pt); dirty = true; }

			/// Add a data point (incrementing x)
			template<typename TYPE>
			Graph& operator+=(TYPE y) { add(y); return *this; }
			/// Add a data point
			Graph& operator+=(const Point pt) { add(pt); return *this; }

			/// Plot the given function of x
			template<typename TYPE>
			Graph& operator=(TYPE(*function)(TYPE)) {
				plot(function);
				return *this;
			}

			/// Plot the given data points
			Graph& operator=(std::initializer_list<Point> values) {
				clear(); return operator+=(values);
			}

			/// Plot the given data points
			Graph& operator+=(std::initializer_list<Point> values) {
				for (const auto& value : values)
					add(value);
				return *this;
			}

			/// Plot the given function for x plus additional arguments
			template<typename... Args> Graph& operator<<(Function<Args...>& function) {
				plot(function.with());
				return *this;
			}

			template<typename TYPE> Graph& operator<<(TYPE& in) { add(in); return *this; } // with processing
			template<typename TYPE> Graph& operator<<(const TYPE& in) { add(in); return *this; } // without/after processing

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

		template<int SIZE>
		inline typename Graph<SIZE>::Series& Graph<SIZE>::Point::operator>>(Graph<SIZE>::Series& series) {
			series.add(*this);
			return series;
		}

		/// Audio oscillator object (output)
		template<typename SIGNAL>
		struct Oscillator : public Generator<SIGNAL> {
		protected:
			Phase increment;				// phase increment (per sample, in seconds or samples)
			Phase position = 0;				// phase position (in radians or wavetable size)
		public:
			Frequency frequency = 1000.f;	// fundamental frequency of oscillator (in Hz)
			Phase offset = 0;				// phase offset (in radians - e.g. for modulation)

			virtual ~Oscillator() {}
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

	/// Audio input object (mono)
	struct Input : Generic::Input<signal> {};
	/// Audio output object (mono)
	struct Output : Generic::Output<signal> {};
	/// Signal generator object (mono)
	struct Generator : Generic::Generator<signal> {};
	/// Signal modifier object (mono input-output)
	struct Modifier : public Generic::Modifier<signal> {};
	/// Audio oscillator object  (mono output)
	struct Oscillator : Generic::Oscillator<signal> {};

	/// @cond
	template <typename TYPE, typename SIGNAL>
	using GeneratorOrModifier = typename std::conditional<std::is_base_of<Generator, TYPE>::value, Generic::Generator<SIGNAL>,
		typename std::conditional<std::is_base_of<Modifier, TYPE>::value, Generic::Modifier<SIGNAL>, void>::type>::type;
	/// @endcond

	/// A parallel bank of multiple audio objects
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

	/// Applies a function to a signal (input-output)
	template<typename... Args>
	struct Function : public Generic::Function<signal, Args...> {

		//static float Identity(float x) { return x; }

		//Function() : Generic::Function<signal, float>([](float x) -> float { return x; }) { }

		Function() : Generic::Function<signal, Args...>(nullptr) {}

		Function(std::function<float(Args...)> function) : Generic::Function<signal, Args...>(function) {}

		using Generic::Function<signal, Args...>::operator>>;
	};

	/// A line graph plotter
	struct Graph : public Generic::Graph<GRAPH_SIZE> {
		using Generic::Graph<GRAPH_SIZE>::operator=;
		using Generic::Graph<GRAPH_SIZE>::plot;

		/// Plot the given function for x plus additional arguments
		template<typename... Args> Graph& operator<<(Function<Args...>& function) {
			plot(function.with());
			return *this;
		}
	};

	/// @cond
	struct GraphPtr {
		std::unique_ptr<Graph> ptr;

		void check() {
			if (ptr.get() == nullptr)
				ptr = std::make_unique<Graph>();
		}

		Graph* operator->() { check(); return (klang::Graph*)ptr.get(); }
		operator Graph* () { check(); return (klang::Graph*)ptr.get(); }
		operator Graph& () { check(); return *(klang::Graph*)ptr.get(); }

		void clear() { check(); ptr->clear(); }
		bool isActive() const { return ptr ? ptr->isActive() : false; }

		Graph& operator()(double min, double max) { check(); return (Graph&)ptr->operator()(min, max); }

		Graph::Series& operator[](int index) { check(); return ptr->operator[](index); }
		const Graph::Series& operator[](int index) const { static const Graph::Series none = Graph::Series(); return ptr ? ptr->operator[](index) : none; }

		Graph& operator()(double x_min, double x_max, double y_min, double y_max) { check(); return (Graph&)ptr->operator()(x_min, x_max, y_min, y_max); }

		operator Graph::Series& () { check(); return ptr->operator Graph::Series & (); }

		template<typename SIGNAL, typename... Args>
		void plot(Generic::Function<SIGNAL, Args...>& function) { check(); ((Graph*)(ptr.get()))->plot(function); }

		template<typename TYPE>
		void plot(TYPE(*function)(TYPE)) { check(); ((Graph*)(ptr.get()))->plot(function); }

		template<typename FUNCTION, typename... VALUES>
		void plot(FUNCTION f, VALUES... values) { check(); ((Graph*)(ptr.get()))->plot(f, values...); }

		template<typename TYPE> void add(TYPE y) { check(); ptr->add(y); }
		void add(const Graph::Point pt) { check(); ptr->add(pt); }

		template<typename TYPE> Graph& operator+=(TYPE y) { check(); return ptr->operator+=(y); }
		Graph& operator+=(const Graph::Point pt) { check(); return (Graph&)ptr->operator+=(pt); }

		template<typename TYPE> Graph& operator=(TYPE(*function)(TYPE)) { check(); return (Graph&)ptr->operator=(function); }

		Graph& operator=(std::initializer_list<Graph::Point> values) { check(); return (Graph&)ptr->operator=(values); }
		Graph& operator+=(std::initializer_list<Graph::Point> values) { check(); return (Graph&)ptr->operator+=(values); }

		template<typename... Args> Graph& operator<<(Generic::Function<Args...>& function) { plot(function.with()); return *this; }
		template<typename TYPE> Graph& operator<<(TYPE& in) { add(in); return *this; } // with processing
		template<typename TYPE> Graph& operator<<(const TYPE& in) { add(in); return *this; } // without/after processing

		const Graph::Axes& getAxes() const { static const Graph::Axes none; return ptr ? ptr->getAxes() : none; }
		void getAxes(Graph::Axes& axes) { check(); ptr->getAxes(axes); }

		const Graph::Data& getData() { check(); return ptr->getData(); }
		bool isDirty() const { return ptr ? ptr->isDirty() : false; }
		void setDirty(bool dirty) { if (ptr) ptr->setDirty(dirty); }

		void truncate(unsigned int count) { check(); ptr->truncate(count); }
	};
	/// @endcond



	// deduction guide for functions
	template <typename... Args>
	Function(float(*)(Args...)) -> Function<Args...>;

	//// deduction guide for functions
	//Function() -> Function<float>;

	THREAD_LOCAL static GraphPtr graph;

	template<typename SIGNAL, typename... Args>
	inline GraphPtr& Generic::Function<SIGNAL, Args...>::operator>>(klang::GraphPtr& graph) {
		graph.plot(*this);
		return graph;
	}

	template<typename TYPE>
	static GraphPtr& operator>>(TYPE(*function)(TYPE), klang::GraphPtr& graph) {
		graph->plot(function);
		return graph;
	}

	template<typename SIGNAL, typename... Args>
	inline Graph& Generic::Function<SIGNAL, Args...>::operator>>(klang::Graph& graph) {
		graph.plot(*this);
		return graph;
	}

	template<typename TYPE>
	static Graph& operator>>(TYPE(*function)(TYPE), Graph& graph) {
		graph.plot(function);
		return graph;
	}

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

	/// Square root function (audio object)
	inline static Function<float> sqrt(SQRTF);
	/// Absolute/rectify function (audio object)
	inline static Function<float> abs(FABS);
	/// Square function (audio object)
	inline static Function<float> sqr([](float x) -> float { return x * x; });
	/// Cube function (audio object)
	inline static Function<float> cube([](float x) -> float { return x * x * x; });

#define sqrt klang::sqrt // avoid conflict with std::sqrt
#define abs klang::abs   // avoid conflict with std::abs

	/// Debug text output
	struct Console : public Text<16384> {
		static std::mutex _lock;
		THREAD_LOCAL static Text<16384> last;
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

	inline THREAD_LOCAL Text<16384> Console::last;

#define PROFILE(func, ...) debug.print("%-16s = %fns\n", #func "(" #__VA_ARGS__ ")", debug.profile(1000, func, __VA_ARGS__));

	/// The Klang debug interface
	struct Debug : Input {

		/// Audio buffer for debug output
		struct Buffer : private buffer {
			using buffer::clear;
			using buffer::rewind;
			using buffer::operator++;
			using buffer::operator signal&;

			Buffer() : buffer(16384) {}

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

		THREAD_LOCAL static Buffer buffer; // support multiple threads/instances

		/// Debug console output
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

		/// @cond
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
		/// @endcond

		void print(const char* format, ...) {
			if (console.length < 10000) {
				THREAD_LOCAL static char string[1024] = { 0 };
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
				THREAD_LOCAL static char string[1024] = { 0 };
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
	inline THREAD_LOCAL Debug::Buffer Debug::buffer; //
	inline std::mutex Console::_lock;

#define FUNCTION(type) (void(*)(type, Result<type>&))[](type x, Result<type>& y)

	/// @cond
	template <typename TYPE>
	struct Result {
		TYPE* y = nullptr;
		int i = 0;
		TYPE sum = 0;

		Result(TYPE* array, int index) : y(&array[index]), i(index) {}

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
	/// @endcond

	/// Lookup table object
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

	/// Audio delay object (fixed size)
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

		void input() override {
			buffer++ = in;
			position++;
			if (position == SIZE) {
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
			// Calculate the read position
			float read = static_cast<float>(position - 1) - delay;
			if (read < 0.f)
				read += SIZE;

			// Separate integer and fractional parts
			const int i = static_cast<int>(read);  // Integer part
			const float fraction = read - i;       // Fractional part

			// Use modulo to get the next index without branching
			const int j = (i + 1) % SIZE;          // Next index in circular buffer

			// Linear interpolation: buffer[i] + fraction * (buffer[j] - buffer[i])
			return buffer[i] + fraction * (buffer[j] - buffer[i]);
		}

		signal lagrange(float delay) const {
			// Calculate the read position
			float read = static_cast<float>(position - 1) - delay;
			if (read < 0.f)
				read += SIZE;

			// Separate integer and fractional parts
			int i = static_cast<int>(read);  // Integer part
			float x = read - i;              // Fractional part (0 ? x < 1)

			// Get four surrounding indices using modulo for circular buffer
			int i0 = (i - 1 + SIZE) % SIZE;
			int i1 = i;                      // Main sample
			int i2 = (i + 1) % SIZE;
			int i3 = (i + 2) % SIZE;

			// Read corresponding samples
			float y0 = buffer[i0];
			float y1 = buffer[i1];
			float y2 = buffer[i2];
			float y3 = buffer[i3];

			// Compute Lagrange interpolation (third-order)
			float c0 = (-x * (x - 1) * (x - 2)) / 6.0f;
			float c1 = ((x + 1) * (x - 1) * (x - 2)) / 2.0f;
			float c2 = (-x * (x + 1) * (x - 2)) / 2.0f;
			float c3 = (x * (x + 1) * (x - 1)) / 6.0f;

			return c0 * y0 + c1 * y1 + c2 * y2 + c3 * y3;
		}


		signal tap() const {
			// Use modulo to get the next index without branching
			const int i = last.position;
			const int j = (i + 1) % SIZE;          // Next index in circular buffer

			// Linear interpolation: buffer[i] + fraction * (buffer[j] - buffer[i])
			return buffer[i] + last.fraction * (buffer[j] - buffer[i]);
		}

		virtual void process() override {
			out = tap();
			last.position = (last.position + 1) % SIZE;
		}

		struct Tap {
			int position;
			float fraction;
		} last;

		virtual void set(param samples) override {
			time = samples < SIZE ? (float)samples : SIZE;

			float read = static_cast<float>(position - 1) - time;
			if (read < 0.f)
				read += SIZE;

			last.position = static_cast<int>(read);  // Integer part
			last.fraction = read - last.position;	 // Fractional part
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

	/// Audio delay object (resizable)
	template<>
	struct Delay<0> : public Modifier {
		using Modifier::in;
		using Modifier::out;

		buffer* buffer;
		float time = 1;
		int position = 0;
		int SIZE = 0;

		Delay() : buffer(new klang::buffer(1, 0)) { clear(); }

		void clear() {
			buffer->clear();
		}

		void resize(int samples) {
			if (samples != SIZE) {
				SIZE = samples;
				klang::buffer* new_buffer = new klang::buffer(SIZE + 1, 0);
				std::swap(buffer, new_buffer);
				delete new_buffer;
			}
		}

		void input() override {
			(*buffer)++ = in;
			position++;
			if (position == SIZE) {
				buffer->rewind();
				position = 0;
			}
		}

		signal tap(int delay) const {
			int read = (position - 1) - delay;
			if (read < 0)
				read += SIZE;
			return (*buffer)[read];
		}

		signal tap(float delay) const {
			// Calculate the read position
			float read = static_cast<float>(position - 1) - delay;
			if (read < 0.f)
				read += SIZE;

			// Separate integer and fractional parts
			const int i = static_cast<int>(read);  // Integer part
			const float fraction = read - i;       // Fractional part

			// Use modulo to get the next index without branching
			const int j = (i + 1) % SIZE;          // Next index in circular buffer

			// Linear interpolation: buffer[i] + fraction * (buffer[j] - buffer[i])
			return (*buffer)[i] + fraction * ((*buffer)[j] - (*buffer)[i]);
		}

		signal tap() const {
			// Use modulo to get the next index without branching
			const int i = last.position;
			const int j = (i + 1) % SIZE;          // Next index in circular buffer

			// Linear interpolation: buffer[i] + fraction * (buffer[j] - buffer[i])
			return (*buffer)[i] + last.fraction * ((*buffer)[j] - (*buffer)[i]);
		}

		virtual void process() override {
			out = tap();
			last.position = (last.position + 1) % SIZE;
		}

		struct Tap {
			int position;
			float fraction;
		} last;

		virtual void set(param samples) override {
			time = samples < SIZE ? (float)samples : SIZE;

			float read = static_cast<float>(position - 1) - time;
			if (read < 0.f)
				read += SIZE;

			last.position = static_cast<int>(read);  // Integer part
			last.fraction = read - last.position;	 // Fractional part
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

	/// Wavetable-based oscillator
	class Wavetable : public Oscillator {
		using Oscillator::set;
	protected:
		buffer buffer;
		const int size;
	public:
		Wavetable(int size = 2048) : buffer(size), size(size) {}

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

	/// Sample-based signal generator
	class Sample : public Oscillator {
		using Oscillator::set;
	protected:
		buffer buffer;
		int size;
	public:
		Sample() : buffer(nullptr, 0), size(0) { }

		signal& operator[](int index) {
			return buffer[index];
		}

		Sample& operator=(const klang::buffer& buffer) {
			size = buffer.size;
			Sample::buffer.attach(buffer, size);
			return *this;
		}

		virtual void set(param frequency) override {
			Oscillator::frequency = frequency;
			increment = 1;// frequency* (44100 / fs);
		}

		virtual void set(param frequency, param phase) override {
			position = phase * float(44100);
			set(frequency);
		}

		virtual void set(relative phase) override {
			offset = phase * float(44100);
		}

		virtual void set(param frequency, relative phase) override {
			set(frequency);
			set(phase);
		}

		void process() override {
			position += { increment, size };
			out = buffer[position + offset];
		}
	};

	/// Envelope object
	class Envelope : public Generator {
		using Generator::set;

	public:

		struct Follower;

		/// Abstract envelope ramp type
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

			virtual ~Ramp() {}

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

		/// Linear envelope ramp (default)
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

		/// Envelope stage
		enum Stage { Sustain, Release, Off };
		/// Envelope mode
		enum Mode { Time, Rate };

		/// Envelope point (x,y)
		struct Point {
			float x, y;

			Point() : x(0), y(0) {}

			template<typename T1, typename T2>
			Point(T1 x, T2 y) : x(float(x)), y(float(y)) {}
		};

		/// @cond
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
		/// @endcond

		/// Envelope loop
		struct Loop {
			Loop(int from = -1, int to = -1) : start(from), end(to) {}

			void set(int from, int to) { start = from; end = to; }
			void reset() { start = end = -1; }

			bool isActive() const { return start != -1 && end != -1; }

			int start;
			int end;
		};

		// Default Envelope (full signal)
		Envelope() : ramp(new Linear()) { set(Points(0.f, 1.f)); }

		// Creates a new envelope from a list of points, e.g. Envelope env = Envelope::Points(0,1)(1,0);
		Envelope(const Points& points) : ramp(new Linear()) { set(points); }

		// Creates a new envelope from a list of points, e.g. Envelope env = { { 0,1 }, { 1,0 } };
		Envelope(std::initializer_list<Point> points) : ramp(new Linear()) { set(points); }

		// Creates a copy of an envelope from another envelope
		Envelope(const Envelope& in) : ramp(new Linear()) { set(in.points); }

		virtual ~Envelope() {}

		// Checks if the envelope is at a specified stage (Sustain, Release, Off)
		bool operator==(Stage stage) const { return Envelope::stage == stage; }
		bool operator!=(Stage stage) const { return Envelope::stage != stage; }

		//operator float() const { return out; }

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
		void set(const Points& point) {
			points.clear();

			const Points* pPoint = &point;
			while (pPoint) {
				points.push_back(*pPoint);
				pPoint = pPoint->next;
			}

			initialise();
		}

		// Converts envelope points based on relative time to absolute time
		void sequence() {
			float time = 0.f;
			for (Point& point : points) {
				const float delta = point.x;
				time += delta + 0.00001f;
				point.x = time;
			}
			initialise();
		}

		// Sets an envelope loop between two points
		void setLoop(int startPoint, int endPoint) {
			if (startPoint >= 0 && endPoint < points.size())
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
		void resetLoop() {
			loop.reset();
			if (stage == Sustain && (point + 1) < points.size())
				setTarget(points[point + 1], points[point].x);
		}

		// Sets the current stage of the envelope
		void setStage(Stage stage) { this->stage = stage; }

		// Returns the current stage of the envelope
		const Stage getStage() const { return stage; }

		// Returns the total length of the envelope (ignoring loops)
		float getLength() const { return points.size() ? points[points.size() - 1].x : 0.f; }

		// Trigger the release of the envelope
		virtual void release(float time, float level = 0.f) {
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
		void initialise() {
			point = 0;
			timeInc = 1.0f / fs;
			loop.reset();
			stage = Sustain;
			if (points.size()) {
				out = points[0].y;
				ramp->setValue(points[0].y);
				if (points.size() > 1)
					setTarget(points[1], points[0].x);
			}
			else {
				out = 1.0f;
				ramp->setValue(1.0f);
			}
		}

		// Scales the envelope duration to specified length 
		void resize(float length) {
			const float old_length = getLength();
			if (old_length == 0.0)
				return;

			const float multiplier = length / (fs * old_length);
			std::vector<Point>::iterator point = points.begin();
			while (point != points.end()) {
				point->x *= multiplier;
				point++;
			}

			initialise();
		}

		// Set the current envelope target
		void setTarget(const Point& point, float time = 0.0) {
			(this->*setTargetFunction)(point, time);
		}

		// Returns the output of the envelope and advances the envelope (for backwards compatibility)
		signal& operator++(int) {
			this->process();
			return out;
		}

		void process() {
			out = (*ramp)++;

			switch (stage) {
			case Sustain:
				time += timeInc;
				if (!ramp->isActive()) { // envelop segment end reached
					if (loop.isActive() && (point + 1) >= loop.end) {
						point = loop.start;
						ramp->setValue(points[point].y);
						if (loop.start != loop.end)
							setTarget(points[point + 1], points[point].x);
					}
					else if ((point + 1) < points.size()) {
						if (mode() == Rate || time >= points[point + 1].x) { // reached target point
							point++;
							ramp->setValue(points[point].y); // make sure exact value is set

							if ((point + 1) < points.size()) // new target point?
								setTarget(points[point + 1], points[point].x);
						}
					}
					else {
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
			ramp->setRate(abs(point.y - ramp->out) / ((point.x - time) * fs));
		}

		void setTargetRate(const Point& point, float rate = 0.0) {
			this->time = 0;
			if (point.x == 0) {
				ramp->setValue(point.y);
			}
			else {
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

	/// Attack-Decay-Sustain-Release Envelope
	struct ADSR : public Envelope {
	private:
		using Envelope::Follower;
	public:
		param A, D, S, R;

		enum Mode { Time, Rate } mode = Time;

		ADSR() { set(0.5, 0.5, 1, 0.5); }

		void set(param attack, param decay, param sustain, param release) override {
			A = attack;
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

	/// FM operator
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

		Operator& operator*(signal amp) {
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

	/// Base class for UI / MIDI controll
	struct Controller {
	protected:
		virtual event control(int index, float value) {};
		virtual event preset(int index) {};
		virtual event midi(int status, int byte1, int byte2) {};
	public:
		virtual void onControl(int index, float value) { control(index, value); };
		virtual void onPreset(int index) { preset(index); };
		virtual void onMIDI(int status, int byte1, int byte2) { midi(status, byte1, byte2); }
	};

	/// Base class for mini-plugin
	struct Plugin : public Controller {
		virtual ~Plugin() {}

		Controls controls;
		Presets presets;
	};

	/// Effect mini-plugin (mono)
	struct Effect : public Plugin, public Modifier {
		virtual ~Effect() {}

		virtual void prepare() {};
		virtual void process() { out = in; }
		virtual void process(buffer buffer) {
			this->prepare();
			while (!buffer.finished()) {
				input(buffer);
				this->process();
				buffer++ = out;
				debug.buffer++;
			}
		}
	};

	/// Base class for synthesiser notes
	template<class SYNTH>
	class NoteBase : public Controller {
		SYNTH* synth;

		class Controls {
			klang::Controls* controls = nullptr;
		public:
			Controls& operator=(klang::Controls& controls) { Controls::controls = &controls; return *this; }
			//signal& operator[](int index) { return controls->operator[](index).operator signal & (); }
			//const signal& operator[](int index) const { return controls->operator[](index).operator const signal & (); }
			Control& operator[](int index) { return controls->operator[](index); }
			const Control& operator[](int index) const { return controls->operator[](index); }
			unsigned int size() { return controls ? controls->size() : 0; }
		};

	protected:
		virtual event on(Pitch p, Velocity v) {}
		virtual event off(Velocity v = 0) { stage = Off; }

		//SYNTH* getSynth() { return synth; }
		auto getSynth() { return static_cast<std::remove_pointer_t<decltype(synth)>*>(synth); }
	public:
		Pitch pitch;
		Velocity velocity;
		Controls controls;

		NoteBase() : synth(nullptr) {}
		virtual ~NoteBase() {}

		void attach(SYNTH* synth) {
			NoteBase::synth = synth;
			controls = synth->controls;
			init();
		}

		virtual void init() {}

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

		virtual void controlChange(int controller, int value) { midi(0xB0, controller, value); };
	};

	struct Synth;

	/// Synthesiser note (mono)
	struct Note : public NoteBase<Synth>, public Generator {
		virtual void prepare() {}
		virtual void process() override = 0;
		virtual bool process(buffer buffer) {
			this->prepare();
			while (!buffer.finished()) {
				this->process();
				buffer++ = out;
				debug.buffer++;
			}
			return !finished();
		}
		virtual bool process(buffer* buffer) {
			return this->process(buffer[0]);
		}
	};

	/// Synthesiser note array
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

		int assign() {
			// favour unused voices
			for (unsigned int i = 0; i < count; i++) {
				if (items[i]->stage == NOTE::Off) {
					noteStart[i] = noteOns++;
					return i;
				}
			}

			// no free notes => steal oldest released note?
			int oldest = -1;
			unsigned int oldest_start = 0;
			for (unsigned int i = 0; i < count; i++) {
				if (items[i]->stage == NOTE::Release) {
					if (oldest == -1 || noteStart[i] < oldest_start) {
						oldest = i;
						oldest_start = noteStart[i];
					}
				}
			}
			if (oldest != -1) {
				noteStart[oldest] = noteOns++;
				return oldest;
			}

			// no available released notes => steal oldest playing note
			oldest = -1;
			oldest_start = 0;
			for (unsigned int i = 0; i < count; i++) {
				if (oldest == -1 || noteStart[i] < oldest_start) {
					oldest = i;
					oldest_start = noteStart[i];
				}
			}
			noteStart[oldest] = noteOns++;
			return oldest;
		}
	};

	/// Synthesiser object (mono)
	struct Synth : public Effect {
		typedef Note Note;

		Notes<Synth, Note> notes;

		Synth() : notes(this) {}
		virtual ~Synth() {}

		//virtual void presetLoaded(int preset) { }
		//virtual void optionChanged(int param, int item) { }
		//virtual void buttonPressed(int param) { };

		int indexOf(Note* note) const {
			int index = 0;
			for (const auto* n : notes.items) {
				if (note == n)
					return index;
				index++;
			}
			return -1; // not found
		}

		// pass to synth and notes
		virtual event onControl(int index, float value) override {
			control(index, value);
			for (unsigned int n = 0; n < notes.count; n++)
				if (notes[n]->stage != Note::Off)
					notes[n]->onControl(index, value);
		};

		// pass to synth and notes
		virtual event onMIDI(int status, int byte1, int byte2) override {
			onMIDI(status, byte1, byte2);
			for (unsigned int n = 0; n < notes.count; n++)
				if (notes[n]->stage != Note::Off)
					notes[n]->onMIDI(status, byte1, byte2);
		}

		// pass to synth and notes
		virtual event onPreset(int index) override {
			preset(index);
			for (unsigned int n = 0; n < notes.count; n++)
				if (notes[n]->stage != Note::Off)
					notes[n]->onPreset(index);
		};

		// assign and start note
		virtual event noteOn(int pitch, float velocity) {
			const int n = notes.assign();
			if (n != -1)
				notes[n]->start((float)pitch, velocity);
		}

		// trigger note off (release)
		virtual event noteOff(int pitch, float velocity) {
			for (unsigned int n = 0; n < notes.count; n++)
				if (notes[n]->pitch == pitch && notes[n]->stage == Note::Sustain)
					notes[n]->release(velocity);
		}

		// post processing (see Effect::process)
		virtual void process() override { out = in; }
		virtual void process(buffer buffer) override { Effect::process(buffer); }

		virtual void process(float* buffer, int length, float* parameters = nullptr) {
			klang::buffer mono(buffer, length);

			// sync parameters
			if (parameters) {
				for (unsigned int c = 0; c < controls.size(); c++)
					controls[c].set(parameters[c]);
			}

			// generate note audio
			for (unsigned int n = 0; n < notes.count; n++) {
				Note* note = notes[n];
				if (note->stage != Note::Off) {
					if (!note->process(mono))
						note->stop();
				}
			}

			// apply post processing
			this->process(mono);

			// sync update (changed by process())
			if (parameters) {
				for (unsigned int c = 0; c < controls.size(); c++)
					parameters[c] = controls[c].value;
			}
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

	/// Objects supporting stereo audio functionality.
	namespace Stereo {
		/// Stereo audio signal
		typedef signals<2> signal;

		/// Stereo sample frame
		struct frame {
			mono::signal& l;
			mono::signal& r;

			frame(mono::signal& left, mono::signal& right) : l(left), r(right) {}
			frame(signal& signal) : l(signal.l), r(signal.r) {}

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

		/// Audio input object (stereo)
		struct Input : Generic::Input<signal> { virtual ~Input() {} };
		/// Audio output object (stereo)
		struct Output : Generic::Output<signal> { virtual ~Output() {} };
		/// Signal generator object (stereo output)
		struct Generator : Generic::Generator<signal> { virtual ~Generator() {} };
		/// Signal modifier object (stereo, input-output)
		struct Modifier : Generic::Modifier<signal> { virtual ~Modifier() {} };
		/// Audio oscillator object (stereo, output)
		struct Oscillator : Generic::Oscillator<signal> { virtual ~Oscillator() {} };

		//inline Modifier& operator>>(signal input, Modifier& modifier) {
		//	modifier << input;
		//	return modifier;
		//}

		/// Stereo audio buffer
		// interleaved access to non-interleaved stereo buffers
		struct buffer {
			typedef Stereo::signal signal;
			mono::buffer& left, right;

			buffer(const buffer& buffer) : left(buffer.left), right(buffer.right) { rewind(); }
			buffer(mono::buffer& left, mono::buffer& right) : left(left), right(right) { rewind(); }

			operator const signal() const { return { left, right }; }
			operator frame () { return { left, right }; }
			bool finished() const { return left.finished() && right.finished(); }
			frame operator++(int) { return { left++, right++ }; }

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

			buffer& operator=(const frame& in) { left = in.l;  right = in.r;		return *this; }
			buffer& operator+=(const frame& in) { left += in.l; right += in.r;	return *this; }
			buffer& operator*=(const frame& in) { left *= in.l; right *= in.r;	return *this; }

			buffer& operator=(const mono::signal in) { left = in;  right = in;	return *this; }
			buffer& operator+=(const mono::signal in) { left += in; right += in; return *this; }
			buffer& operator*=(const mono::signal in) { left *= in; right *= in; return *this; }

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

		/// Stereo audio object adapter
		template<class TYPE>
		struct Bank : klang::Bank<TYPE, 2> {};

		/// Audio delay object (stereo)
		template<int SIZE>
		struct Delay : Bank<klang::Delay<SIZE>> {
			using Bank<klang::Delay<SIZE>>::items;
			using Bank<klang::Delay<SIZE>>::in;
			using Bank<klang::Delay<SIZE>>::out;

			void clear() {
				items[0].clear();
				items[1].clear();
			}

			klang::Delay<SIZE>& l, & r;
			Delay<SIZE>() : l(items[0]), r(items[1]) {}

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

		/// Stereo effect mini-plugin
		struct Effect : public Plugin, public Modifier {
			virtual ~Effect() {}

			virtual void prepare() {};
			virtual void process() { out = in; };
			virtual void process(Stereo::buffer buffer) {
				this->prepare();
				while (!buffer.finished()) {
					input(buffer);
					this->process();
					buffer++ = out;
					debug.buffer++;
				}
			}
		};

		struct Synth;

		/// Base class for stereo synthesiser note
		struct Note : public NoteBase<Synth>, public Generator {
			//virtual signal output() { return 0; };

			virtual void prepare() {}
			virtual void process() override = 0;
			virtual bool process(Stereo::buffer buffer) {
				this->prepare();
				while (!buffer.finished()) {
					this->process();
					buffer++ += out;
				}
				return !finished();
			}
			virtual bool process(mono::buffer* buffers) {
				buffer buffer = { buffers[0], buffers[1] };
				return this->process(buffer);
			}
		};

		namespace Mono {
			struct Note : public Stereo::Note, public klang::Mono::Generator {
				using klang::Mono::Generator::out;

				virtual void prepare() override {}
				virtual void process() override = 0;
				virtual bool process(Stereo::buffer buffer) override {
					this->prepare();
					while (!buffer.finished()) {
						this->process();
						buffer.left += out;
						buffer.right += out;
						buffer++;
					}
					return !finished();
				}
			};
		}

		/// Synthesier mini-plugin (stereo)
		struct Synth : public Effect {
			typedef Stereo::Note Note;

			struct Mono { typedef Stereo::Mono::Note Note; };

			/// Synthesiser note array (stereo)
			struct Notes : klang::Notes<Synth, Note> {
				using klang::Notes<Synth, Note>::Notes;
			} notes;

			Synth() : notes(this) {}
			virtual ~Synth() {}

			//virtual void presetLoaded(int preset) { }
			//virtual void optionChanged(int param, int item) { }
			//virtual void buttonPressed(int param) { };

			int indexOf(Note* note) const {
				int index = 0;
				for (const auto* n : notes.items) {
					if (note == n)
						return index;
					index++;
				}
				return -1; // not found
			}

			// pass to synth and notes
			virtual event onControl(int index, float value) override {
				control(index, value);
				for (unsigned int n = 0; n < notes.count; n++)
					if (notes[n]->stage != Note::Off)
						notes[n]->onControl(index, value);
			};

			// pass to synth and notes
			virtual event onMIDI(int status, int byte1, int byte2) override {
				onMIDI(status, byte1, byte2);
				for (unsigned int n = 0; n < notes.count; n++)
					if (notes[n]->stage != Note::Off)
						notes[n]->onMIDI(status, byte1, byte2);
			}

			// pass to synth and notes
			virtual event onPreset(int index) override {
				preset(index);
				for (unsigned int n = 0; n < notes.count; n++)
					if (notes[n]->stage != Note::Off)
						notes[n]->onPreset(index);
			};

			// assign and start note
			virtual event noteOn(int pitch, float velocity) {
				const int n = notes.assign();
				if (n != -1)
					notes[n]->start((float)pitch, velocity);
			}

			// trigger note off (release)
			virtual event noteOff(int pitch, float velocity) {
				for (unsigned int n = 0; n < notes.count; n++)
					if (notes[n]->pitch == pitch && notes[n]->stage == Note::Sustain)
						notes[n]->release(velocity);
			}

			// post processing (see Effect::process)
			virtual void process() override { out = in; }
			virtual void process(buffer buffer) override { Effect::process(buffer); }

			virtual void process(float** buffers, int length, float* parameters = nullptr) {
				klang::buffer left(buffers[0], length);
				klang::buffer right(buffers[1], length);
				klang::Stereo::buffer buffer(left, right);

				// sync parameters
				if (parameters) {
					for (unsigned int c = 0; c < controls.size(); c++)
						controls[c].set(parameters[c]);
				}

				// generate note audio
				for (unsigned int n = 0; n < notes.count; n++) {
					Note* note = notes[n];
					if (note->stage != Note::Off) {
						if (!note->process(buffer))
							note->stop();
					}
				}

				// apply post processing
				this->process(buffer);

				// sync update (changed by process())
				if (parameters) {
					for (unsigned int c = 0; c < controls.size(); c++)
						parameters[c] = controls[c].value;
				}
			}
		};
	}

	/// @cond
	namespace stereo {
		using namespace Stereo;
	}
	/// @endcond

	/// Feed audio source to destination (with source processing)
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

	/// Feed audio source to destination (no source processing)
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

	/// Common audio generators / oscillators.
	namespace Generators {
		using namespace klang;

		/// Simple oscillators
		namespace Basic {
			/// Sine wave oscillator
			struct Sine : public Oscillator {
				void process() {
					out = sin(position + offset);
					position += increment;
				}
			};

			/// Saw wave oscillator (aliased)
			struct Saw : public Oscillator {
				void process() {
					out = position * pi.inv - 1.f;
					position += increment;
				}
			};

			/// Triangle wave oscillator (aliased)
			struct Triangle : public Oscillator {
				void process() {
					out = abs(2.f * position * pi.inv - 2) - 1.f;
					position += increment;
				}
			};

			/// Square wave oscillator (aliased)
			struct Square : public Oscillator {
				void process() {
					out = position > pi ? 1.f : -1.f;
					position += increment;
				}
			};

			/// Pulse wave oscillator (aliased)
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

			/// White noise generator
			struct Noise : public Generator {
				void process() {
					out = rand() * 2.f / (const float)RAND_MAX - 1.f;
				}
			};
		};

		/// Performance-optimised oscillators
		namespace Fast {
			constexpr float twoPi = float(2.0 * 3.1415926535897932384626433832795);
			constexpr float twoPiInv = float(1.0 / twoPi);

			/// Phase increment (optimised)
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

			/// Oscillator phase (optimised)
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

			/// sin approximation [-pi/2, pi/2] using odd minimax polynomial (Robin Green)
			inline static float polysin(float x) {
				const float x2 = x * x;
				return (((-0.00018542f * x2 + 0.0083143f) * x2 - 0.16666f) * x2 + 1.0f) * x;
			}

			/// fast sine (based on V2/Farbrausch; using polysin)
			inline static float fastsin(float x)
			{
				// Range reduction to [0, 2pi]
				//x = fmodf(x, twoPi);
				//if (x < 0)			// support -ve phase (e.g. for FM)
				//	x += twoPi;		//
				x = fast_mod2pi(x);

				// Range reduction to [-pi/2, pi/2]
				if (x > 3 / 2.f * pi)	// 3/2pi ... 2pi
					x -= twoPi;	// (= translated)
				else if (x > pi / 2)	// pi/2 ... 3pi/2
					x = pi - x;		// (= mirrored)

				return polysin(x);
			}

			/// fast sine (using polysin and integer math)
			inline static float fastsinp(unsigned int p)
			{
				// Range reduction to [0, 2pi]
				//x = fmodf(x, twoPi);
				//if (x < 0)			// support -ve phase (e.g. for FM)
				//	x += twoPi;		//
				float x = fast_modp(p);

				// Range reduction to [-pi/2, pi/2]
				if (x > 3.f / 2.f * pi)	// 3/2pi ... 2pi
					x -= twoPi;			// (= translated)
				else if (x > pi / 2.f)	// pi/2 ... 3pi/2
					x = pi - x;			// (= mirrored)

				return polysin(x);
			}

			/// Sine wave oscillator (band-limited, optimised)
			struct Sine : public Oscillator {
				void reset() override {
					Sine::position = Oscillator::position = 0;
					Sine::offset = Oscillator::offset = 0;
					set(Oscillator::frequency, 0.f);
				}

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

			/// Oscillator State Machine
			struct OSM {
				using Waveform = float(OSM::*)();
				const Waveform waveform;

				OSM(Waveform waveform) : waveform(waveform) {}

				enum State { // carry:old_up:new_up
					NewUp = 0b001, NewDown = 0b000,
					OldUp = 0b010, OldDown = 0b000,
					Carry = 0b100, NoCarry = 0b000,

					Down = OldDown | NewDown | NoCarry,
					UpDown = OldUp | NewDown | NoCarry,
					Up = OldUp | NewUp | NoCarry,
					DownUpDown = OldDown | NewDown | Carry,
					DownUp = OldDown | NewUp | Carry,
					UpDownUp = OldUp | NewUp | Carry
				};

				Fast::Increment increment;
				Fast::Phase offset;
				Fast::Phase duty;
				State state;
				float delta;

				// coeffeficients
				float f, omf, rcpf, rcpf2, col;
				float c1, c2;

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
					state = (State)(((state << 1) | (offset.position < duty.position)) & (NewUp | OldUp));

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

			/// @cond
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
			/// @endcond

			/// Saw wave oscillator (band-limited, optimised)
			struct Saw : public Osm { Saw() : Osm(&OSM::saw, 0.f) {} };
			/// Triangle wave oscillator (band-limited, optimised)
			struct Triangle : public Osm { Triangle() : Osm(&OSM::saw, 1.f) {} };
			/// Square wave oscillator (band-limited, optimised)
			struct Square : public Osm { Square() : Osm(&OSM::pulse, 1.0f) {} };
			/// Pulse wave oscillator (band-limited, optimised)
			struct Pulse : public Osm { Pulse() : Osm(&OSM::pulse, 0.5f) {} };

			/// White noise generator (optimised)
			struct Noise : public Generator {
				static constexpr unsigned int bias = 0b1000011100000000000000000000000;
				/// @cond
				union { unsigned int i; float f; };
				/// @endcond
				void process() {
					i = ((rand() & 0b111111111111111UL) << 1) | bias;
					out = f - 257.f;
				}
			};
		};

		/// Wavetable-based oscillators
		namespace Wavetables {
			/// Sine wave oscillator (wavetable)
			struct Sine : public Wavetable {
				Sine() : Wavetable(Basic::Sine()) {}
			};

			/// Saw wave oscillator (wavetable)
			struct Saw : public Wavetable {
				Saw() : Wavetable(Basic::Saw()) {}
			};
		}
	};

	/// Common audio filters.
	namespace Filters {

		/// Simple DC filter / blocker
		struct DCF : public Modifier {
			float r = 0.995f; // Decay factor (adjustable)
			float z = 0;	  // Filter state (last input)

			void set(float r) { this->r = r; }

			void process() {
				out = in - z + r * out;
				z = in;
			}
		};

		/// IIR filter for specified order
		template<int ORDER>
		struct IIR : public Modifier {
			virtual ~IIR() {}

			float a[ORDER] = { }; // Feedback coefficients (a1, a2, ..., aN)
			float y[ORDER] = { }; // Previous outputs (y[n-1], y[n-2], ..., y[n-N])

			template <typename... Coeffs>
			void set(Coeffs... coeffs) {
				static_assert(sizeof...(coeffs) == ORDER, "Incorrect number of coefficients.");
				_set<0>(coeffs...);
			}

			void process() {
				out = in;
				for (size_t i = 0; i < ORDER; ++i)
					out -= a[i] * y[i];

				for (size_t i = ORDER - 1; i > 0; --i)
					y[i] = y[i - 1];
				y[0] = out;
			}

		protected:
			// Helper function to unpack variadic arguments into a[]
			template <size_t index, typename First, typename... Rest>
			void _set(First first, Rest... rest) {
				a[index] = first;
				if constexpr (index + 1 < ORDER)
					_set<index + 1>(rest...);
			}
		};
		
		// optimised first-order IIR
		template <>
		struct IIR<1> : public Modifier {
			virtual ~IIR() {}

			float a = 1, b = 0; // Feedback coefficients (a1, a2)

			void set(param coeff) {
				a = coeff;
				b = 1.f - a;
			}

			void process() {
				out = in * a + out * b;
			}

			/// Compute the phase offset in seconds at a given frequency
			float phase(float f) const {
				float omega = 2.0f * pi.f * f / fs.f;
				float cos_w = cosf(omega);
				float sin_w = sinf(omega);

				return atan2(b * sin_w, 1.f - b * cos_w) / (2.0f * pi.f * f); // Convert to seconds
			}

			/// Compute the group delay in samples at a given frequency
			float delay(float f) const {
				const float omega = 2.0f * pi.f * f / fs.f;
				const float cos_w = cosf(omega);
				return (1.f - a * a) / (1.f - 2.f * a * cos_w + a * a);
			}
		};

		/// Single-pole (one-pole, one-zero) First Order Filters
		namespace OnePole
		{
			/// Abstract filter class
			struct Filter : Modifier {
				virtual ~Filter() {}

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
						init();
					}
				}

				virtual void init() = 0;

				void process() {
					out = b0 * in + b1 * z + a1 * out + DENORMALISE;
					z = in;
				}
			};

			/// Low-pass filter (LPF)
			struct LPF : Filter {
				virtual ~LPF() {}

				void init() {
					const float exp0 = expf(-f * fs.w);
					b0 = 1 - exp0;
					a1 = exp0;
				}

				void process() {
					out = b0 * in + a1 * out + DENORMALISE;
				}

				/// Compute the phase delay in seconds at a given frequency
				float phase(float f) const {
					float omega = 2.0f * pi.f * f / fs.f;
					float cos_w = cosf(omega);
					float sin_w = sinf(omega);

					float real = b0 - a1 * cos_w;
					float imag = -a1 * sin_w;

					float phaseRadians = atan2f(imag, real);
					return phaseRadians / (2.0f * pi.f * f); // Convert to seconds
				}
			};

			/// High-pass filter (HPF)
			struct HPF : Filter {
				virtual ~HPF() {}

				void init() {
					const float exp0 = expf(-f * fs.w);
					b0 = 0.5f * (1.f + exp0);
					b1 = -b0;
					a1 = exp0;
				}
			};
		};

		/// Transposed Direct Form II Biquadratic Filter
		namespace Biquad {

			/// Abstract filter class
			struct Filter : Modifier
			{
				virtual ~Filter() {}

				float f = 0; 	// cutoff/centre f
				float Q = 0;	// Q (resonance)

				float /*a0 = 1*/ a1 = 0, a2 = 0, b0 = 1, b1 = 0, b2 = 0; // coefficients

				float a = 0;		// alpha
				float cos0 = 1;		// cos(omega)
				float sin0 = 0;		// sin(omega)
				float z[2] = { 0 };	// filter state

				/// Reset filter state
				void reset() {
					f = 0;
					Q = 0;
					b0 = 1;
					a1 = a2 = b1 = b2 = 0;
					a = 0;
					z[0] = z[1] = 0;
				}

				/// Set the filter cutoff (default Q)
				void set(param f) { this->set(f, root2.inv); }

				/// Set the filter cutoff and bandwidth
				void set(param f, relative bw) {
					if (bw > 0)
						this->set(f, param(f / bw));
				}

				/// Set the filter cutoff and Q
				void set(param f, param Q) {
					if (Q < 0) // treat negative Q as bandwidth
						Q = f / -Q;

					if (Filter::f != f || Filter::Q != Q) {
						Filter::f = f;
						Filter::Q = Q;

						const float w = f * fs.w;
						cos0 = cosf(w);
						sin0 = sinf(w);

						if (Q < 0.5) Q = 0.5;
						a = sin0 / (2.f * Q);
						this->init();
					}
				}

				virtual void init() = 0;

				/// Apply the biquad filter (Transposed Direct Form II)
				void process() noexcept {
					const float z0 = z[0];
					const float z1 = z[1];
					const float y = b0 * in + z0;
					z[0] = b1 * in - a1 * y + z1;
					z[1] = b2 * in - a2 * y;
					out = y;
				}

				/// Return the phase offset for the specified frequency (in seconds)
				float phase(float f) const
				{
					float omega = 2.0f * pi.f * f / fs.f;
					float cos_w = std::cos(omega);
					float sin_w = std::sin(omega);

					float real = b0 + b1 * cos_w + b2 * cos(2 * omega);// -(a1 * cos_w + a2 * cos(2 * omega));
					float imag =      b1 * sin_w + b2 * sin(2 * omega);// -(a1 * sin_w + a2 * sin(2 * omega));
					float phaseRadians = std::atan2(-imag, real);

					real = 1 + (a1 * cos_w + a2 * cos(2 * omega));
					imag = (a1 * sin_w + a2 * sin(2 * omega));

					phaseRadians -= std::atan2(-imag, real);
					return phaseRadians / (2.0f * pi.f * f); // Convert to seconds
				}

				/// Return the group delay for the specified frequency (in samples)
				float delay(float f) const
				{
					float omega = 2.0f * pi.f * f / fs.f;
					float cos_w = std::cos(omega);
					float sin_w = std::sin(omega);

					// Compute transfer function components
					float R = b0 + b1 * cos_w + b2 * cos(2 * omega) - (a1 * cos_w + a2 * cos(2 * omega));
					float I =      b1 * sin_w + b2 * sin(2 * omega) - (a1 * sin_w + a2 * sin(2 * omega));

					// Compute phase response
					//float phaseRadians = std::atan2(I, R);

					// Compute the derivative of phase (group delay)
					float dR_dOmega = -b1 * sin_w - 2 * b2 * sin(2 * omega) + a1 * sin_w + 2 * a2 * sin(2 * omega);
					float dI_dOmega =  b1 * cos_w + 2 * b2 * cos(2 * omega) - a1 * cos_w - 2 * a2 * cos(2 * omega);

					return (R * dI_dOmega - I * dR_dOmega) / (R * R + I * I);
				}
			};

			/// Low-pass filter (LPF)
			struct LPF : Filter {
				virtual ~LPF() {}

				void init() override {
					constant a0 = { 1.f + a };
					a1 = a0.inv * (-2.f * cos0);
					a2 = a0.inv * (1.f - a);

					b2 = b0 = a0.inv * (1.f - cos0) * 0.5f;
					b1 = a0.inv * (1.f - cos0);
				}
			};

			typedef LPF HCF; ///< High-cut filter (HCF)
			typedef LPF HRF; ///< High-reject filter (HRF)

			/// High-pass filter (HPF)
			struct HPF : Filter {
				virtual ~HPF() {}

				void init() {
					constant a0 = { 1.f + a };
					a1 = a0.inv * (-2.f * cos0);
					a2 = a0.inv * (1.f - a);

					b2 = b0 = a0.inv * (1.f + cos0) * 0.5f;
					b1 = a0.inv * -(1.f + cos0);
				}
			};

			typedef HPF LCF; ///< Low-cut filter (LCF)
			typedef HPF LRF; ///< Low-reject filter (LRF)

			/// Band-pass filter (BPF)
			struct BPF : Filter {

				enum Gain {
					ConstantSkirtGain,  ///< Constant Skirt Gain
					ConstantPeakGain    ///< Constant Peak Gain
				};

				/// Set the constant gain mode.
				BPF& operator=(Gain gain) {
					init_gain = gain == ConstantSkirtGain ? &BPF::init_skirt : &BPF::init_peak;
					init();
					return *this;
				}

				using Init = void(BPF::*)(void); ///< @internal
				Init init_gain = &BPF::init_peak; ///< @internal
				void init() { (this->*init_gain)(); } ///< @internal

				/// @internal
				void init_skirt() {
					// constant skirt gain
					constant a0 = { 1.f + a };
					a1 = a0.inv * (-2.f * cos0);
					a2 = a0.inv * (1.f - a);

					b0 = a0.inv * sin0 * 0.5f;
					b1 = 0;
					b2 = -b0;
				}

				/// @internal
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

			/// Band-Reject Filter (BRF)
			struct BRF : Filter {
				void init() {
					constant a0 = { 1.f + a };
					b1 = a1 = a0.inv * (-2.f * cos0);
					a2 = a0.inv * (1.f - a);
					b0 = b2 = a0.inv;
				}
			};

			typedef BRF BSF; ///< Band-stop filter (BSF)

			/// All-pass filter (APF)
			struct APF : Filter {
				param r = 1.f;

				/// Set the filter cutoff (default r)
				void set(param f) { this->set(f, 1.f); }

				/// Set the pole frequency and radius (r)
				void set(param f, param r) {
					if (Filter::f != f || a != r) {
						Filter::f = f;
						a = r;

						const float w = f * fs.w;
						cos0 = cosf(w);
						sin0 = sinf(w);
						
						init();
					}
				}

				void init() {
					float omega = 2.0f * pi * f / fs;
					float cos0 = cos(omega);

					b0 = a2 = a * a;
					b1 = a1 = (-2.f * a * cos0);
					b2 = 1.f;
				}
			};
		}
		//		}
	
		/// One-pole Butterworth filter.
		namespace Butterworth {
			/// Low-pass filter (LPF).
			template<int ORDER>
			struct LPF : OnePole::Filter {
				virtual ~LPF() {}
			};

			template<>
			struct LPF<1> : OnePole::Filter {
				virtual ~LPF() {}

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

			template<>
			struct LPF<2> : Biquad::Filter {
				void init() {
					constant a0 = { 1.f + a };
					b0 = a0.inv * ((1.f - cos0) / 2.f);
					b1 = a0.inv * (1.f - cos0);
					b2 = a0.inv * ((1.f - cos0) / 2.f);
					a1 = a0.inv * ( - 2.f * cos0);
					a2 = a0.inv * (1.f - a);
				}
			};
		};
	}

	/// Common audio modifiers.
	namespace Modifiers {
		/// Modal resonator
		struct Modal : public Modifier {
			float a1 = 0, a2 = 0;
			float y1 = 0, y2 = 0;

			float gain = 0.05f;

			// initialise the resonator
			void set(param f, param decay, param gain) {
				set(f, decay);
				this->gain = std::clamp(gain.value * 0.05f, -0.05f, 0.05f);
			}

			void set(param f, param decay) {
				gain = 0.05f;

				constexpr float min_d = 1e-6f;  // Prevents underflow
				constexpr float max_d = 0.9999f;  // Ensures stability
			
				const float w = f.value * fs.w;
				const float d = std::clamp(std::exp(-pi / (decay * fs)), min_d, max_d);

				a1 = 2.f * d * std::clamp(std::cos(w), -0.9999f, 0.9999f);
				a2 = -d * d;

				y2 = 0;
				y1 = 0;
				in = 0;
				out = 0;
			}

			void input() {
				in *= gain;
			}

			void process() {
				out = in + a1 * y1 + a2 * y2;
				y2 = y1;
				y1 = out;
				in = 0;
			}
		};
	};

	/// Envelope follower (Peak / RMS)
	struct Envelope::Follower : Modifier {

		/// Attack / Release IIR Filter (~Butterworth when attack == release)
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

		void process() { (this->*_process)(); }

		void peak() { abs(in) >> ar >> out; }
		void rms() { (in * in) >> ar >> sqrt >> out; }

		/// Window-based envelope follower
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
				sum* window.inv >> ar >> out;
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

	struct File {
#if defined(_MSC_VER)
#define packed __pragma(pack(push,1)) struct __pragma(pack(pop))
#else
#define packed struct __attribute__((packed))
#endif
		struct Format {
			unsigned int channels = 0;
			unsigned int samplerate = 0;
			unsigned int bitrate = 0;
		};

		packed Chunk{
			struct ID {
				char id[4] = { 0 };
				ID() = default;
				ID(const char* id) : id{ id[0], id[1], id[2], id[3] } {}
				bool operator==(const ID& id) const {
					return memcmp(ID::id, id.id, 4) == 0;
				}
				bool operator==(const char* const id) const {
					return memcmp(ID::id, id, 4) == 0;
				}
			} id;
			unsigned int size = 0;

			Chunk() = default;
			Chunk(ID id, unsigned int size = 0) : id(id), size(size) {}
		};

		packed Header : Chunk{
			ID format;

			Header() = default;
			Header(ID id, ID format, unsigned int size = 0) : Chunk(id, size), format(format) {}

			bool isWAV()  const { return id == "RIFF" && format == "WAVE"; }
			bool isAIFF() const { return id == "FORM" && format == "AIFF"; }
		};

		packed WAV{
			Memory memory;

			packed Header : File::Header {
				Header() : File::Header("RIFF", "WAVE") {}
			} *header = nullptr;

			packed Format : Chunk {
				unsigned short AudioFormat;
				unsigned short NumChannels;
				unsigned int   SampleRate;
				unsigned int   ByteRate;
				unsigned short BlockAlign;
				unsigned short BitsPerSample;

				Format() : Chunk("fmt ", sizeof(Format)) {}

				operator File::Format() const { return { NumChannels, SampleRate, BitsPerSample }; }
			} *format = nullptr;

			packed Data : Chunk {
				unsigned char data[1] = { 0 };

				Data() : Chunk("data") {}
			} *data = nullptr;

			// use internal memory
			bool load(const char* path) {
				memory.load(path);
				return load();
			}

			// use external memory
			bool load(Memory& memory) {
				WAV::memory = &memory;
				return load();
			}

			bool load() {
				if (!memory.ptr)
					return false;
				memory.rewind();

				// header
				header = (Header*)memory.ptr;
				if (!header->isWAV())
					return false;
				memory.skip(sizeof(Header));

				// other chunks
				while (!memory.finished()) {
					const Chunk chunk = memory;
					memory.rewind(sizeof(Chunk));
					if (chunk.id == "fmt ")
						format = (Format*)memory.ptr;
					else if (chunk.id == "data") {
						if (memory.ptr + chunk.size > memory.end)
							return false; // corrupt
						data = (Data*)memory.ptr;
					}
					memory.skip(sizeof(Chunk) + chunk.size);
				}
				return true;
			}

			template<typename TYPE, int STEP = 1>
			inline static void decode(float* buffer, const TYPE* data, int length) {
				if constexpr (std::is_floating_point<TYPE>::value) {
					while (length--) {
						*buffer++ = *data; 
						data += STEP;
					}
				} else {
					constexpr unsigned int twoComp = 1U << (sizeof(TYPE) * 8 - 1);
					if constexpr (std::numeric_limits<TYPE>::is_signed) {
						constexpr float scale = 1.f / twoComp;
						while (length--) {
							*buffer++ = static_cast<float>(*data) * scale; 
							data += STEP;
						}
					} else {
						constexpr float scale = 1.f / ((twoComp << 1) - 1);
						while (length--) {
							*buffer++ = (static_cast<float>(*data) - twoComp) * scale; 
							data += STEP;
						}
					}
				}
			}

			bool operator>>(variable::buffer& buffer) {
				if (!header || !format || !data) 
					return false;

				buffer.resize(data->size / format->BlockAlign);
				if (format->AudioFormat == 1) {	// PCM (integer)
					if (format->BitsPerSample == 8)
						decode<unsigned char>(buffer.data(), (unsigned char*)data->data, buffer.size); // 8-bit (unsigned)
					else if(format->BitsPerSample == 16)
						decode<signed short>(buffer.data(), (signed short*)data->data, buffer.size); // 16-bit (signed)
					else if(format->BitsPerSample == 32)
						decode<signed int>(buffer.data(), (signed int*)data->data, buffer.size); // 32-bit (signed)
				} else if (format->AudioFormat == 3) { 
					if(format->BitsPerSample == 32)
						decode<float>(buffer.data(), (float*)data->data, buffer.size); // 32-bit (float)
				}
				return true;
			}
		};

		//struct AIFF {		
		//	struct Binary Extended {
		//		unsigned short exponent;	  // 15-bit exponent + sign bit
		//		unsigned long long mantissa;  // 64-bit mantissa
		//
		//		operator double() const {
		//			uint16_t exp = (exponent >> 8) | (exponent << 8); // Swap bytes (big-endian to host)
		//			const bool sign = exp & 0x8000;		// Extract sign bit
		//			exp &= 0x7FFF;						// Remove sign bit
		//
		//			uint64_t mant = __builtin_bswap64(mantissa); // Convert mantissa (big-endian to host)
		//			if (exp == 0 && mant == 0) return 0.0; // Handle zero case
		//
		//			double fraction = static_cast<double>(mant) / (1ULL << 63); // normalize mantissa (hidden bit)
		//			return (sign ? -fraction : fraction) * std::pow(2.0, exp - 16383);
		//		}
		//	};
		//
		//	struct Binary Header : File::Header {
		//		Header() : File::Header("FORM", "AIFF") {}
		//	};
		//
		//	struct Binary Format : Chunk {
		//		unsigned short numChannels;
		//		unsigned int   numSamplesFrames;
		//		unsigned short sampleSize;
		//		Extended	   sampleRate;
		//
		//		Format() : Chunk("COMM ", sizeof(Format)) { }
		//
		//		operator File::Format() const { return { numChannels, sampleRate, sampleSize }; }
		//	};
		//};
	};

	namespace basic {
		using namespace klang;

		using namespace Generators::Basic;
		using namespace Modifiers;
		using namespace Filters;
		using namespace Filters::Biquad;
	};

	namespace optimised {
		using namespace klang;

		using namespace Generators::Fast;
		using namespace Modifiers;
		using namespace Filters;
		using namespace Filters::Biquad;
	};

	namespace minimal {
		using namespace klang;
	};

	//using namespace optimised;
};

//using namespace klang;