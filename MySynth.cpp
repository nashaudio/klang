//
//  SynthPlugin.cpp
//  MySynth Plugin Source Code - for the top-level synthesiser object
//
//  Used to define the bodies of functions used by the plugin, as declared in SynthPlugin.h.
//

#include "MySynth.h"
#include "MySynthNote.h"

////////////////////////////////////////////////////////////////////////////
// SYNTH - represents the whole synthesis plugin
////////////////////////////////////////////////////////////////////////////

SYNTH(MySynth)

// Constructor: called when the synth plugin is first created / loaded
MySynth::MySynth()
: Synth()
{
    for(int n=0; n<32; n++) // create synthesiser's notes
        notes[n] = new MySynthNote(this);
    
    // Initialise synthesier variables, etc.
	parameters.add("Gain", Parameter::ROTARY, 0, 1, 0.5);
}

// Destructor: called when the synthesiser is terminated / unloaded
MySynth::~MySynth()
{
    // Put your own additional clean up code here (e.g. free memory)
    
    for(int n=0; n<32; n++) // delete synthesiser's notes
        delete notes[n];
}

// EVENT HANDLERS: handle different user input (button presses, preset selection, drop menus)

void MySynth::presetLoaded(int iPresetNum, const char *sPresetName)
{
    // A preset has been loaded, so you could perform setup, such as retrieving parameter values
    // using getParameter and use them to set state variables in the plugin
}

void MySynth::optionChanged(int iOptionMenu, int iItem)
{
    // An option menu, with index iOptionMenu, has been changed to the entry, iItem
}

void MySynth::buttonPressed(int iButton)
{
    // A button, with index iButton, has been pressed
}

// Applies audio post-processing to a buffer of audio
// (inputBuffer contains the input audio, and processed samples should be stored in outputBuffer)
void MySynth::process(const float** inputBuffers, float** outputBuffers, int numSamples)
{
    float fIn0, fIn1, fOut0 = 0, fOut1 = 0;
    const float *pfInBuffer0 = inputBuffers[0], *pfInBuffer1 = inputBuffers[1];
    float *pfOutBuffer0 = outputBuffers[0], *pfOutBuffer1 = outputBuffers[1];
      
    while(numSamples--)
    {
        // Get sample from input
        fIn0 = *pfInBuffer0++;
        fIn1 = *pfInBuffer1++;
        
        // Add your effect processing here
        fOut0 = fIn0;
        fOut1 = fIn1;
        
        // Copy result to output
        *pfOutBuffer0++ = fOut0;
        *pfOutBuffer1++ = fOut1;
    }
}
