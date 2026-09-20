//
//  MySynthNote.cpp
//  MySynth Plugin Source Code - for individual notes
//
//  Used to define the bodies of functions used by the plugin, as declared in SynthPlugin.h.
//

#include "MySynthNote.h"

//================================================================================
// MyNote - object representing a single note (within the synthesiser - see above)
//================================================================================

// Triggered when a note is started (use to initialise / prepare note processing)
void MySynthNote::onStartNote(int pitch, float velocity)
{
    // convert note number to fundamental frequency (Hz)
    fFrequency = 440.f * pow(2.f, (pitch - 69.f) / 12.f);
    fVelocity = parameters[0];  // store velocity

    signalGenerator.reset();
    signalGenerator.setFrequency(fFrequency);
}
    
// Triggered when a note is stopped (return false to keep the note alive)
bool MySynthNote::onStopNote (float velocity){
    return true;
}

void MySynthNote::onPitchWheel (int value){

}
 
void MySynthNote::onControlChange (int controller, int value){
    
}
    
// Called to render the note's next buffer of audio (generates the sound)
// (return false to terminate the note)
bool MySynthNote::process (float** outputBuffer, int numChannels, int numSamples)
{
    float fMix = 0;
    float* pfOutBuffer0 = outputBuffer[0], *pfOutBuffer1 = outputBuffer[1];
    
    float fGain = parameters[0];
    while(numSamples--)
    {
        fMix = signalGenerator.tick();
        fMix *= fGain;      // apply gain
        fMix *= fVelocity; // apply velocity
        
        *pfOutBuffer0++ = fMix;
        *pfOutBuffer1++ = fMix;
    }
    
    return true;
}
