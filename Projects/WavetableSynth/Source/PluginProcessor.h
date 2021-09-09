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
    WavetableOscillator(juce::AudioSampleBuffer);
    void setCurrentSampleRate(float);
    void init();
    void setFrequency(float);
    float getNextSample() noexcept;
    void changeWavetable(juce::AudioSampleBuffer&);
    
private:
    juce::AudioSampleBuffer wavetable;
    const int tableSize;
    float currentIndex = 0.0f, tableDelta = 0.0f, currentSampleRate;
};

struct SineWaveSound : public juce::SynthesiserSound {
    SineWaveSound() {}
    bool appliesToNote(int) override {
        return true;
    }
    bool appliesToChannel(int) override {
        return true;
    }
};

struct SineWaveVoice : public juce::SynthesiserVoice {
    
    SineWaveVoice() {
        initOscillators();
    }

    void initOscillators() {
        juce::AudioBuffer<float> wavetable;
        wavetable.setSize(1, TABLESIZE + 1);
        wavetable.clear();

        auto* samples = wavetable.getWritePointer(0);

        auto angleDelta = juce::MathConstants<float>::twoPi / (float)(TABLESIZE); // [2]
        auto currentAngle = 0.0;

        for (unsigned int i = 0; i < TABLESIZE; ++i)
        {
            samples[i] = (float)std::sin(currentAngle);
            currentAngle += angleDelta;
            if (currentAngle > juce::MathConstants<float>::twoPi)
                currentAngle -= juce::MathConstants<float>::twoPi;
        }
        samples[TABLESIZE] = samples[0];

        auto* oscillator = new WavetableOscillator(wavetable);
        oscillators.add(oscillator);
    }

    bool canPlaySound(juce::SynthesiserSound* sound) override
    {
        return dynamic_cast<SineWaveSound*> (sound) != nullptr;
    }

    void startNote(int midiNoteNumber, float velocity,
        juce::SynthesiserSound*, int /*currentPitchWheelPosition*/) override
    {
        level = velocity * 0.15;
        tailOff = 0.0;
        playNote = true;

        for (auto* oscillator : oscillators) {
            oscillator->init();
            oscillator->setCurrentSampleRate(getSampleRate());
            oscillator->setFrequency(juce::MidiMessage::getMidiNoteInHertz(midiNoteNumber));
        }
    }

    void stopNote(float /*velocity*/, bool allowTailOff) override
    {
        if (allowTailOff)
        {
            if (tailOff == 0.0)
                tailOff = 1.0;
        }
        else
        {
            clearCurrentNote();
            playNote = false;
        }
    }

    void pitchWheelMoved(int) override {}
    void controllerMoved(int, int) override {}

    void renderNextBlock(juce::AudioSampleBuffer& outputBuffer, int startSample, int numSamples) override {

        if (playNote) {
            if (tailOff > 0.0) {
                for (int i = startSample; i < startSample + numSamples; i++) {

                    for (auto* oscillator : oscillators) {

                        auto currentSample = oscillator->getNextSample() * level * tailOff;
                        for (auto j = outputBuffer.getNumChannels() - 1; j >= 0; j--)
                            outputBuffer.addSample(j, i, currentSample);
                    }

                    tailOff *= 0.99;

                    if (tailOff <= 0.005) {
                        clearCurrentNote();
                        playNote = false;
                        break;
                    }
                }
            }
            else
            {
                for (int i = startSample; i < startSample + numSamples; i++) {
                    for (auto* oscillator : oscillators) {
                        auto currentSample = oscillator->getNextSample() * level;

                        for (auto j = outputBuffer.getNumChannels() - 1; j >= 0; j--)
                            outputBuffer.addSample(j, i, currentSample);
                    }
                }
            }
        }
    }

private:
    juce::OwnedArray<WavetableOscillator> oscillators;
    double tailOff = 0.0f, level = 0.0f;
    bool playNote = false;
};

//==============================================================================
/**
*/
class WavetableSynthAudioProcessor  : public juce::AudioProcessor
{
public:
    //==============================================================================
    WavetableSynthAudioProcessor();
    ~WavetableSynthAudioProcessor() override;

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

    //==============================================================================
    //void createWavetables();
    //void initOscillators();

private:
    //==============================================================================

    /*std::function<float(float)> functLambdaArray[WAVETABLE_CNT] = {
        //sine wave
        [](float x) { return std::sin(x); },
        //square wave
        [](float x) { return x < juce::MathConstants<float>::pi ? 0 : 1; },
        //triangle wave
        [](float x) { return x > 3.0f * juce::MathConstants<float>::halfPi ?
                                4.0f - x / juce::MathConstants<float>::halfPi :
                             x > juce::MathConstants<float>::halfPi ?
                                x / juce::MathConstants<float>::halfPi - 2.0f :
                                -x / juce::MathConstants<float>::halfPi; },
        //sawtooth wave
        [](float x) { return x / juce::MathConstants<float>::pi - 1; },
        //silence
        [](float x) { return 0; }
    };*/

    //juce::AudioFormatManager formatManager;

    //juce::AudioBuffer<float> waveTables[WAVETABLE_CNT];
    //juce::AudioBuffer<float> wavetable;
    //juce::OwnedArray<WavetableOscillator> oscillators;

    juce::Synthesiser synth;

    JUCE_DECLARE_NON_COPYABLE_WITH_LEAK_DETECTOR (WavetableSynthAudioProcessor)
};
