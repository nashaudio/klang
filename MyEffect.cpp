//
//  MyEffect.cpp
//  MyEffect Plugin Source Code
//
//  Used to define the bodies of functions used by the plugin, as declared in Plugin.h.
//

#include "MyEffect.h"

EFFECT(MyEffect)

// Constructor: called when the effect is first created / loaded
MyEffect::MyEffect()
{
    // Configure the UI and parameters
    parameters.add("Gain");

    // Add factory presets
    presets.add( "My Preset", 0.5f);

    // Initialise shared member variables, etc.
}

// Destructor: called when the effect is terminated / unloaded
MyEffect::~MyEffect()
{
    // Put your own additional clean up code here (e.g. free memory)
}

// EVENT HANDLERS: handle different user input (button presses, preset selection, drop menus)

void MyEffect::presetLoaded(int iPresetNum, const char *sPresetName)
{
    // A preset has been loaded, so you could perform setup, such as retrieving parameter values
    // using getParameter and use them to set state variables in the plugin
}

void MyEffect::optionChanged(int iOptionMenu, int iItem)
{
    // An option menu, with index iOptionMenu, has been changed to the entry, iItem
}

void MyEffect::buttonPressed(int iButton)
{
    // A button, with index iButton, has been pressed
}

// Applies audio processing to a buffer of audio
// (inputBuffer contains the input audio, and processed samples should be stored in outputBuffer)
void MyEffect::process(const float** inputBuffers, float** outputBuffers, int numSamples)
{
    float fIn0, fIn1, fOut0 = 0, fOut1 = 0;
    const float *pfInBuffer0 = inputBuffers[0], *pfInBuffer1 = inputBuffers[1];
    float *pfOutBuffer0 = outputBuffers[0], *pfOutBuffer1 = outputBuffers[1];
    
    const float fGain = parameters[0];
    
    while(numSamples--)
    {
        // Get sample from input
        fIn0 = *pfInBuffer0++;
        fIn1 = *pfInBuffer1++;
        
        // Add your effect processing here
        fOut0 = fIn0 * fGain;
        fOut1 = fIn1 * fGain;
       
        // Copy result to output
        *pfOutBuffer0++ = fOut0;
        *pfOutBuffer1++ = fOut1;
    }
}