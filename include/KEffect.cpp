#define KLANG 1
#include "KEffect.h"

#ifndef PLUGIN_NAME
#define PLUGIN_NAME MyEffect
#endif

#define STR(x) #x
#define FILE(NAME,EXT) STR(NAME.EXT)
#define PLUGIN_K FILE(PLUGIN_NAME,k)

#include PLUGIN_K

extern "C" {                                                                                                                
    PTR_FUNCTION effectCreate(float sampleRate) {                                                                           
        try {
            klang::fs = sampleRate;
            ::stk::Stk::setSampleRate(sampleRate);
            return (DSP::Effect*)new KEffect(new PLUGIN_NAME());
        } catch(...) {
			return nullptr;
		}
    }
                                                                                                                            
    VOID_FUNCTION getBackground(void** data, int* size) {                                                                   
        *data = (void*)Background::data;                                                                                    
        *size = Background::size;                                                                                           
    }                                                                                                                       

    INT_FUNCTION getDebugData(void* effect, const float** const buffer, int* size, void** graph, void** console) {
        try {
            if (buffer) *size = ((DSP::Effect*)effect)->getDebugAudio(buffer);
            if (graph) ((DSP::Effect*)effect)->getDebugGraph(graph);
            if (console) ((DSP::Effect*)effect)->getDebugConsole(console);
            return 0;
        } catch (...) { return 1; }
    }
                                                                                                                            
    VOID_FUNCTION effectDestroy(void* effect) {                                                                             
        try { delete (DSP::Effect*)effect; }
		catch(...) { }
    }                                                                                                                       
                                                                                                                            
    INT_FUNCTION effectProcess(void* effect, const float** inputBuffers, float** outputBuffers, int numSamples) {           
        try { ((DSP::Effect*)effect)->process(inputBuffers, outputBuffers, numSamples); return 0;
        } catch(...) { return 1; }                                                                                          
    }                                                                                                                       
}