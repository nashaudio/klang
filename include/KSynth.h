//
//  KEffect.h
//  KEffect Plugin Header File
//
//  Internal adapter for Klang-based effect mini-plugin
//

#include "plugin.h"
//#include "klang.h"

#pragma once

class KSynth : public MiniPlugin::Synth
{
public:
    template<class SYNTH>
    class Note : public MiniPlugin::Note
    {
    public:
        // constructor (initialise variables, etc.)
        Note(MiniPlugin::Synth* synth, typename SYNTH::Note* note) : MiniPlugin::Note(synth), note(note) {
            parameters = synth->parameters;
            presets = synth->presets;
        }
        ~Note() { delete note; } // destructor (clean up, free memory, etc.)

        KSynth* getSynth() { return (KSynth*)synth; }
    
        void onStartNote(int pitch, float velocity) {
            if (note) {
                note->start(pitch, velocity);

                const int nbParameters = std::min(parameters.count, note->controls.size());
                for (int p = 0; p < nbParameters; p++)
                    parameters[p].set(note->controls[p]);
            }
        }
        bool onStopNote(float velocity) {
            if (note)
                return note->release(velocity);
            return true;
        }

        void onControl(int param, float value) {
            if (note)
				note->onControl(param, value);
        }
        void onPreset(int preset) {
            if (note)
                note->onPreset(preset);
        }

        void onPitchWheel (int value) { }
        void onControlChange(int controller, int value) {
            if (note)
                note->controlChange(controller, value);
        }
    
        bool process(float** outputBuffers, int numChannels, int numSamples) {
            for(int c=0; c<numChannels; c++)
                memset(outputBuffers[c], 0, numSamples * sizeof(float));

            bool bContinue = false;
            if (note) {
                const int nbParameters = std::min(parameters.count, note->controls.size());
                for(int p=0; p<nbParameters; p++)
                    note->controls[p] = parameters[p];

                // wrap in klang buffer
                klang::buffer buffer[] = {
                    { outputBuffers[0], numSamples },
                    { outputBuffers[1], numSamples },
                };

                // debug buffer
                klang::Debug::Session debug(outputBuffers[numChannels], numSamples, klang::Debug::Buffer::Notes);

                // apply audio processing
                bContinue = note->process(buffer);
                if(note->out.channels() == 1)
                    buffer[1] = buffer[0];

                for (int p = 0; p < nbParameters; p++)
                    parameters[p].set(note->controls[p]);
            }
            return bContinue;
        }

        void setSampleRate(float sampleRate){ klang::fs = sampleRate; }
        float getSampleRate() const { return klang::fs; };
       
        void presetLoaded(int iPresetNum, const char *sPresetName) { }
        void optionChanged(int iOptionMenu, int iItem) { }
        void buttonPressed(int iButton) { }

    private:
        // Declare shared member variables here
        typename SYNTH::Note* note;
    };

    template<class TYPE>
    KSynth(TYPE* synth) : synth(synth) {
        for (int n = 0; n < 128; n++) // create synthesiser's notes
            notes[n] = new Note<TYPE>(this, synth->notes[n]);

        parameters = synth->controls;
        parameters.groups = synth->controls.groups;
        presets = synth->presets;

        if (klang::graph->isActive())
            debug.graph = klang::graph;
        if (klang::debug.console.length)
            debug.console = &klang::debug.console;
    }

    void setSampleRate(float sampleRate){ klang::fs = sampleRate; }
    float getSampleRate() const { return klang::fs; };
    
    void process(const float** inputBuffers, float** outputBuffers, int numSamples) { 
        // copy input to output for in-place/replacing processing
        memcpy(outputBuffers[0], inputBuffers[0], numSamples * sizeof(float));
        memcpy(outputBuffers[1], inputBuffers[1], numSamples * sizeof(float));

        if (synth) {
            const int nbParameters = std::min(parameters.count, synth.controls());
            for(int p=0; p<nbParameters; p++)
                synth.control(p) = parameters[p];

            // wrap in klang buffer
            klang::mono::buffer buffers[] = {
                { outputBuffers[0], numSamples },
                { outputBuffers[1], numSamples },
            };

            klang::Debug::Session kdbg(nullptr, numSamples, klang::Debug::Buffer::Synth);

            // apply audio processing
            if (synth.isMono()) {
                synth.mono->process(buffers[0]);
                buffers[1] = buffers[0];
            } else {
                klang::stereo::buffer buffer = { buffers[0], buffers[1] };
                synth.stereo->process(buffer);
            }

            debug.reset();
            if (kdbg.hasAudio()) {
                debug.buffer = kdbg.getAudio();
                debug.size = numSamples;
            }
            if (klang::graph->isActive())
                debug.graph = klang::graph.ptr.get();
            if (klang::debug.console.length)
                debug.console = &klang::debug.console;

            for (int p = 0; p < nbParameters; p++)
                parameters[p].set(synth.control(p));
        }
    }

    void onControl(int parameter, float value) {
        if (synth) {
            if (synth.isMono())
                synth.mono->onControl(parameter, value);
            else
                synth.stereo->onControl(parameter, value);
        }
    }

    void onPreset(int preset) {
        if (synth) {
            if (synth.isMono())
                synth.mono->onPreset(preset);
            else
                synth.stereo->onPreset(preset);
        }
    }

    //void presetLoaded(int iPresetNum, const char *sPresetName) { }
    //void optionChanged(int iOptionMenu, int iItem) { }
    //void buttonPressed(int iButton) { }

private:
    // Class to accomodate mono/stereo versions of the synth
    struct Synth {
        Synth(klang::mono::Synth* mono) : mono(mono) { }
        Synth(klang::stereo::Synth* stereo) : stereo(stereo) { }
        ~Synth() { delete mono; delete stereo; }

        bool isMono() const { return mono; }
        bool isStereo() const { return stereo; }
        operator bool() const { return mono || stereo; }

        unsigned int controls() { return isMono() ? mono->controls.count : stereo->controls.count; }
        float& control(int index) { return isMono() ? mono->controls[index].value : stereo->controls[index].value; }

        klang::mono::Synth* mono = nullptr;
        klang::stereo::Synth* stereo = nullptr;
    } synth;
};

