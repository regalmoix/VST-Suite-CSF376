/*
  ==============================================================================

    This file contains the basic framework code for a JUCE plugin processor.

  ==============================================================================
*/

#include "PluginProcessor.h"
#include "PluginEditor.h"

WavetableSynthAudioProcessor::WavetableSynthAudioProcessor()
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
    waveLambdas =
    {
        // Sine Wave
        [] (float x)
        {
            return std::sin(x);
        },                                
        
        // Square Wave
        [] (float x)
        {
            return x < juce::MathConstants<float>::pi ? 0 : 1; 
        }, 

        // Triangle Wave
        [] (float x) 
        { 
            float  halfPi       = 1.0f * MathConstants<float>::halfPi;
            int    normX        = (int) (x / halfPi);

            switch (normX)
            {
                case 0 :    return -x / halfPi + 0.0f;

                case 1 :    return +x / halfPi - 2.0f;
                case 2 :    return +x / halfPi - 2.0f;

                case 3 :    return -x / halfPi + 4.0f;

                default:    return 0.0f;
            }      

            return 0.0f;             
        },
        
        // Saw Wave
        [] (float x) 
        { 
            return x / MathConstants<float>::pi - 1; 
        },

        // Silence
        [] (float x) 
        { 
            return 0; 
        }
    };

    for (auto i = 0; i < 10; ++i)
        synth.addVoice(new SineWaveVoice(*this));

    synth.addSound(new SineWaveSound());
}

WavetableSynthAudioProcessor::~WavetableSynthAudioProcessor() {}

//==============================================================================

void WavetableSynthAudioProcessor::prepareToPlay (double sampleRate, int samplesPerBlock)
{
    synth.setCurrentPlaybackSampleRate(sampleRate);
}

void WavetableSynthAudioProcessor::releaseResources()
{
    // When playback stops, you can use this as an opportunity to free up any
    // spare memory, etc.
}

void WavetableSynthAudioProcessor::processBlock (juce::AudioBuffer<float>& buffer, juce::MidiBuffer& midiMessages)
{
    synth.renderNextBlock( buffer, midiMessages, 0, buffer.getNumSamples() );
}

void WavetableSynthAudioProcessor::getStateInformation (juce::MemoryBlock& destData)
{
    // You should use this method to store your parameters in the memory block.
    // You could do that either as raw data, or use the XML or ValueTree classes
    // as intermediaries to make it easy to save and load complex data.
}

void WavetableSynthAudioProcessor::setStateInformation (const void* data, int sizeInBytes)
{
    // You should use this method to restore your parameters from this memory block,
    // whose contents will have been created by the getStateInformation() call.
}

//==============================================================================

AudioProcessorValueTreeState::ParameterLayout WavetableSynthAudioProcessor::createParameterLayout()
{
    AudioProcessorValueTreeState::ParameterLayout layout;

    float skewFactor = 0.4;

    // Adding Attack Slider with appropriate range and defaults
    layout.add(std::make_unique<juce::AudioParameterFloat>
        (
            "Attack",
            "Attack",
            juce::NormalisableRange(0.0f, 10000.0f, 5.0f, skewFactor),
            10.0f
        )
    );

    // Adding Decay Slider with appropriate range and defaults
    layout.add(std::make_unique<juce::AudioParameterFloat>
        (
            "Decay",
            "Decay",
            juce::NormalisableRange(0.0f, 10000.0f, 5.0f, skewFactor),
            10.0f
        )
    );

    // Adding Sustain Slider with appropriate range and defaults
    layout.add(std::make_unique<juce::AudioParameterFloat>
        (
            "Sustain",
            "Sustain",
            juce::NormalisableRange(0.0f, 1.0f, 0.1f, 1.0f),
            0.0f
        )
    );

    // Adding Release Slider with appropriate range and defaults
    layout.add(std::make_unique<juce::AudioParameterFloat>
        (
            "Release",
            "Release",
            juce::NormalisableRange(0.0f, 10000.0f, 5.0f, skewFactor),
            10.0f
        )
    );

    return layout;
}

ADSRSettings getADSRSettings (const AudioProcessorValueTreeState& apvts, const double sampleRate)
{
    //   1 msec  : (r / 1000)     samples
    //   x msec  : x * (r / 1000) samples

    ADSRSettings settings;
    const double samplesPerMilliSecond = sampleRate / 1000.0;

    settings.attackDuration     = apvts.getRawParameterValue("Attack") ->load() * samplesPerMilliSecond;
    settings.decayDuration      = apvts.getRawParameterValue("Decay")  ->load() * samplesPerMilliSecond;
    settings.sutainGain         = apvts.getRawParameterValue("Sustain")->load();
    settings.releaseDuration    = apvts.getRawParameterValue("Release")->load() * samplesPerMilliSecond;

    return settings;
}

//==============================================================================
juce::AudioProcessor* JUCE_CALLTYPE createPluginFilter()
{
    return new WavetableSynthAudioProcessor();
}
