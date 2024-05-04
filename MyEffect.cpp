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
    parameters.add("Choo Choo");
    parameters.add("Rate", Parameter::ROTARY, 1.0, 100.0,10.0, Automatic);
	parameters.add("Output Gain");
	parameters.add("Another");
	//parameters += Menu("Main", "One", "Two");


    // Add factory presets
    presets.add( "Preset 3", 0.f, 1.f, 2.f, 3.f, 4.f, 5.f, 6.f, 7.f, 8.f, 9.f );
    presets.add( "Preset 4", 10.f, 11.f, 12.f, 13.f, 14.f, 15.f, 16.f, 17.f, 18.f, 19.f );
    presets.add( "Preset 5", 20.f, 21.f, 22.f, 23.f, 24.f, 25.f, 26.f, 27.f, 28.f, 29.f );

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
    
    float fGain = parameters[0];
	
	double value = 0.1f;
	
	lfo.setFrequency(parameters[2]);
	
	char *ptr = NULL;
	//char b = ptr[1] / *ptr;
	
    
    while(numSamples--)
    {
        // Get sample from input
        fIn0 = *pfInBuffer0++;
        fIn1 = *pfInBuffer1++;
        
        float fNoise = parameters[1] * noise.tick();

        // Add your effect processing here
        fOut0 = fGain * fIn0 + fNoise * 0.1;
        fOut1 = fGain * fIn1 + fNoise * 0.1;

		float fMod = lfo.tick() * 0.5 + 0.5;
		
		fMod *= parameters[3];

		fOut0 *= fMod;
		fOut1 *= fMod;
        
        // Copy result to output
        *pfOutBuffer0++ = fOut0;
        *pfOutBuffer1++ = fOut1;
    }
}