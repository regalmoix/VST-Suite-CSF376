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
        synth.addVoice(new SineWaveVoice());

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

juce::AudioProcessor* JUCE_CALLTYPE createPluginFilter()
{
    return new WavetableSynthAudioProcessor();
}
