/*
  ==============================================================================

    This file contains the basic framework code for a JUCE plugin processor.

  ==============================================================================
*/

#pragma once

#include <JuceHeader.h>

#include "voices.h"

//==============================================================================
/**
*/
class NoteSliderAudioProcessor  : public juce::AudioProcessor
{
public:
    //==============================================================================
    NoteSliderAudioProcessor();
    ~NoteSliderAudioProcessor() override;

    //==============================================================================
    void prepareToPlay (double sampleRate, int samplesPerBlock) override;
    void releaseResources() override;

   #ifndef JucePlugin_PreferredChannelConfigurations
    bool isBusesLayoutSupported (const BusesLayout& layouts) const override;
   #endif

    void processBlock (juce::AudioBuffer<float>&, juce::MidiBuffer&) override;

    //==============================================================================
    juce::AudioProcessorEditor* createEditor() override;
    bool hasEditor() const override;

    //==============================================================================
    const juce::String getName() const override;

    bool acceptsMidi() const override;
    bool producesMidi() const override;
    bool isMidiEffect() const override;
    double getTailLengthSeconds() const override;

    //==============================================================================
    int getNumPrograms() override;
    int getCurrentProgram() override;
    void setCurrentProgram (int index) override;
    const juce::String getProgramName (int index) override;
    void changeProgramName (int index, const juce::String& newName) override;

    //==============================================================================
    void getStateInformation (juce::MemoryBlock& destData) override;
    void setStateInformation (const void* data, int sizeInBytes) override;

    void update_pingpong(){
        double pp_a, pp_b;
        pp_b = speeds->pp_omega * speeds->pp_omega / currSamplingRate;
        pp_a = 2 * speeds->pp_zeta * speeds->pp_omega / currSamplingRate;
        speeds->pp_K1 = pp_b / currSamplingRate;
        speeds->pp_K2 = 2.0f - pp_a - speeds->pp_K1;
        speeds->pp_K3 = pp_a - 1;
    }

    double currSamplingRate = 44100.0f;

    juce::Synthesiser synth;

    //these will be declared on the heap
    struct SpeedValues* speeds;
    SineWaveVoice* synthVoices[5];
    SineWaveSound* sineSound;

private:

    //==============================================================================
    JUCE_DECLARE_NON_COPYABLE_WITH_LEAK_DETECTOR (NoteSliderAudioProcessor)
};
