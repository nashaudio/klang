#define KLANG 1
#include "KSynth.h"

#ifndef PLUGIN_NAME
#define PLUGIN_NAME MySynth
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

PLUGIN(MiniPlugin::Synth)

PTR_FUNCTION synthCreate(float sampleRate) {                                                                            
    klang::fs = sampleRate;                                                                                             
    //::stk::Stk::setSampleRate(sampleRate);                                                                              
    try { 
        return (MiniPlugin::Synth*)new KSynth(new ::PLUGIN_NAME());
    } catch (...) {
        return nullptr;
    }
}

VOID_FUNCTION synthDestroy(void* synth) {                                                                               
    try {
        delete (KSynth*)synth;
    } catch (...) { }
}                                                                                                                       

INT_FUNCTION synthOnControl(void* synth, int param, float value) {
    try { ((MiniPlugin::Synth*)synth)->onControl(param, value); return 0; }
    catch (...) { return 1; }
}

INT_FUNCTION synthOnPreset(void* synth, int param) {
    try { ((MiniPlugin::Synth*)synth)->onPreset(param); return 0; }
    catch (...) { return 1; }
}
                                                                                                                                                                                                                                                      
INT_FUNCTION noteOnStart(void* note, int pitch, float velocity){                                                                
    try { 
        ((MiniPlugin::Note*)note)->onStartNote(pitch, velocity);
        return 0;
    } CATCH_ALL
}                                                                                                                       
                                                                                                                            
INT_FUNCTION noteOnStop(void* note, float velocity, bool* hasRelease = NULL){                                           
    try { 
        bool terminate = ((MiniPlugin::Note*)note)->onStopNote(velocity);
        if (hasRelease) *hasRelease = !terminate;                                                                     
        return 0;                                                                                                     
    } CATCH_ALL
}                                                                                                                       
                                                                                                                            
INT_FUNCTION noteOnPitchWheel(void* note, int value){                                                                   
    try { 
        ((MiniPlugin::Note*)note)->onPitchWheel(value);
        return 0;                                                           
    } CATCH_ALL
}                                                                                                                       
                                                                                                                            
INT_FUNCTION noteOnControlChange(void* note, int controller, int value){                                                
    try { 
        ((MiniPlugin::Note*)note)->onControlChange(controller, value);
        return 0;                                            
    } CATCH_ALL
}           

INT_FUNCTION noteOnControl(void* note, int param, float value) {
    try { ((MiniPlugin::Note*)note)->onControl(param, value); return 0; }
    catch (...) { return 1; }
}

INT_FUNCTION noteOnPreset(void* note, int param) {
    try { ((MiniPlugin::Note*)note)->onPreset(param); return 0; }
    catch (...) { return 1; }
}

INT_FUNCTION noteProcess(void* note, float** outputBuffers, int numSamples, bool* shouldContinue = NULL) {
    try {
        bool bContinue = ((MiniPlugin::Note*)note)->process(outputBuffers, 2, numSamples);
        if(shouldContinue) *shouldContinue = bContinue;                                                               
        return 0;                                                                                                     
    } CATCH_ALL
}                                                                                                                       
                                                                                                                            
INT_FUNCTION synthProcess(void* synth, const float** inputBuffers, float** outputBuffers, int numSamples) {             
    try { 
        ((MiniPlugin::Synth*)synth)->process(inputBuffers, outputBuffers, numSamples);
        return 0;                            
    } CATCH_ALL
}

};