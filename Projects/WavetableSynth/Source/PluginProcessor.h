/*
  ==============================================================================

    This file contains the basic framework code for a JUCE plugin processor.

  ==============================================================================
*/

#pragma once

#include <JuceHeader.h>
#include <algorithm>
#define WAVETABLE_CNT 5
#define OSC_CNT 3
#define TABLESIZE 44100

class WavetableOscillator
{
public:
    WavetableOscillator(AudioSampleBuffer);

    void    setCurrentSampleRate (float sampleRate);
    void    init ();
    void    setFrequency (float frequency);
    float   getNextSample () noexcept;
    void    changeWavetable (AudioSampleBuffer& newWavetableToUse);
    
private:
    AudioSampleBuffer   wavetable;
    const int           tableSize;
    float               currentIndex    { 0.0f };
    float               tableDelta      { 0.0f };
    float               currentSampleRate;

    void    updateIndex();
};

class SineWaveSound : public juce::SynthesiserSound 
{
public:
    bool    appliesToNote (int midiNoteNumber) override;
    bool    appliesToChannel (int midiChannel) override;
};

class SineWaveVoice : public juce::SynthesiserVoice 
{
private:
    juce::OwnedArray<WavetableOscillator> oscillators;

    double  tailOff     { 0.0f };
    double  level       { 0.0f };
    bool    playNote    { false };

public:
    SineWaveVoice ();

    ~SineWaveVoice();

    void initOscillators();

    bool canPlaySound(juce::SynthesiserSound* sound) override;

    void startNote (int midiNoteNumber, float velocity, juce::SynthesiserSound*, int currentPitchWheelPosition) override;
    void stopNote  (float velocity, bool allowTailOff) override;

    void pitchWheelMoved (int newPitchWheelValue) override;
    void controllerMoved (int controllerNumber, int newControllerValue) override;

    void renderNextBlock (juce::AudioSampleBuffer& outputBuffer, int startSample, int numSamples) override;
};

class WavetableSynthAudioProcessor  : public juce::AudioProcessor
{
public:
    //==============================================================================
    WavetableSynthAudioProcessor ();
    ~WavetableSynthAudioProcessor() override;

    //==============================================================================
    void prepareToPlay      (double sampleRate, int samplesPerBlock)    override;
    void releaseResources   ()                                          override;

    //==============================================================================
#ifndef JucePlugin_PreferredChannelConfigurations
    bool isBusesLayoutSupported (const BusesLayout& layouts) const override;
#endif

    //==============================================================================
    void processBlock (juce::AudioBuffer<float>&, juce::MidiBuffer&) override;

    //==============================================================================
    AudioProcessorEditor* createEditor()    override;
    bool            hasEditor()       const override;

    //==============================================================================
    const String    getName()       const override;
    bool            acceptsMidi()   const override;
    bool            producesMidi()  const override;
    bool            isMidiEffect()  const override;
    double          getTailLengthSeconds() const override;

    //==============================================================================
    int     getNumPrograms()    override;
    int     getCurrentProgram() override;
    void    setCurrentProgram (int index)       override;
    const   String getProgramName (int index)   override;
    void    changeProgramName (int index, const String& newName) override;

    //==============================================================================
    void getStateInformation (juce::MemoryBlock& destData)          override;
    void setStateInformation (const void* data, int sizeInBytes)    override;

    //==============================================================================
    //void createWavetables();
    //void initOscillators();

private:
    //==============================================================================
    juce::Synthesiser synth;
    std::array<std::function<float(float)>, WAVETABLE_CNT> waveLambdas;


    //juce::AudioFormatManager formatManager;
    //juce::AudioBuffer<float> waveTables[WAVETABLE_CNT];
    //juce::AudioBuffer<float> wavetable;
    //juce::OwnedArray<WavetableOscillator> oscillators;


    JUCE_DECLARE_NON_COPYABLE_WITH_LEAK_DETECTOR (WavetableSynthAudioProcessor)
};
