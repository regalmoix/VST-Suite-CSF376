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
                       .withInput  ("Input",  AudioChannelSet::stereo(), true)
                      #endif
                       .withOutput ("Output", AudioChannelSet::stereo(), true)
                     #endif
                       )
#endif
{
    waveLambdas =
    {
        // Sine Wave
        [] (double x)
        {
            return std::sin(x);
        },                                
        
        // Square Wave
        [] (double x)
        {
            return x < MathConstants<float>::pi ? 0 : 1; 
        }, 

        // Triangle Wave
        [] (double x) 
        { 
            float  halfPi       = 1.0 * MathConstants<float>::halfPi;
            int    normX        = (int) (x / halfPi);

            switch (normX)
            {
                case 0 :    return -x / halfPi + 0.0;

                case 1 :    return +x / halfPi - 2.0;
                case 2 :    return +x / halfPi - 2.0;

                case 3 :    return -x / halfPi + 4.0;

                default:    return 0.0;
            }      

            return 0.0;             
        },
        
        // Saw Wave
        [] (double x) 
        { 
            return x / MathConstants<float>::pi - 1; 
        },

        // Silence
        [] (double x) 
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

void WavetableSynthAudioProcessor::processBlock (AudioBuffer<float>& buffer, MidiBuffer& midiMessages)
{
    synth.renderNextBlock( buffer, midiMessages, 0, buffer.getNumSamples() );
}

void WavetableSynthAudioProcessor::getStateInformation (MemoryBlock& destData)
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
    layout.add(std::make_unique<AudioParameterFloat>
        (
            "Attack",
            "Attack",
            NormalisableRange(0.0f, 10000.0f, 5.0f, skewFactor),
            10.0f
        )
    );

    // Adding Decay Slider with appropriate range and defaults
    layout.add(std::make_unique<AudioParameterFloat>
        (
            "Decay",
            "Decay",
            NormalisableRange(0.0f, 10000.0f, 5.0f, skewFactor),
            10.0f
        )
    );

    // Adding Sustain Slider with appropriate range and defaults
    layout.add(std::make_unique<AudioParameterFloat>
        (
            "Sustain",
            "Sustain",
            NormalisableRange(0.0f, 1.0f, 0.1f, 1.0f),
            0.0f
        )
    );

    // Adding Release Slider with appropriate range and defaults
    layout.add(std::make_unique<AudioParameterFloat>
        (
            "Release",
            "Release",
            NormalisableRange(0.0f, 10000.0f, 5.0f, skewFactor),
            10.0f
        )
    );
    StringArray wavetableChoices = {"Sine Wave", "Square Wave", "Triangle Wave", "Saw Wave", "Silence"};

    layout.add(std::make_unique<AudioParameterChoice>
        (
            "WaveTable Choice",
            "WaveTable Choice",
            wavetableChoices,
            0
        )
    );

    return layout;
}

std::function<double(double)> WavetableSynthAudioProcessor::getWaveFunction() const
{  
    int waveIndex = apvts.getRawParameterValue("WaveTable Choice")->load();
    DBG(waveIndex);
    return waveLambdas[waveIndex];
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
AudioProcessor* JUCE_CALLTYPE createPluginFilter()
{
    return new WavetableSynthAudioProcessor();
}
