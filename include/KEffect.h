//
//  KEffect.h
//  KEffect Plugin Header File
//
//  Internal adapter for Klang-based effect mini-plugin
//

#include "plugin.h"
//#include "dsp.h"
//#include "klang.h"

#pragma once

class KEffect : public MiniPlugin::Effect
{
public:
    template<class TYPE>
    KEffect(TYPE* effect) : effect(effect) {
        parameters = effect->controls;
        parameters.groups = effect->controls.groups;
        presets = effect->presets;

        if (klang::graph.isActive())
            debug.graph = klang::graph;
        if (klang::debug.console.length)
            debug.console = &klang::debug.console;
    }

    void setSampleRate(float sampleRate) override { klang::fs = sampleRate; }
    float getSampleRate() const override { return klang::fs; };
    
    void process(const float** inputBuffers, float** outputBuffers, int numSamples) override { 
        // copy input to output for in-place/replacing processing
        memcpy(outputBuffers[0], inputBuffers[0], numSamples * sizeof(float));
        memcpy(outputBuffers[1], inputBuffers[1], numSamples * sizeof(float));

        if (effect) {
            const int nbParameters = std::min(parameters.count, effect.controls());
            for (int p = 0; p < nbParameters; p++)
                effect.control(p) = parameters[p];

            // wrap in klang buffer
            klang::buffer buffers[] = {
                { outputBuffers[0], numSamples },
                { outputBuffers[1], numSamples },
            };

            klang::Debug::Session kdbg(nullptr, numSamples, klang::Debug::Buffer::Effect);

            // apply audio processing (stereo)
            if (effect.isMono()) {
                effect.mono->process(buffers[0]);
                buffers[1] = buffers[0];
            } else if (effect.isStereo()) {
                klang::stereo::buffer buffer = { buffers[0], buffers[1] };
                effect.stereo->process(buffer);
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
                parameters[p].value = effect.control(p);
        }
    }

    void onControl(int parameter, float value) override {
        if (effect) {
            if(effect.isMono())
                effect.mono->onControl(parameter, value);
            else
                effect.stereo->onControl(parameter, value);
        }
	}

    void onPreset(int preset) override {
		if (effect) {
			if(effect.isMono())
				effect.mono->onPreset(preset);
			else
				effect.stereo->onPreset(preset);
		}
	}

private:
    struct Effect {
        Effect(klang::mono::Effect* mono) : mono(mono) { }
        Effect(klang::stereo::Effect* stereo) : stereo(stereo) { }
        ~Effect() {
            delete mono; delete stereo;
        }

        bool isMono() const { return mono; }
        bool isStereo() const { return stereo; }
        operator bool() const { return mono || stereo; }

        unsigned int controls() { return isMono() ? mono->controls.count : stereo->controls.count; }
        float& control(int index) { return isMono() ? mono->controls[index].value : stereo->controls[index].value; }

        klang::mono::Effect* mono = nullptr;
        klang::stereo::Effect* stereo = nullptr;
    } effect;
};

//#define KEFFECT(name)                                                                                                       \
//extern "C" {                                                                                                                \
//    PTR_FUNCTION effectCreate(float sampleRate) {                                                                           \
//        klang::fs = sampleRate;                                                                                             \
//        return (MiniPlugin::Effect*)new KEffect(new name());                                                                      \
//    }                                                                                                                       \
//                                                                                                                            \
//    BACKGROUND_FUNCTION getBackground(void** data, int* size) {                                                             \
//        *data = (void*)Background::data;                                                                                    \
//        *size = Background::size;                                                                                           \
//        return true;                                                                                                        \
//    }                                                                                                                       \
//                                                                                                                            \
//    DESTROY_FUNCTION effectDestroy(void* effect) {                                                                          \
//        delete (MiniPlugin::Effect*)effect;                                                                                       \
//    }                                                                                                                       \
//                                                                                                                            \
//    PROCESS_FUNCTION effectProcess(void* effect, const float** inputBuffers, float** outputBuffers, int numSamples) {       \
//        try { ((MiniPlugin::Effect*)effect)->process(inputBuffers, outputBuffers, numSamples); return 0;                          \
//        } catch(...) { return 1; }                                                                                          \
//    }                                                                                                                       \
//}
