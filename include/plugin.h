//
//  Plugin.h
//  Effect & Synth Plugin Framework - Mini-Plugin Wrappers
//
//  Created by Chris Nash on 02/10/2013.
//  Updated by Chris Nash on 18/01/2016 with support for OS X 10.10 SDK.
//  Updated by Chris Nash on 02/10/2018 with support for TestEffectAU.
//  Updated by Chris Nash on 02/09/2020 with new architecture for MacOS and Windows.
//  Updated by Chris Nash on 10/08/2022 with Klang support and Base64 backgrounds.
//
//  This file describes a number of abstractions and extensions to STK,
//  to support audio processing and programming teaching at UWE.

#include <string>
#include "klang.h"

#pragma once

#define STR(x) #x
#define FILE(NAME,EXT) STR(NAME.EXT)

#define PLUGIN_K FILE(PLUGIN_NAME,k)

#ifndef BACKGROUND
    #if __has_include("Background")
        #include "Background"
    #else
        #define BACKGROUND 0
        //#pragma message("Background not found")
    #endif

    namespace Background {
        thread_local static const char data[] = {
            BACKGROUND 
        };
        thread_local static const int size = sizeof(data);
    }
#endif

namespace klang { struct Graph; struct Console; }

#if defined(_WIN32)
#define PTR_FUNCTION   __declspec(dllexport) void* __stdcall
#define UINT_FUNCTION   __declspec(dllexport) unsigned int __stdcall
#define INT_FUNCTION   __declspec(dllexport) int __stdcall
#define VOID_FUNCTION  __declspec(dllexport) void __stdcall
#else
#define PTR_FUNCTION   void*
#define UINT_FUNCTION   unsigned int
#define INT_FUNCTION   int
#define VOID_FUNCTION  void
#endif

#define PLUGIN(CLASS)                                                                                                       \
extern "C" {                                                                                                                \
    UINT_FUNCTION getVersion() { return *(unsigned int*)(&klang::version); }                                                \
																															\
    VOID_FUNCTION getBackground(void** data, int* size) {                                                                   \
        *data = (void*)Background::data;                                                                                    \
        *size = Background::size;                                                                                           \
    }                                                                                                                       \
                                                                                                                            \
    INT_FUNCTION getDebugData(void* plugin, const float** const buffer, int* size, void** graph, void** console) {          \
        try {                                                                                                               \
            if (buffer) *size = ((CLASS*)plugin)->getDebugAudio(buffer);                                                    \
            if (graph) ((CLASS*)plugin)->getDebugGraph(graph);                                                              \
            if (console) ((CLASS*)plugin)->getDebugConsole(console);                                                        \
            return 0;                                                                                                       \
        }                                                                                                                   \
        catch (...) { return 1; }                                                                                           \
    }                                                                                                                       \
}

#if !KLANG
#define EFFECT(NAME)                                                                                                        \
extern "C" {                                                                                                                \
    PLUGIN(MiniPlugin::Effect)                                                                                              \
																															\
    PTR_FUNCTION effectCreate(float sampleRate) {                                                                           \
        try {                                                                                                               \
            klang::fs = sampleRate;                                                                                         \
            ::stk::Stk::setSampleRate(sampleRate);                                                                          \
            return (MiniPlugin::Effect*)new NAME();                                                                         \
        } catch (...) {                                                                                                     \
            return nullptr;                                                                                                 \
        }                                                                                                                   \
    }                                                                                                                       \
                                                                                                                            \
    VOID_FUNCTION effectDestroy(void* effect) {                                                                             \
        try { delete (NAME*)effect; }                                                                                       \
        catch (...) { }                                                                                                     \
    }                                                                                                                       \
                                                                                                                            \
    INT_FUNCTION effectOnControl(void* effect, int param, float value) {                                                    \
        try { ((MiniPlugin::Effect*)effect)->onControl(param, value); return 0; }                                           \
        catch (...) { return 1; }                                                                                           \
    }                                                                                                                       \
                                                                                                                            \
    INT_FUNCTION effectOnPreset(void* effect, int param) {                                                                  \
        try { ((MiniPlugin::Effect*)effect)->onPreset(param); return 0; }                                                   \
        catch (...) { return 1; }                                                                                           \
    }                                                                                                                       \
                                                                                                                            \
    INT_FUNCTION effectProcess(void* effect, const float** inputBuffers, float** outputBuffers, int numSamples) {           \
        try { ((MiniPlugin::Effect*)effect)->process(inputBuffers, outputBuffers, numSamples); return 0;                    \
        } catch(...) { return 1; }                                                                                          \
    }                                                                                                                       \
}

#define SYNTH(NAME)                                                                                                         \
extern "C" {                                                                                                                \
    PLUGIN(MiniPlugin::Synth)                                                                                               \
                                                                                                                            \
    PTR_FUNCTION synthCreate(float sampleRate) {                                                                            \
        ::stk::Stk::setSampleRate(sampleRate);                                                                              \
        return (MiniPlugin::Synth*)new NAME();                                                                              \
    }                                                                                                                       \
                                                                                                                            \
    VOID_FUNCTION synthDestroy(void* synth) {                                                                               \
        try { delete (NAME*)synth; }                                                                                        \
        catch (...) { }                                                                                                     \
    }                                                                                                                       \
                                                                                                                            \
    INT_FUNCTION noteOnStart(void* note, int pitch, float velocity){                                                        \
        try { ((MiniPlugin::Note*)note)->onStartNote(pitch, velocity); return 0;                                            \
        } catch(...) { return 1; }                                                                                          \
    }                                                                                                                       \
                                                                                                                            \
    INT_FUNCTION noteOnStop(void* note, float velocity, bool* hasRelease = NULL){                                           \
        try { bool terminate = ((MiniPlugin::Note*)note)->onStopNote(velocity);                                             \
              if (hasRelease) *hasRelease = !terminate;                                                                     \
              return 0;                                                                                                     \
        } catch(...) { return 1; }                                                                                          \
    }                                                                                                                       \
                                                                                                                            \
    INT_FUNCTION noteOnPitchWheel(void* note, int value){                                                                   \
        try { ((MiniPlugin::Note*)note)->onPitchWheel(value); return 0;                                                     \
        } catch(...) { return 1; }                                                                                          \
    }                                                                                                                       \
                                                                                                                            \
    INT_FUNCTION noteOnControlChange(void* note, int controller, int value){                                                \
        try { ((MiniPlugin::Note*)note)->onControlChange(controller, value); return 0;                                      \
        } catch(...) { return 1; }                                                                                          \
    }                                                                                                                       \
                                                                                                                            \
    INT_FUNCTION noteOnControl(void* note, int param, float value) {                                                        \
        try { ((MiniPlugin::Note*)note)->onControl(param, value); return 0; }                                               \
        catch (...) { return 1; }                                                                                           \
    }                                                                                                                       \
                                                                                                                            \
    INT_FUNCTION noteOnPreset(void* note, int param, float value) {                                                         \
        try { ((MiniPlugin::Note*)note)->onPreset(param); return 0; }                                                       \
        catch (...) { return 1; }                                                                                           \
    }                                                                                                                       \
                                                                                                                            \
    INT_FUNCTION noteProcess(void* note, float** outputBuffers, int numSamples, bool* shouldContinue = NULL) {              \
        try { bool bContinue = ((MiniPlugin::Note*)note)->process(outputBuffers, 2, numSamples);                            \
              if(shouldContinue) *shouldContinue = bContinue;                                                               \
              return 0;                                                                                                     \
        } catch(...) { return 1; }                                                                                          \
    }                                                                                                                       \
                                                                                                                            \
    INT_FUNCTION synthOnControl(void* synth, int param, float value) {                                                      \
        try { ((MiniPlugin::Synth*)synth)->onControl(param, value); return 0; }                                             \
        catch (...) { return 1; }                                                                                           \
    }                                                                                                                       \
                                                                                                                            \
    INT_FUNCTION synthOnPreset(void* synth, int param, float value) {                                                       \
        try { ((MiniPlugin::Synth*)synth)->onPreset(param); return 0; }                                                     \
        catch (...) { return 1; }                                                                                           \
    }                                                                                                                       \
                                                                                                                            \
    INT_FUNCTION synthProcess(void* synth, const float** inputBuffers, float** outputBuffers, int numSamples) {             \
        try { ((MiniPlugin::Synth*)synth)->process(inputBuffers, outputBuffers, numSamples); return 0;                      \
        } catch(...) { return 1; }                                                                                          \
    }                                                                                                                       \
}
#endif

namespace DSP {
    class Wavetable;
}

namespace MiniPlugin
{
    using klang::Text;
    using klang::Caption;
    using klang::Array;
    using klang::Control;
    typedef klang::Control Parameter;

    using klang::Dial;
    using klang::Button;
    using klang::Toggle;
    using klang::Slider;

    using klang::Preset;
    using klang::Presets;

	const Parameter::Size AUTO_SIZE = { -1, -1, -1, -1 };
	
    struct Parameters : Array<Parameter, 128>
    {
		float value[128] = { 0 }; // 128 * 4 bytes (512 bytes)
        Array<Control::Group, 10> groups;

        void operator+= (const Parameter& control) {
            items[count++] = control;
        }

        bool inGroup(int index) {
            for (unsigned int g = 0; g < groups.count; g++) {
                if (groups[g].contains(index))
                    return true;
            }
            return false;
        }

        const Control::Group* getGroup(int index) {
            for (unsigned int g = 0; g < groups.count; g++) {
                if (groups[g].contains(index))
                    return &groups[g];
            }
            return nullptr; // Not found
        }

        void operator= (const Parameters& controls) {
            for (int c = 0; c < 128 && controls[c].type != Parameter::NONE; c++)
                operator+=(controls[c]);
        }

        void operator= (const klang::Controls& controls) {
            for (int c = 0; c < 128 && controls[c].type != Parameter::NONE; c++)
                operator+=(controls[c]);
        }

        void operator=(std::initializer_list<Parameter> controls) {
            for (auto control : controls)
                operator+=(control);
        }

        void add(const char* name, Parameter::Type type = Parameter::ROTARY, float min = 0.f, float max = 1.f, float initial = 0.f, Parameter::Size size = AUTO_SIZE) {
            items[count].name = name;
            items[count].type = type;
            items[count].min = min;
            items[count].max = max;
            items[count].initial = initial;
            items[count].size = size;
            items[count++].value = initial;
        }

        void add(const char* name, Parameter::Type type, std::initializer_list<const char*> options, Parameter::Size size = AUTO_SIZE) {
            items[count].name = name;
            items[count].type = type;
            items[count].min = 0.f;
            items[count].max = (float)options.size() - 1;
            items[count].initial = 0;
            auto option = options.begin();
            while (option != options.end()) {
                items[count].options.add(Caption::from(*option));
                option++;
            }
            items[count].size = size;
            items[count++].value = 0;
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
    };

    struct Debug {
        const float* buffer = nullptr;
        klang::Graph* graph = nullptr;
        klang::Console* console = nullptr;
        int size = 0;

        void reset() { buffer = nullptr; graph = nullptr; console = nullptr; size = 0; }
    };

    class Effect
    {
    public:       
        virtual void process(const float** inputBuffers, float** outputBuffers, int numSamples) = 0;
        
        // events
        virtual void onControl(int param, float value) { };
        virtual void onPreset(int preset) { };

        // events (DEPRECATED)
        virtual void presetLoaded(int iPresetNum, const char *sPresetName) { };
        virtual void optionChanged(int iOptionMenu, int iItem) { };
        virtual void buttonPressed(int iButton) { };
        
        virtual void setSampleRate(float sr) = 0;
        virtual float getSampleRate() const = 0;

        virtual int getDebugAudio(const float** buffer) const {
            if (buffer)
                *buffer = debug.buffer;
            return debug.size;
        }

        virtual void getDebugGraph(void** graph) const {
            if (graph)
                *graph = debug.graph;
        }

        virtual void getDebugConsole(void** console) const {
            if (console)
                *console = debug.console;
        }
        
        Parameters parameters;
        Presets presets;

        Debug debug;
    };

    class Note;

    class Synth : public Effect
    {
    public:
        Synth()
        : wavetables(nullptr) { }
        
        Synth(const char* resources) : wavetables(nullptr) {
            try { if(resources) loadWavetables(resources); } catch (...) { };
        }
        
        virtual ~Synth() {
#if !KLANG
            try { unloadWavetables(); } catch (...) { };
#endif
        }
               
        Note* notes[128] = { 0 };
        const DSP::Wavetable* const getWavetable(int index) const;
        
    private:
        DSP::Wavetable* wavetables;
        void loadWavetables(const char* path);
        void unloadWavetables();
    };

    class Note
    {
    public:
        Note(Synth* synth) : parameters(synth->parameters), presets(synth->presets), synth(synth) { }
        virtual ~Note() { }
            
        float getSampleRate() const { return synth->getSampleRate(); }
            
        virtual void onStartNote (int pitch, float velocity) = 0;
        virtual bool onStopNote (float velocity) = 0;
            
        virtual void onPitchWheel (const int value) { };
        virtual void onControlChange (const int controller, const int value) { };
        virtual void onControl(int param, float value) { };
        virtual void onPreset(int preset) { };
            
        virtual bool process (float** outputBuffer, int numChannels, int numSamples) = 0;
                
    protected:
        Parameters& parameters;
        Presets& presets;
        Synth* synth;
    };

} // namespace MiniPlugin

