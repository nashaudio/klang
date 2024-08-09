//
//  Helpers.h
//  Effect & Synth Plugin Framework - Helper DSP Objects / Functions
//
//  Created by Chris Nash on 02/10/2013.
//  Updated by Chris Nash on 18/01/2016 with support for OS X 10.10 SDK.
//  Updated by Chris Nash on 02/10/2018 with support for TestEffectAU.
//  Updated by Chris Nash on 02/09/2020 with new platform-agnostic architecture for MacOS and Windows.
//  Updated by Chris Nash on 01/02/2021 with support for MySynth.   
//
//  This file describes a number of abstractions and extensions to STK,
//  to support audio processing and programming learning & teaching.

#pragma once

#include "stk.h"
#if !defined(M_PI)
#define M_PI_2 1.5707963267948966192313216916398f
#define M_PI 3.1415926535897932384626433832795f
#define M_3PI_2 4.7123889803846898576939650749193f
#define M_2PI 6.28318530717958647692528676655901f
#endif

#include "plugin.h"

//==============================================================================
// DSP OBJECTS - These STK objects have been adapted to support development.
// The original STK objects they are based on are identified by the stk:: label
// (e.g. SineWave for sine). To find out more about them, you can either drill
// down to see their implementation (selecting them and "Jump to Definition"),
// or look them up in the STK documentation (found online).
//==============================================================================

namespace DSP {
    using namespace MiniPlugin;

    static float getSampleRate() { return ::stk::Stk::sampleRate(); }
    
    typedef stk::Generator Oscillator;
    
    class Sine : public stk::SineWave
    {
    public:
        void setFrequency(float f) {
            frequency = f;
            stk::SineWave::setFrequency(f);
        }
        
        float getFrequency() const {
            return frequency;
        }
    protected:
        float frequency;
    };
    
    class Square : public stk::BlitSquare {};
    // class Triangle {};
    class Saw : public stk::BlitSaw {};
    class Noise : public stk::Noise {};
    
    class Delay : public stk::DelayL {};
    
    class Filter : public stk::BiQuad {};
    class LPF : public Filter {
    public:
        LPF() : Filter() {
            setCutoff(20000.0);
        }
        
        void setCutoff(float frequency){
            float fOmega = M_PI * (frequency/sampleRate());
            float fKval = tan(fOmega);
            float fKvalsq = fKval * fKval;
            float fRootTwo = sqrt(2.0f);
            float ffrac = 1.0f / (1.0f + fRootTwo * fKval + fKvalsq);
            
            setB0(fKvalsq * ffrac);
            setB1(2.0f * fKvalsq * ffrac);
            setB2(fKvalsq * ffrac);
            
            //      setA0(0.0);
            setA1(2.0f * (fKvalsq - 1.0f) * ffrac);
            setA2((1.0f - fRootTwo * fKval + fKvalsq) * ffrac);
        }
    };
    class HPF : public Filter {
    public:
        HPF() : Filter() {
            setCutoff(0.f);
        }
        
        void setCutoff(float frequency){
            float fOmega = M_PI * (frequency/sampleRate());
            float fKval = tan(fOmega);
            float fKvalsq = fKval * fKval;
            float fRootTwo = sqrt(2.f);
            float ffrac = 1.f / (1.f + fRootTwo * fKval + fKvalsq);
            
            setB0(ffrac);
            setB1(-2.f * ffrac);
            setB2(ffrac);
            
            //      setA0(0.0);
            setA1(2.f * (fKvalsq - 1.f) * ffrac);
            setA2((1.f - fRootTwo * fKval + fKvalsq) * ffrac);
        }
    };
    
    class BPF : public Filter {
        using stk::BiQuad::tick;
    public:
        BPF() : Filter() {
            set(1000.0, 100.0);
        }
        
        void setQ(float centre, float Q){
            set(centre, centre / Q);
        }
        
        void set(float centre, float bandwidth){
            const float fSampleRate = sampleRate();
            
            // if possible, better to fix out of range values than fail silently
            if(centre < 20) centre = 20; // value of 20 produces less clicks than allowing all the way to 0
            if(bandwidth < 0) bandwidth = 0;
            
            // Check for values which cause instability and make corrections (rather than
            // failing completely by returning)
            // In particular, tan(PI/2) tends to infinity, so must be avoided:
            // --> fc must be < 0.5 Fs
            // --> bw must be < 0.25 Fs
            // keep them slightly under just to be on the safe side
            if(centre > 0.49f * fSampleRate) {
                centre = 0.49f * fSampleRate;
            }
            if(bandwidth > 0.24f * fSampleRate) {
                bandwidth = 0.24f * fSampleRate;
            }
            
            float fOmegaA = M_PI * (centre/fSampleRate);
            float fOmegaB = M_PI * (bandwidth/fSampleRate);
            float fCval = (tan(fOmegaB) - 1.f) / (tan(2.0f * fOmegaB) + 1.f);
            float fDval = -1.0f * cos(2.0f * fOmegaA);
            
            setB0(-1.0f * fCval);
            setB1(fDval * (1.0f - fCval));
            setB2(1.0f);
            
            //		setA0(0.0);
            setA1(-1.0f * fDval * (1.0f - fCval));
            setA2(fCval);
        }
        
        float tick(float sample){
            inputs_[2] = inputs_[1];
            inputs_[1] = inputs_[0];
            outputs_[2] = outputs_[1];
            outputs_[1] = outputs_[0];
            inputs_[0] = sample;
            
            outputs_[0] = inputs_[0] * b_[0]
            + inputs_[1] * b_[1]
            + inputs_[2] * b_[2]
            + outputs_[1] * a_[1]
            + outputs_[2] * a_[2];
            
            return 0.5f * (inputs_[0] - outputs_[0]); // BPF
            //      return 0.5 * (fFiltvalx[0] + fFiltvaly[0]); // BSF
        }
    };
    
    
    class Envelope : public stk::Envelope {
        using stk::Envelope::tick;
    public:
        enum STAGE
        {
            ENV_SUSTAIN,
            ENV_RELEASE,
            ENV_OFF
        };
        
        struct Point
        {
            float x;
            float y;
        };
        
        struct Points : public Point
        {
            Points(float x, float y){
                Point::x = x;
                Point::y = y;
                next = NULL;
            }
            Points(){
                delete next;
            }
            
            Points& operator()(float x, float y){
                last().next = new Points(x, y);
                return *this;
            }
            
            Points& last(){
                return next ? next->last() : *this;
            }
            
            int count() const {
                return next ? 1 + next->count() : 1;
            }
            
            Points* next;
        };
        
        struct Loop
        {
            Loop(int from = -1, int to = -1) : start(from), end(to) {}
            
            void set(int from, int to) { start = from; end = to; }
            void reset() { start = end = -1; }
            
            bool isActive() const { return start != -1 && end != -1; }
            
            int start;
            int end;
        };
        
        Envelope() : stk::Envelope() {
            set(Points(0.0,1.0));
            setLoop(0,0);
        }
        
        Envelope(const Points& points) : stk::Envelope() {
            set(points);
            setLoop(0,0);
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
            if(stage == ENV_SUSTAIN && (point+1) < points.size())
                setTarget(points[point+1], points[point].x);
        }
        
        void setStage(STAGE stage){ this->stage = stage; }
        const STAGE getStage() const { return stage; }
        
        float getLength() const { return points.size() ? points[points.size() - 1].x : 0.f; }
        
        void release(float time){
            stage = ENV_RELEASE;
            setTime(time);
            stk::Envelope::setTarget(0.f);
        }
        
        void initialise(){
            point = 0;
            timeInc = 1.0f / getSampleRate();
            loop.reset();
            stage = ENV_SUSTAIN;
            if(points.size()){
                setValue(points[0].y);
                if(points.size() > 1)
                    setTarget(points[1], points[0].x);
            }else{
                stk::Envelope::setValue(1.0);
            }
        }
        
        void resize(int samples){
            float length = getLength();
            if(length == 0.0)
                return;
            
            float multiplier = samples/(sampleRate() * length);
            std::vector<Point>::iterator point = points.begin();
            while(point != points.end()){
                point->x *= multiplier;
                point++;
            }
            
            initialise();
        }
        
        void setTarget(Point& point, float time = 0.0){
            this->time = time;
            stk::Envelope::setTarget(point.y);
            stk::Envelope::setRate(fabs(point.y - value_) / ((point.x - time) * Stk::sampleRate()));
        }
        
        float tick(){
            float amplitude = stk::Envelope::tick();
            
            if(stage == ENV_SUSTAIN){
                time += timeInc;
                if(stk::Envelope::getState() == 0){ // envelop segment end reached
                    if(loop.isActive() && (point+1) >= loop.end){
                        point = loop.start;
                        stk::Envelope::setValue(points[point].y);
                        if(loop.start != loop.end){
                            setTarget(points[point+1], points[point].x);
                        }
                    }else if((point+1) < points.size()){
                        if(time >= points[point+1].x){ // reached target point
                            point++;
                            setValue(points[point].y); // make sure exact value is set
                            
                            if((point+1) < points.size()) // new target point?
                                setTarget(points[point+1], points[point].x);
                        }
                    }else{
                        stage = ENV_OFF;
                    }
                }
            }else if(stage == ENV_RELEASE){
                if(amplitude == 0.0)
                    stage = ENV_OFF;
            }
            
            return amplitude;
        }
        
        const Point& operator[](int point) const {
            return points[point];
        }
        
    private:
        std::vector<Point> points;
        Loop loop;
        
        int point;
        float time, timeInc;
        STAGE stage;
    };
    
    template<typename T>
    class Array : public std::vector<T>
    {
        typedef std::vector<T> ArrayType;
    public:
        void add(T value){
            ArrayType::push_back(value);
        }
        void remove(T value){
            typename ArrayType::iterator p = std::find(ArrayType::begin(), ArrayType::end(), value);
            if(p != ArrayType::end())
                ArrayType::erase(p);
        }
    };
    
    typedef float (*Function)(float x);
    typedef float (*FunctionA)(float x, float a);
    typedef float (*FunctionAB)(float x, float a, float b);
    //typedef float (*FunctionP)(float x, DSP::Note* ptr);
    
    class Wavetable : public stk::FileLoop
    {
        using stk::FileLoop::tick;
    public:
        Wavetable() : fBaseFrequency(261.626f) {}
        
        void setFrequency( float frequency ) {
            setRate( frequency / fBaseFrequency);
        };
        
        void setBaseFrequency( float frequency ) {
            fBaseFrequency = frequency;
        }
        
        virtual float tick(){
            interpolate_ = true;
            chunking_ = false;
            return FileLoop::tick();
        }
        
        virtual float tick(float phase){
            interpolate_ = true;
            chunking_ = false;
            time_ = phase * file_.fileSize();
            return FileLoop::tick();
        }
        
        Wavetable& operator=(const Wavetable& in){
            // Call close() in case another file is already open.
            this->closeFile();
            
            // Attempt to open the file ... an error might be thrown here.
            file_ = in.file_;
            
            // Determine whether chunking or not.
            if ( in.file_.fileSize() > chunkThreshold_ ) {
                chunking_ = in.chunking_;
                chunkPointer_ = in.chunkPointer_;
                data_.resize( in.chunkSize_ + 1, in.file_.channels() );
                normalizing_ = in.normalizing_;
            }
            else {
                chunking_ = false;
                data_.resize( in.file_.fileSize() + 1, in.file_.channels() );
            }
            
            // Load all or part of the data.
            data_ = in.data_;
            
            if ( chunking_ ) { // If chunking, save the first sample frame for later.
                firstFrame_.resize( 1, data_.channels() );
                for ( unsigned int i=0; i<data_.channels(); i++ )
                    firstFrame_[i] = data_[i];
            }
            else {  // If not chunking, copy the first sample frame to the last.
                for ( unsigned int i=0; i<data_.channels(); i++ )
                    data_( data_.frames() - 1, i ) = data_[i];
            }
            
            // Resize our lastOutputs container.
            lastFrame_.resize( 1, in.file_.channels() );
            
            // Set default rate based on file sampling rate.
            
            this->setRate(1.0);
//            this->setRate( in.data_.dataRate() / Stk::sampleRate() );
            this->reset();
            
            fBaseFrequency = in.fBaseFrequency;
            
            return *this;
        }
        
        Wavetable& operator=(const Envelope& envelope){
            if(envelope.getLength() == 0.f)
                return *this;
            
            const int waveLength = (int)getSampleRate();
            
            Envelope new_envelope = envelope;
            new_envelope.resize(waveLength);
            
            file_.set(waveLength);
            data_.resize(waveLength + 1);
            lastFrame_.resize(1,1);
            float* pSample = &data_[0];
            int nbChannels = data_.channels();
            
            for(int x=0; x<waveLength; x++){
                float sample = x == 0 ? new_envelope.lastOut() : new_envelope.tick();
                for(int c=0; c<nbChannels; c++)
                    *pSample++ = sample;
            }

            // If not chunking, copy the first sample frame to the last.
            for ( unsigned int i=0; i<data_.channels(); i++ )
                    data_( data_.frames() - 1, i ) = data_[i];
            
            this->setRate(1.f);
            setBaseFrequency(1.f);
            this->reset();
            
            return *this;
        }
        
        #define BEGIN_DISTORT                               \
            float waveLength = (float)file_.fileSize();     \
            float* pSample = &data_[0];                     \
            int nbChannels = data_.channels();              \
            for(int x=0; x<waveLength; x++){
        
        #define END_DISTORT                                 \
                for(int c=0; c<nbChannels; c++)             \
                    *pSample++ = sample;                    \
            }

        void distort(Function function){
            BEGIN_DISTORT
                float sample = (function)(*pSample);
            END_DISTORT }
        
        void distort(FunctionA function, float a){
            BEGIN_DISTORT
                float sample = (function)(*pSample, a);
            END_DISTORT
        }
        
        void distort(FunctionAB function, float a, float b){
            BEGIN_DISTORT
                float sample = (function)(*pSample, a, b);
            END_DISTORT
        }
        
        /*void distort(FunctionP function, Note* note){
            BEGIN_DISTORT
                float sample = (function)(*pSample, note);
            END_DISTORT
        }*/
        
        void setOffset(float samples){
            time_ = samples;
            
            if ( time_ < 0.0f ) time_ = 0.0f;
            if ( time_ > file_.fileSize() - 1.0f ) {
                time_ = file_.fileSize() - 1.0f;
                for ( unsigned int i=0; i<lastFrame_.size(); i++ ) lastFrame_[i] = 0.0f;
                finished_ = true;
            }
        }
        
        #define BEGIN_GENERATE                                            \
            reset();                                                      \
                                                                          \
            const float fLength = getSampleRate();                        \
            const int iLength = (int)fLength + 128;                       \
                                                                          \
            file_.set(iLength);                                           \
            data_.resize(iLength + 1);                                    \
            float* pSample = &data_[0];                                   \
            int nbChannels = data_.channels();                            \
            lastFrame_.resize(1,1);                                       \
            for(int x=0; x<iLength; x++){                                 \
                float phase = (float)((2.0 * M_PI * x) / fLength);
                
        #define END_GENERATE                                              \
                for(int c=0; c<nbChannels; c++)                           \
                    *pSample++ = sample;                                  \
            }                                                             \
            setBaseFrequency(1.f);                                        \
            for ( unsigned int i=0; i<data_.channels(); i++ )             \
                    data_( data_.frames() - 1, i ) = data_[i];            \
            reset();
        
        void generate(Function function){
            BEGIN_GENERATE
                float sample = (function)(phase);
            END_GENERATE
        }
        
        void generate(FunctionA function, float a){
            BEGIN_GENERATE
                float sample = (function)(phase, a);
            END_GENERATE
        }
        
        void generate(FunctionAB function, float a, float b){
            BEGIN_GENERATE
                float sample = (function)(phase, a, b);
            END_GENERATE
        }
        
        //void generate(FunctionP function, Note* note){
        //    BEGIN_GENERATE
        //        float sample = (function)(phase, note);
        //    END_GENERATE
        //}
        
    private:
        float fBaseFrequency;
    };
    
    class Buffer : public Wavetable
    {
    public:
        float tick(){
            return stk::FileWvIn::tick();
        }
        
        float getDuration() const {
            return getSize() / getSampleRate();
        }
    };

} // namespace DSP

inline const DSP::Wavetable* const MiniPlugin::Synth::getWavetable(int index) const {
    return wavetables ? &wavetables[index] : nullptr;
}

inline void MiniPlugin::Synth::loadWavetables(const char* path)
{
    if (!path || wavetables)
        return;
    wavetables = new DSP::Wavetable[16];
    char wav_path[1024] = { 0 };
    for (int w = 0; w < 16; w++) {
        snprintf(wav_path, 1024, "%sSound%02d.wav", path, w);
        wavetables[w].openFile(wav_path);
        wavetables[w].detachFile();
    }
}

inline void MiniPlugin::Synth::unloadWavetables()
{
    if (!wavetables) return;
    delete[] wavetables;
    wavetables = nullptr;
}
