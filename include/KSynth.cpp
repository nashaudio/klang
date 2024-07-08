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
        throw err;                                 \
    }

PTR_FUNCTION synthCreate(float sampleRate) {                                                                            
    klang::fs = sampleRate;                                                                                             
    ::stk::Stk::setSampleRate(sampleRate);                                                                              
    try { 
        return (DSP::Synth*)new KSynth(new PLUGIN_NAME());
    } CATCH_ALL(nullptr)
}

VOID_FUNCTION synthDestroy(void* synth) {                                                                               
    delete (DSP::Synth*)synth;
}                                                                                                                       
                                                                                                                            
VOID_FUNCTION getBackground(void** data, int* size) {                                                                   
    *data = (void*)Background::data;                                                                                    
    *size = Background::size;                                                                                           
}                                                                                                                       

INT_FUNCTION getDebugData(void* synth, const float** const buffer, int* size, void** graph, void** console) {
    try {
        if (buffer) *size = ((DSP::Synth*)synth)->getDebugAudio(buffer);
        if (graph) ((DSP::Synth*)synth)->getDebugGraph(graph);          
        if (console) ((DSP::Synth*)synth)->getDebugConsole(console);
        return 0;                                                       
    } catch (...) { return 1; }                                         
}
                                                                                                                           
INT_FUNCTION noteOnStart(void* note, int pitch, float velocity){                                                                
    try { 
        ((DSP::Note*)note)->onStartNote(pitch, velocity);
        return 0;
    } CATCH_ALL(1)
}                                                                                                                       
                                                                                                                            
INT_FUNCTION noteOnStop(void* note, float velocity, bool* hasRelease = NULL){                                           
    try { 
        bool terminate = ((DSP::Note*)note)->onStopNote(velocity);
        if (hasRelease) *hasRelease = !terminate;                                                                     
        return 0;                                                                                                     
    } CATCH_ALL(1)
}                                                                                                                       
                                                                                                                            
INT_FUNCTION noteOnPitchWheel(void* note, int value){                                                                   
    try { 
        ((DSP::Note*)note)->onPitchWheel(value);
        return 0;                                                           
    } CATCH_ALL(1)
}                                                                                                                       
                                                                                                                            
INT_FUNCTION noteOnControlChange(void* note, int controller, int value){                                                
    try { 
        ((DSP::Note*)note)->onControlChange(controller, value);
        return 0;                                            
    } CATCH_ALL(1)
}                                                                                                                       

INT_FUNCTION noteProcess(void* note, float** outputBuffers, int numSamples, bool* shouldContinue = NULL) {
    try {
        bool bContinue = ((DSP::Note*)note)->process(outputBuffers, 2, numSamples);
        if(shouldContinue) *shouldContinue = bContinue;                                                               
        return 0;                                                                                                     
    } CATCH_ALL(1)
}                                                                                                                       
                                                                                                                            
INT_FUNCTION synthProcess(void* synth, const float** inputBuffers, float** outputBuffers, int numSamples) {             
//    try { 
    ((DSP::Synth*)synth)->process(inputBuffers, outputBuffers, numSamples); 
        return 0;                            
//    } catch (...) {
//        return 1;
//    }
}

};