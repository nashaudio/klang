#define KLANG 1
#include "KSynth.h"

#ifndef PLUGIN_NAME
#define PLUGIN_NAME MySynth
#endif

#define STR(x) #x
#define FILE(NAME,EXT) STR(NAME.EXT)
#define PLUGIN_K FILE(PLUGIN_NAME,k)

#include PLUGIN_K

extern "C" {                                                                                                                

#define CATCH_ALL(err)                              \
    catch (...) {                                   \
        throw;                                      \
    }

PTR_FUNCTION synthCreate(float sampleRate) {                                                                            
    klang::fs = sampleRate;                                                                                             
    //::stk::Stk::setSampleRate(sampleRate);                                                                              
    try { 
        return (MiniPlugin::Synth*)new KSynth(new PLUGIN_NAME());
    } catch (...) {
        return nullptr;
    }
}

VOID_FUNCTION synthDestroy(void* synth) {                                                                               
    try {
        delete (MiniPlugin::Synth*)synth;
    } catch (...) { }
}                                                                                                                       
                                                                                                                            
VOID_FUNCTION getBackground(void** data, int* size) {                                                                   
    *data = (void*)Background::data;                                                                                    
    *size = Background::size;                                                                                           
}                                                                                                                       

INT_FUNCTION getDebugData(void* synth, const float** const buffer, int* size, void** graph, void** console) {
    try {
        if (buffer) *size = ((MiniPlugin::Synth*)synth)->getDebugAudio(buffer);
        if (graph) ((MiniPlugin::Synth*)synth)->getDebugGraph(graph);
        if (console) ((MiniPlugin::Synth*)synth)->getDebugConsole(console);
        return 0;                                                       
    } catch (...) { return 1; }                                         
}
                                                                                                                           
INT_FUNCTION noteOnStart(void* note, int pitch, float velocity){                                                                
    try { 
        ((MiniPlugin::Note*)note)->onStartNote(pitch, velocity);
        return 0;
    } CATCH_ALL(1)
}                                                                                                                       
                                                                                                                            
INT_FUNCTION noteOnStop(void* note, float velocity, bool* hasRelease = NULL){                                           
    try { 
        bool terminate = ((MiniPlugin::Note*)note)->onStopNote(velocity);
        if (hasRelease) *hasRelease = !terminate;                                                                     
        return 0;                                                                                                     
    } CATCH_ALL(1)
}                                                                                                                       
                                                                                                                            
INT_FUNCTION noteOnPitchWheel(void* note, int value){                                                                   
    try { 
        ((MiniPlugin::Note*)note)->onPitchWheel(value);
        return 0;                                                           
    } CATCH_ALL(1)
}                                                                                                                       
                                                                                                                            
INT_FUNCTION noteOnControlChange(void* note, int controller, int value){                                                
    try { 
        ((MiniPlugin::Note*)note)->onControlChange(controller, value);
        return 0;                                            
    } CATCH_ALL(1)
}                                                                                                                       

INT_FUNCTION noteProcess(void* note, float** outputBuffers, int numSamples, bool* shouldContinue = NULL) {
    try {
        bool bContinue = ((MiniPlugin::Note*)note)->process(outputBuffers, 2, numSamples);
        if(shouldContinue) *shouldContinue = bContinue;                                                               
        return 0;                                                                                                     
    } CATCH_ALL(1)
}                                                                                                                       
                                                                                                                            
INT_FUNCTION synthProcess(void* synth, const float** inputBuffers, float** outputBuffers, int numSamples) {             
    try { 
        ((MiniPlugin::Synth*)synth)->process(inputBuffers, outputBuffers, numSamples);
        return 0;                            
    } catch (...) {
        return 1;
    }
}

};