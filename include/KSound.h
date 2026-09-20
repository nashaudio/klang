//
//  KSound.h
//  KSound Plugin Header File
//
//  Internal adapter for Klang-based sound mini-plugin
//

#include "plugin.h"

#pragma once

class KSound : public MiniPlugin::Effect
{
public:
    template<class TYPE>
    KSound(TYPE* sound) : sound(sound) {
        parameters = sound->controls;
        parameters.groups = sound->controls.groups;
        presets = sound->presets;

        if (klang::graph.isActive())
            debug.graph = klang::graph;
        if (klang::debug.console.length)
            debug.console = &klang::debug.console;
    }

    void setSampleRate(float rate) override { sampleRate = rate; klang::fs = rate; }
    float getSampleRate() const override { return sampleRate; }
    int getNumChannels() const { return sound.isMono() ? 1 : 2; }

    void process(const float** inputBuffers, float** outputBuffers, int numSamples) override {
        klang::fs = sampleRate;
        // A Sound generates audio independently of host input.
        memset(outputBuffers[0], 0, numSamples * sizeof(float));
        memset(outputBuffers[1], 0, numSamples * sizeof(float));

        if (sound) {
            const int nbParameters = std::min(parameters.count, sound.controls());
            for (int p = 0; p < nbParameters; p++)
                sound.control(p) = parameters[p];

            // wrap in klang buffer
            klang::buffer buffers[] = {
                { outputBuffers[0], numSamples },
                { outputBuffers[1], numSamples },
            };

            klang::Debug::Session kdbg(nullptr, numSamples, klang::Debug::Buffer::Effect);

            // apply audio processing (stereo)
            if (sound.isMono()) {
                sound.mono->process(buffers[0]);
                buffers[1] = buffers[0];
            } else if (sound.isStereo()) {
                klang::stereo::buffer buffer = { buffers[0], buffers[1] };
                sound.stereo->process(buffer);
            }

            debug.reset();
            if (kdbg.hasAudio()) {
                debug.buffer = kdbg.getAudio();
                debug.size = numSamples;
            }
            if (klang::graph.isActive())
                debug.graph = klang::graph;
            if (klang::debug.console.length)
                debug.console = &klang::debug.console;

            for (int p = 0; p < nbParameters; p++)
                parameters[p].value = sound.control(p);
        }
    }

    void onControl(int parameter, float value) override {
        if (sound && parameter >= 0 && parameter < parameters.count) {
            parameters[parameter].value = std::max(parameters[parameter].min, std::min(parameters[parameter].max, value));
            sound.control(parameter) = parameters[parameter].value;
            if(sound.isMono())
                sound.mono->onControl(parameter, value);
            else
                sound.stereo->onControl(parameter, value);
        }
	}

    void onPreset(int preset) override {
		if (sound) {
			if(sound.isMono())
				sound.mono->onPreset(preset);
			else
				sound.stereo->onPreset(preset);
		}
	}

private:
    float sampleRate = klang::fs;
    struct Sound {
        Sound(klang::mono::Sound* mono) : mono(mono) { }
        Sound(klang::stereo::Sound* stereo) : stereo(stereo) { }
        ~Sound() {
            delete mono; delete stereo;
        }

        bool isMono() const { return mono; }
        bool isStereo() const { return stereo; }
        operator bool() const { return mono || stereo; }

        unsigned int controls() { return isMono() ? mono->controls.count : stereo->controls.count; }
        float& control(int index) { return isMono() ? mono->controls[index].value : stereo->controls[index].value; }

        klang::mono::Sound* mono = nullptr;
        klang::stereo::Sound* stereo = nullptr;
    } sound;
};

