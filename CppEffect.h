//
//  CppEffect.h
//  CppEffect Plugin Header File
//
//  Used to declare objects and data structures used by the plugin.
//

#pragma once

#include "include/plugin.h"
#include "include/dsp.h"
using namespace DSP;

//#include "Extra.h"

class CppEffect : public MiniPlugin::Effect
{
public:
    CppEffect();     // constructor (initialise variables, etc.)
    ~CppEffect();    // destructor (clean up, free memory, etc.)

    void setSampleRate(float sampleRate){ stk::Stk::setSampleRate(sampleRate); }
    float getSampleRate() const { return stk::Stk::sampleRate(); };
    
    void process(const float** inputBuffers, float** outputBuffers, int numSamples);
    
    void presetLoaded(int iPresetNum, const char *sPresetName);
    void optionChanged(int iOptionMenu, int iItem);
    void buttonPressed(int iButton);

private:
    // Declare shared member variables here
};