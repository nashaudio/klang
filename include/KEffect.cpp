#define KLANG 1
#include "KEffect.h"

#ifndef PLUGIN_NAME
#define PLUGIN_NAME MyEffect
#endif

#define STR(x) #x
#define FILE(NAME,EXT) STR(NAME.EXT)
#define PLUGIN_K FILE(PLUGIN_NAME,k)

#include PLUGIN_K

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
            return dynamic_cast<MiniPlugin::Effect*>(new KEffect(new ::PLUGIN_NAME()));
        } catch(...) {
			return nullptr;
		}
    }

    VOID_FUNCTION effectDestroy(void* effect) {
        try { delete (KEffect*)effect; }
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
        try { ((KEffect*)effect)->process(inputBuffers, outputBuffers, numSamples); return 0;
        } CATCH_ALL
    }                                                                                                                       
}