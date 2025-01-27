/*
  ==============================================================================

    This file contains the basic framework code for a JUCE plugin processor.

  ==============================================================================
*/

#include "PluginProcessor.h"
#include "PluginEditor.h"

//==============================================================================
KlangSynthAudioProcessor::KlangSynthAudioProcessor()
#ifndef JucePlugin_PreferredChannelConfigurations
     : AudioProcessor (BusesProperties()
                     #if ! JucePlugin_IsMidiEffect
                      #if ! JucePlugin_IsSynth
                       .withInput  ("Input",  juce::AudioChannelSet::stereo(), true)
                      #endif
                       .withOutput ("Output", juce::AudioChannelSet::stereo(), true)
                     #endif
                       )
#endif
{
    // KLANG : register synth parameters
    for (unsigned int c = 0; c < subtractive.controls.size(); c++) {
        const klang::Control& control = subtractive.controls[c];
        switch (control.type) {
        case klang::Control::ROTARY:
        case klang::Control::SLIDER:
            addParameter(new juce::AudioParameterFloat(c, control.name.c_str(), control.min, control.max, control.initial));
            break;
        case klang::Control::BUTTON:
        case klang::Control::TOGGLE:
            addParameter(new juce::AudioParameterBool(c, control.name.c_str(), control.initial));
            break;
        case klang::Control::MENU:
        {   juce::StringArray choices;
            for (unsigned int o = 0; o < control.options.size(); o++)
                choices.add(control.options[o].c_str());
            addParameter(new juce::AudioParameterChoice(c, control.name.c_str(), choices, (int)control.initial));
            break;
        }
        }
    }
}

KlangSynthAudioProcessor::~KlangSynthAudioProcessor()
{
}

//==============================================================================
const juce::String KlangSynthAudioProcessor::getName() const
{
    return JucePlugin_Name;
}

bool KlangSynthAudioProcessor::acceptsMidi() const
{
   #if JucePlugin_WantsMidiInput
    return true;
   #else
    return false;
   #endif
}

bool KlangSynthAudioProcessor::producesMidi() const
{
   #if JucePlugin_ProducesMidiOutput
    return true;
   #else
    return false;
   #endif
}

bool KlangSynthAudioProcessor::isMidiEffect() const
{
   #if JucePlugin_IsMidiEffect
    return true;
   #else
    return false;
   #endif
}

double KlangSynthAudioProcessor::getTailLengthSeconds() const
{
    return 0.0;
}

int KlangSynthAudioProcessor::getNumPrograms()
{
    return 1;   // NB: some hosts don't cope very well if you tell them there are 0 programs,
                // so this should be at least 1, even if you're not really implementing programs.
}

int KlangSynthAudioProcessor::getCurrentProgram()
{
    return 0;
}

void KlangSynthAudioProcessor::setCurrentProgram (int index)
{
}

const juce::String KlangSynthAudioProcessor::getProgramName (int index)
{
    return {};
}

void KlangSynthAudioProcessor::changeProgramName (int index, const juce::String& newName)
{
}

//==============================================================================
void KlangSynthAudioProcessor::prepareToPlay (double sampleRate, int samplesPerBlock)
{
    // Use this method as the place to do any pre-playback
    // initialisation that you need..
}

void KlangSynthAudioProcessor::releaseResources()
{
    // When playback stops, you can use this as an opportunity to free up any
    // spare memory, etc.
}

#ifndef JucePlugin_PreferredChannelConfigurations
bool KlangSynthAudioProcessor::isBusesLayoutSupported (const BusesLayout& layouts) const
{
  #if JucePlugin_IsMidiEffect
    juce::ignoreUnused (layouts);
    return true;
  #else
    // This is the place where you check if the layout is supported.
    // In this template code we only support mono or stereo.
    // Some plugin hosts, such as certain GarageBand versions, will only
    // load plugins that support stereo bus layouts.
    if (layouts.getMainOutputChannelSet() != juce::AudioChannelSet::mono()
     && layouts.getMainOutputChannelSet() != juce::AudioChannelSet::stereo())
        return false;

    // This checks if the input layout matches the output layout
   #if ! JucePlugin_IsSynth
    if (layouts.getMainOutputChannelSet() != layouts.getMainInputChannelSet())
        return false;
   #endif

    return true;
  #endif
}
#endif

void KlangSynthAudioProcessor::processBlock (juce::AudioBuffer<float>& buffer, juce::MidiBuffer& midiMessages)
{
    juce::ScopedNoDenormals noDenormals;
    auto totalNumInputChannels  = getTotalNumInputChannels();
    auto totalNumOutputChannels = getTotalNumOutputChannels();

    // In case we have more outputs than inputs, this code clears any output
    // channels that didn't contain input data, (because these aren't
    // guaranteed to be empty - they may contain garbage).
    // This is here to avoid people getting screaming feedback
    // when they first compile a plugin, but obviously you don't need to keep
    // this code if your algorithm always overwrites all the output channels.
    for (auto i = totalNumInputChannels; i < totalNumOutputChannels; ++i)
        buffer.clear (i, 0, buffer.getNumSamples());

    // KLANG : pass MIDI to synth
    for (const auto& message : midiMessages)
        subtractive.input(message.data[0], message.data[1], message.data[2]);

    // KLANG : copy parameters to synth
    for (unsigned int c = 0; c < subtractive.controls.size(); c++)
        subtractive.controls[c].setNormalised(getParameters()[c]->getValue());

    // KLANG : generate + process audio
    subtractive.output((float**)buffer.getArrayOfWritePointers(), buffer.getNumSamples());

    // KLANG : copy parameters from synth (may have changed during process)
    for (unsigned int c = 0; c < subtractive.controls.size(); c++)
        getParameters()[c]->setValue(subtractive.controls[c].normalised());
}

//==============================================================================
bool KlangSynthAudioProcessor::hasEditor() const
{
    return true; // (change this to false if you choose to not supply an editor)
}

juce::AudioProcessorEditor* KlangSynthAudioProcessor::createEditor()
{
    return new KlangSynthAudioProcessorEditor (*this);
}

//==============================================================================
void KlangSynthAudioProcessor::getStateInformation (juce::MemoryBlock& destData)
{
    // You should use this method to store your parameters in the memory block.
    // You could do that either as raw data, or use the XML or ValueTree classes
    // as intermediaries to make it easy to save and load complex data.
}

void KlangSynthAudioProcessor::setStateInformation (const void* data, int sizeInBytes)
{
    // You should use this method to restore your parameters from this memory block,
    // whose contents will have been created by the getStateInformation() call.
}

//==============================================================================
// This creates new instances of the plugin..
juce::AudioProcessor* JUCE_CALLTYPE createPluginFilter()
{
    return new KlangSynthAudioProcessor();
}
