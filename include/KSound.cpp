#define KLANG 1
#include "KSound.h"

#ifndef PLUGIN_NAME
#define PLUGIN_NAME MySound
#endif

#define STR(x) #x
#define FILE(NAME,EXT) STR(NAME.EXT)
#define PLUGIN_K FILE(PLUGIN_NAME,k)

#ifdef PLUGIN_HEADER
#include PLUGIN_HEADER
#else
#include PLUGIN_K
#endif

#define CATCH_ALL                                   \
    catch (...) {                                   \
        throw;                                      \
    }

extern "C" {
    PLUGIN(MiniPlugin::Effect)

    PTR_FUNCTION effectCreate(float sampleRate) {
        try {
            klang::fs = sampleRate;
            //::stk::Stk::setSampleRate(sampleRate);
            return dynamic_cast<MiniPlugin::Effect*>(new KSound(new ::PLUGIN_NAME()));
        } catch(...) {
			return nullptr;
		}
    }

    VOID_FUNCTION effectDestroy(void* effect) {
        try { delete (KSound*)effect; }
        catch (...) { }
    }

    INT_FUNCTION effectOnControl(void* effect, int param, float value) {
        try { ((MiniPlugin::Effect*)effect)->onControl(param, value); return 0; }
        catch (...) { return 1; }
    }

    INT_FUNCTION effectOnPreset(void* effect, int param) {
        try { ((MiniPlugin::Effect*)effect)->onPreset(param); return 0; }
        catch (...) { return 1; }
    }

    INT_FUNCTION effectProcess(void* effect, const float** inputBuffers, float** outputBuffers, int numSamples) {
        try { ((KSound*)effect)->process(inputBuffers, outputBuffers, numSamples); return 0;
        } CATCH_ALL
    }
}
// Sound entry points use the existing mini-plugin calling convention and handle.
extern "C" {
    PTR_FUNCTION soundCreate(float sampleRate) { return effectCreate(sampleRate); }
    VOID_FUNCTION soundDestroy(void* sound) { effectDestroy(sound); }
    INT_FUNCTION soundOnControl(void* sound, int parameter, float value) {
        return effectOnControl(sound, parameter, value);
    }
    INT_FUNCTION soundOnPreset(void* sound, int preset) { return effectOnPreset(sound, preset); }
    INT_FUNCTION soundGetNumChannels(void* sound) { return ((KSound*)sound)->getNumChannels(); }
    // Studio's mini-plugin buffers are planar stereo; mono is duplicated to both outputs.
    INT_FUNCTION soundProcess(void* sound, float** outputBuffers, int numSamples) {
        return effectProcess(sound, nullptr, outputBuffers, numSamples);
    }
}
