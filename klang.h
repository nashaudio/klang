#pragma once

#include <stdio.h>
#include <stdlib.h>
#include <math.h>
#include <limits>
#include <assert.h>
#include <array>

namespace klang {
	typedef const float constant;
	typedef void event;

	static float fs = 44100.f;
	static constant pi = 3.141592f;
	static float sin(float phase) { return sinf(phase); }
	
	struct Output;

	struct signal {
		//INFO("Signal", 0.f, -1.f, 1.f)

		float value = 0.f;

		signal(float initial = 0.f) : value(initial) { }
		signal(double initial) : value((float)initial) { }
		signal(int value) : value((float)value) { }

		const signal& operator<<(const signal& input) {
			value = input;
			return *this;
		}

		signal& operator>>(signal& destination) const {
			destination = value;
			return destination;
		}

		signal& operator=(const signal& in) { value = in; return *this; };
		signal& operator=(Output& in); // e.g. out = ramp

		operator float() const { return value; }
		operator float&() {		 return value; }
	};

	static signal& operator>>(float input, signal& destination) {
		destination << input;
		return destination;
	}

	struct increment {
		float amount;
		const float size;

		increment(float amount, float size = 2 * pi) : amount(amount), size(size) { }
		increment(float amount, int size) : amount(amount), size((float)size) { }
		increment(float amount, double size) : amount(amount), size((float)size) { }
	};

	enum class Type : short {
		Generic = 0,
		Frequency,
		Pitch,
		Gain,
		Amplitude,
		Phase
	};

	typedef int Conversion;
	constexpr Conversion operator>(Type A, Type B) {
		return (((int)A) << 4) | ((int)B);
	}

	class param : public signal {
		struct ptr : public std::shared_ptr<param> {
			ptr(param* p) : std::shared_ptr<param>(p) { }
			operator const param () { return *get(); }
			virtual ~ptr() { }
		};

	protected:
		virtual Type type() const { return Type::Generic; }

	public:
		using signal::signal;
		param(const signal& in) : signal(in) { }

		param& operator+=(const increment& increment) {
			value += increment.amount;
			if (value >= increment.size)
				value -= increment.size;
			return *this;
		}

		void convert(const param& from, param& to);
		virtual param* convert(Type to);

		ptr operator>(Type type) {
			return ptr(convert(type));
		}
	};

	static param& operator>>(param from, param& to) {
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


	struct Phase : public param {
		Type type() const { return Type::Phase; };

		//INFO("Phase", 0.f, 0.f, 1.0f)
		using param::param;
		Phase(float p = 0.f) : param(p) { };

		param& operator+=(float increment) {
			value += increment;
			if (value > (2.f * pi))
				value -= (2.f * pi);
			return *this;
		}

		param& operator+=(const increment& increment) {
			value += increment.amount;
			if (value > increment.size)
				value -= increment.size;
			return *this;
		}
	};

	struct Pitch : public param {
		Type type() const { return Type::Pitch; };

		//INFO("Pitch", 60.f, 0.f, 127.f)
		using param::param;
		
		Pitch(float p = 60.f) : param(p) { };
		Pitch(int p) : param((float)p) { };

		template<typename TYPE>	Pitch operator+(TYPE in) { return value + in; }
		template<typename TYPE>	Pitch operator-(TYPE in) { return value - in; }
		template<typename TYPE>	Pitch operator*(TYPE in) { return value * in; }
		template<typename TYPE>	Pitch operator/(TYPE in) { return value / in; }
	};

	//static Pitch operator+(const Pitch& a, float b) {		return a.value + b;			}

	struct Frequency : public param { 
		Type type() const { return Type::Frequency; };

		//INFO("Frequency", 1000.f, -FLT_MAX, FLT_MAX)
		using param::param;
		Frequency(float f = 1000.f) : param(f) { };
	};


	template<typename TYPE>
	inline static TYPE operator<(Type type, const TYPE& source) {

	}

	// gain (decibels)
	struct Gain : public param {
		Type type() const { return Type::Gain; };

		//INFO("dB", 0.f, -FLT_MAX, FLT_MAX)
		using param::param;

		Gain(float gain = 0.f) : param(gain) { };
	};

	// amplitude (linear gain)
	struct Amplitude : public param {
		//INFO("Gain", 1.f, -FLT_MAX, FLT_MAX)
		using param::param;

		Amplitude(float a = 1.f) : param(a) { };
		Amplitude(const Gain& db) { 
			value = pow(db / 20.f, 10.f);
		};

		operator Gain() const {
			return 20.f * log(value);
		}
	};

	class Buffer {
	protected:
		float* samples;
		signal* ptr;
		signal* end;
		bool owned = false;
	public:
		const int size;

		Buffer(float* buffer, int size)
		: samples(buffer), size(size) { 
			init();
		}

		Buffer(float* buffer, int size, float initial)
		: samples(buffer), size(size) { 
			init();
			set(initial);
		}

		Buffer(int size, float initial = 0)
		: samples(new float[size]), owned(true), size(size) {
			samples = new float[size];

			init();
			set(initial);
		}

		virtual ~Buffer() {
			if (owned)
				delete[] samples;
		}

		void init() {
			ptr = (signal*)&samples[0];
			end = (signal*)&samples[size];
		}

		void clear() {
			memset(samples, 0, sizeof(float) * size);
		}

		void set(float value = 0) {
			if (value == 0)
				clear();
			else for (int s = 0; s < size; s++)
				samples[s] = value;
		}

		//void resize(int size) {
		//	if (owned && Buffer::size != size) {
		//		float* samples = new float[size];
		//		memcpy(samples, Buffer::samples, std::min(size, Buffer::size));
		//		if(size > Buffer::size)
		//			memset(&samples[Buffer::size], 0, (size - Buffer::size) * sizeof(float));
		//		delete[] Buffer::samples;
		//		Buffer::samples = samples;
		//		Buffer::size = size;
		//		init();
		//	}
		//}

		signal& operator[](int offset) {
			return *(signal*)&samples[offset];
		}

		signal operator[](float offset) {
			return ((const Buffer*)this)->operator[](offset);
		}

		signal operator[](float offset) const {
			const float f = floor(offset);
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

		operator bool() const {
			return ptr != end;
		}

		signal& operator++(int) {
			return *ptr++;
		}

		signal& operator=(const signal& in) {
			return *ptr = in;
		}

		Buffer& operator=(const Buffer& in) {
			assert(size == in.size);
			memcpy(samples, in.samples, size * sizeof(float));
			return *this;
		}
	};

	struct Input {
		signal in = 0.f;
		virtual void input(const signal& source) { in = source; }
		virtual void operator<<(float source) { input(source); }
	};

	struct Output {
		signal out = 0.f;
		virtual signal output() { return 0.f; };
		virtual signal& operator>>(signal& destination) { return destination = out = output(); }

		virtual operator float() { return out = output(); }
		virtual operator signal() { return out = output(); }
	};

	inline signal& signal::operator=(Output& b) { 
		b >> *this;
		return *this;
	}

	struct Generator : public Output { 
		// inline parameter(s) support
		template<typename... params>
		Generator& operator()(params... p) {
			set(p...); return *this;
		}

		virtual void set(param p) { };
		virtual void set(param p1, param p2) { };
		virtual void set(param p1, param p2, param p3) { };
		virtual void set(param p1, param p2, param p3, param p4) { };
	};

	struct Modifier : public Input, public Output { 
		// signal processing (input-output)
		signal output() { return out = output(in); }
		operator signal() { return output(); }
		virtual signal output(const signal& input) = 0;	

		// inline parameter(s) support
		template<typename... params>
		Modifier& operator()(params... p) {
			set(p...); return *this;
		}
		virtual void set(param p) { };
		virtual void set(param p1, param p2) { };
		virtual void set(param p1, param p2, param p3) { };
		virtual void set(param p1, param p2, param p3, param p4) { };
	};

	static Modifier& operator>>(float input, Modifier& modifier) {
		modifier << input;
		return modifier;
	}

	template<int SIZE>
	class Delay : public Modifier {
	protected:
		float buffer[SIZE];
		const int size = SIZE;
		float time = 1;
		int position = 0;
	public:
		Delay() { clear(); }

		void clear() {
			memset(buffer, 0, sizeof(float) * SIZE);
		}

		void operator<<(float input) override {
			buffer[position] = Delay::in = input;
			if(++position == SIZE)
				position = 0;
		}

		signal tap(float delay) const {
			float read = (float)(position - 1) - delay;
			if (read < 0.f)
				read += SIZE;

			float f = floor(read);
			delay = read - f;

			int i = (int)read;
			int j = (i == (SIZE - 1)) ? 0 : (i + 1);
			
			return buffer[i] * (1.f - delay) + buffer[j] * delay;
		}

		signal& operator>>(signal& destination) override {
			return destination = out = tap(time);
		}

		virtual operator signal () override {
			return out = tap(time);
		}

		virtual signal output(const signal& input) override {
			operator<<(input);
			return out = tap(time);
		}

		virtual void set(param delay) override {
			assert(delay <= SIZE);
			Delay::time = delay;
		}
	};

	struct LPF : public Modifier {
		float a, b;

		void set(param coeff) {
			a = coeff;
			b = 1 - a;
		}

		signal output(const signal& input) {
			return out = (a * in) + (b * out);
		}
	};

	struct HPF : public LPF 		{
		signal output(const signal& input) {
			return out = (a * in) - (b * out);
		}
	};

	class Noise : public Generator {
		signal output() {
			return (rand() / (float)RAND_MAX) * 2.f - 1.f;
		}
	};

	class Oscillator : public Generator {
	protected:
		param increment = 1.f;
		Phase offset = 0;
		using Generator::set;
	public:
		Frequency frequency = 1000.f;
		Phase phase = 0;	

		//template<class... TYPES>
		//void set(TYPES... params) {
		//	const int size = sizeof...(params);
		//	const void* res[size] = { &params... };

		//	for(int p = 0; p < size; p++) {
		//		const Signal* param = dynamic_cast<Signal*>(res[p]);
		//		if (param) {
		//			if (*param == "Frequency")
		//				print("Frequency\n");
		//			else if (*param == "Pitch")
		//				print("Pitch\n");
		//			else if (*param == "Phase")
		//				print("Phase\n");
		//		}
		//	}
		//}

		virtual void set(param frequency) {
			Oscillator::frequency = frequency;
			increment = frequency * ((2 * pi) / fs);
		}

		virtual void set(param frequency, param phase) {
			set(frequency);
			Oscillator::phase = offset = phase;
		}
	};

	class Wavetable : public Oscillator {
		using Oscillator::set;
	protected:
		Buffer buffer;
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
		Wavetable& operator=(TYPE oscillator) {
			oscillator.set(fs / size);
			for (int s = 0; s < size; s++)
				buffer[s] = oscillator;
			return *this;
		}

		virtual void set(param frequency) {
			frequency = frequency;
			increment = frequency * (size / fs);
		}

		signal output() {
			offset += { increment, size };
			return buffer[offset];
		}
	};

	class Sine : public Oscillator {
		signal output() {
			offset += increment;
			return sin(offset);
		}
	};

	class Saw : public Oscillator {
		signal output() {
			offset += increment;
			return offset * (1.f / pi) - 1;
		}
	};

	struct Sine2 : public Wavetable {
		Sine2() : Wavetable(Sine()) { }
	};

	struct Saw2 : public Wavetable {
		Saw2() : Wavetable(Saw()) { }
	};

	class Osc : public Oscillator {
		std::unique_ptr<Oscillator> osc;

	public:
		template<typename TYPE>
		void set(TYPE) {
			osc = new TYPE();
			osc->set(frequency, offset);
		}

		void set(param frequency) {
			Oscillator::set(frequency);
			osc->set(frequency);
		}

		void set(param frequency, param phase) {
			Oscillator::set(frequency, phase);
			osc->set(frequency, phase);
		}
	};

	class Envelope : public Generator {
		using Generator::set;
		class Ramp : public Generator {
			float target;
			float rate;
			bool active = false;
		public:

			Ramp(float value = 1.f) {
				setValue(value);
			}

			Ramp(float start, float target, float time) {
				setValue(start);
				setTarget(target);
				setTime(time);
			}

			bool isActive() const {
				return active;
			}

			void setTarget(float target) {
				Ramp::target = target;
				active = (out != target);
				//if (active)
				//	rate = abs(rate) * (target > output ? 1.f : -1.f);
			}
			void setValue(float value) {
				out = value;
				Ramp::target = value;
				active = false;
			}
			void setRate(float rate) { Ramp::rate = rate; }
			void setTime(float time) { Ramp::rate = time ? 1.f / (time * fs) : 0; }

			signal operator++(int) {
				signal output = out;

				if (active) {
					if (target > out) {
						out += rate;
						if (out >= target) {
							out = target;
							active = false;
						}
					} else {
						out -= rate;
						if (out <= target) {
							out = target;
							active = false;
						}
					}
				}

				return output;
			}

			signal output() {
				return out;
			}
		};

	public:
		enum Stage { Sustain, Release, Off };

		struct Point {
			float x, y;
			Point(double x = 0, double y = 0) : x(float(x)), y(float(y)) { }
		};

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

		struct Loop {
			Loop(int from = -1, int to = -1) : start(from), end(to) {}

			void set(int from, int to) { start = from; end = to; }
			void reset() { start = end = -1; }

			bool isActive() const { return start != -1 && end != -1; }

			int start;
			int end;
		};

		Envelope() {
			set(Points(0.0, 1.0));
		}

		Envelope(const Points& points) {
			set(points);
		}

		Envelope(std::initializer_list<Point> points) {
			set(points);
		}

		bool operator==(Stage stage) const { return Envelope::stage == stage; }
		bool operator!=(Stage stage) const { return Envelope::stage != stage; }

		void set(const std::vector<Point>& points) {
			Envelope::points = points;
			initialise();
		}

        void set(const Points& point){
            points.clear();
            
            const Points* pPoint = &point;
            while(pPoint){
                points.push_back(*pPoint);
                pPoint = pPoint->next;
            }
            
            initialise();
        }

        void setLoop(int startPoint, int endPoint){
            if(startPoint >= 0 && endPoint < points.size())
                loop.set(startPoint, endPoint);
        }
        
        void resetLoop(){
            loop.reset();
            if(stage == Sustain && (point+1) < points.size())
                setTarget(points[point+1], points[point].x);
        }
        
        void setStage(Stage stage){ this->stage = stage; }
        const Stage getStage() const { return stage; }
        
        float getLength() const { return points.size() ? points[points.size() - 1].x : 0.f; }
        
        void release(float time){
            stage = Release;
            ramp.setTime(time);
            ramp.setTarget(0.f);
        }

		bool finished() const {
			return getStage() == Stage::Off;
		}
        
        void initialise(){
            point = 0;
            timeInc = 1.0f / fs;
            loop.reset();
            stage = Sustain;
            if(points.size()){
                ramp.setValue(points[0].y);
                if(points.size() > 1)
                    setTarget(points[1], points[0].x);
            }else{
                ramp.setValue(1.0);
            }
        }
        
        void resize(int samples){
            float length = getLength();
            if(length == 0.0)
                return;
            
            const float multiplier = samples / (fs * length);
            std::vector<Point>::iterator point = points.begin();
            while(point != points.end()){
                point->x *= multiplier;
                point++;
            }
            
            initialise();
        }
        
        void setTarget(Point& point, float time = 0.0){
            this->time = time;
            ramp.setTarget(point.y);
            ramp.setRate(fabs(point.y - ramp.out) / ((point.x - time) * fs));
        }
        
        signal& operator++(int){ 
            out = ramp++;
            
            switch(stage){
			case Sustain:
                time += timeInc;
				if (!ramp.isActive()) { // envelop segment end reached
					if (loop.isActive() && (point + 1) >= loop.end) {
						point = loop.start;
						ramp.setValue(points[point].y);
						if (loop.start != loop.end)
							setTarget(points[point + 1], points[point].x);
					} else if ((point + 1) < points.size()) {
						if (time >= points[point + 1].x) { // reached target point
							point++;
							ramp.setValue(points[point].y); // make sure exact value is set

							if ((point + 1) < points.size()) // new target point?
								setTarget(points[point + 1], points[point].x);
						}
					} else {
						stage = Off;
					}
				} break;
			case Release:
                if(out == 0.0)
                    stage = Off;
				break;
			case Off:
				break;
            }
            
            return out;
        }

		signal output() {
			return out = ramp;
		}
        
        const Point& operator[](int point) const {
            return points[point];
        }
        
    protected:
        std::vector<Point> points;
        Loop loop;
        
        int point;
        float time, timeInc;
        Stage stage;

		Ramp ramp;
    };

	struct ADSR : public Envelope {
		param A, D, S, R;

		enum Mode { Time, Rate } mode = Time;

		ADSR() { set(0.5, 0.5, 1, 0.5f); }

		void set(param attack, param decay, param sustain, param release) {
			initialise();

			ADSR::A = attack;
			ADSR::D = decay;
			ADSR::S = sustain;
			ADSR::R = release;

			points.resize(3);
			points[0] = { 0, 0 };
			points[1] = { A, 1 };
			points[2] = { A + D, S };
			setLoop(2, 2);
		}

		void release() {
			Envelope::release(ADSR::R);
		}

		bool operator==(Envelope::Stage stage) const {
			return getStage() == stage;
		}
	};

	struct Effect : public Modifier {
		Effect() { }

		virtual void process(Buffer buffer) = 0;
	};

	class Synth;

	class Note : public Generator {
		Synth* synth;
		
	protected:
		Pitch pitch;
		Amplitude velocity;

		virtual void on(Pitch pitch, Amplitude velocity) { }
		virtual void off() { stage = Off; }
		
		virtual signal output() { return 0.f; };

		Synth* getSynthesizer() { return synth; }
	public:
		Note(Synth* synth = NULL) : synth(synth) { }

		virtual void start(Pitch pitch, Amplitude velocity) {
			stage = Onset;
			Note::pitch = pitch;
			Note::velocity = velocity;
			on(pitch, velocity);
			stage = Sustain;
		}

		virtual bool stop() {
			if (stage == Off)
				return true;

			if (stage == Release) {
				stage = Off;
			} else {
				stage = Release;
				off();
			}
			return stage != Release;
		}

		bool finished() const { return stage == Off; }

		enum Stage { Onset, Sustain, Release, Off } stage = Release;

		virtual void controlChange(int controller, int value) { };
		
		virtual bool process(Buffer buffer) {
			while (buffer)
				buffer++ = output() * velocity;
			return !finished();
		}
	};

	inline param* param::convert(Type to) {
		switch (type() > to) {
		case Type::Frequency > Type::Pitch:
			return new Pitch(log2(value / 440) * 12 + 69);

		case Type::Pitch > Type::Frequency:
			return new Frequency(440 * pow(2, (value - 69) / 12));
		
		default:
			return nullptr;
		}
	}

	inline void param::convert(const param& from, param& to) {
		switch (from.type() > to.type())
		{
		case Type::Frequency > Type::Pitch:
			to.value = log2(from.value / 440) * 12 + 69;
			break;
		case Type::Pitch > Type::Frequency:
			to.value = (float)(440 * pow(2, (from.value - 69) / 12));
			break;
		}
	}

	class Synth : public Effect {
	protected:
		const int POLYPHONY;
		Note* notes; // array of pointers to voice instances
	public:
		template<typename NOTE>
		Synth(int polyphony) : POLYPHONY(polyphony) {
			for(int v=0; v<POLYPHONY; v++)
				notes = new NOTE[POLYPHONY](this);
		}

		~Synth() {
			for(int v=0; v<POLYPHONY; v++)
				delete[] notes;
		}

		virtual void presetLoaded(int preset) { }
		virtual void optionChanged(int param, int item) { }
		virtual void buttonPressed(int param) { };

		// post processing of voice buffer (i.e. effects stage)
		virtual void process(Buffer buffer) { };
	
	};
};