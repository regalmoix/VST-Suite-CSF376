/*
  ==============================================================================

   This file is part of the JUCE tutorials.
   Copyright (c) 2020 - Raw Material Software Limited

   The code included in this file is provided under the terms of the ISC license
   http://www.isc.org/downloads/software-support-policy/isc-license. Permission
   To use, copy, modify, and/or distribute this software for any purpose with or
   without fee is hereby granted provided that the above copyright notice and
   this permission notice appear in all copies.

   THE SOFTWARE IS PROVIDED "AS IS" WITHOUT ANY WARRANTY, AND ALL WARRANTIES,
   WHETHER EXPRESSED OR IMPLIED, INCLUDING MERCHANTABILITY AND FITNESS FOR
   PURPOSE, ARE DISCLAIMED.

  ==============================================================================
*/

/*******************************************************************************
 The block below describes the properties of this PIP. A PIP is a short snippet
 of code that can be read by the Projucer and used to generate a JUCE project.

 BEGIN_JUCE_PIP_METADATA

 name:             WavetableSynthTutorial
 version:          1.0.0
 vendor:           JUCE
 website:          http://juce.com
 description:      Wavetable synthesiser.

 dependencies:     juce_audio_basics, juce_audio_devices, juce_audio_formats,
                   juce_audio_processors, juce_audio_utils, juce_core,
                   juce_data_structures, juce_events, juce_graphics,
                   juce_gui_basics, juce_gui_extra
 exporters:        xcode_mac, vs2019, linux_make

 type:             Component
 mainClass:        MainContentComponent

 useLocalCopy:     1

 END_JUCE_PIP_METADATA

*******************************************************************************/


#pragma once

#define freqRangeLeft 30.0f
#define freqRangeRight 5000.0f

#include <algorithm>

//==============================================================================
class WavetableOscillator
{
public:
    WavetableOscillator(const juce::AudioSampleBuffer& wavetableToUse)
        : wavetable(wavetableToUse),
        tableSize(wavetable.getNumSamples() - 1)
    {
        jassert(wavetable.getNumChannels() == 1);
    }

    void setCurrentSampleRate(float sampleRate) {
        currentSampleRate = sampleRate;
    }

    void setFrequency(float frequency)
    {
        auto tableSizeOverSampleRate = (float)tableSize / currentSampleRate;
        tableDelta = frequency * tableSizeOverSampleRate;
    }

    forcedinline float getNextSample() noexcept
    {
        auto index0 = (unsigned int)currentIndex;
        auto index1 = index0 + 1;

        auto frac = currentIndex - (float)index0;

        auto* table = wavetable.getReadPointer(0);
        auto value0 = table[index0];
        auto value1 = table[index1];

        auto currentSample = value0 + frac * (value1 - value0);

        if ((currentIndex += tableDelta) > (float)tableSize)
            currentIndex -= (float)tableSize;

        return currentSample;
    }

private:
    const juce::AudioSampleBuffer& wavetable;
    const int tableSize;
    float currentIndex = 0.0f, tableDelta = 0.0f, currentSampleRate;
};

//==============================================================================
class MainContentComponent : public juce::AudioAppComponent,
    public juce::Timer
{
public:
    MainContentComponent()
    {
        cpuUsageLabel.setText("CPU Usage", juce::dontSendNotification);
        cpuUsageText.setJustificationType(juce::Justification::right);
        addAndMakeVisible(cpuUsageLabel);
        addAndMakeVisible(cpuUsageText);

        freqSlider.setSliderStyle(juce::Slider::SliderStyle::LinearHorizontal);
        freqSlider.setTextBoxStyle(juce::Slider::TextEntryBoxPosition::TextBoxLeft, true, 50, 50);
        freqSlider.setRange(freqRangeLeft, freqRangeRight, 0.01f);
        freqSlider.setValue(100.0f, juce::dontSendNotification);
        freqSlider.onValueChange = [this] {
            for (auto oscillatorIndex = 0; oscillatorIndex < oscillators.size(); ++oscillatorIndex)
            {
                auto* oscillator = oscillators.getUnchecked(oscillatorIndex);
                oscillator->setFrequency((float)freqSlider.getValue());
            }
        };

        addAndMakeVisible(freqSlider);

        waveChooser.setEditableText(false);
        waveChooser.setJustificationType(juce::Justification::centred);
        waveChooser.addItemList({ "Sine", "Square", "Triangle", "Saw" }, 1);
        waveChooser.onChange = [this] {
            createWavetable(waveChooser.getSelectedId() - 1);
        };
        addAndMakeVisible(waveChooser);

        freqLabel.setText("Frequency", juce::NotificationType::dontSendNotification);
        addAndMakeVisible(freqLabel);

        createWavetable(0);

        setSize(700, 300);
        setAudioChannels(0, 2); // no inputs, two outputs
        startTimer(50);
    }

    ~MainContentComponent() override
    {
        shutdownAudio();
    }

    void resized() override
    {
        cpuUsageLabel.setBounds(10, 10, getWidth() - 20, 20);
        cpuUsageText.setBounds(10, 10, getWidth() - 20, 20);

        freqSlider.setBounds( getWidth()/2 - 200, getHeight()/4 - 50, 400, 100 );
        freqLabel.setBounds(getWidth() / 2 - 300, getHeight() / 4 - 50, 100, 100);

        waveChooser.setBounds(getWidth() / 2 - 200, 3*getHeight() / 4 - 50, 400, 100);
    }

    void timerCallback() override
    {
        auto cpu = deviceManager.getCpuUsage() * 100;
        cpuUsageText.setText(juce::String(cpu, 6) + " %", juce::dontSendNotification);
    }

    void createWavetable(int waveType)
    {
        sineTable.setSize(1, (int)tableSize + 1);
        sineTable.clear();

        auto* samples = sineTable.getWritePointer(0);

        //float a_var = 0.816055;
        //const b_var = 7.0f;

        //int harmonics[] = { 1, 3, 5, 6, 7, 9, 13, 15 };
        //float harmonicWeights[] = { 0.5f, 0.1f, 0.05f, 0.125f, 0.09f, 0.005f, 0.002f, 0.001f };     // [1]
        int harmonics[] = { 1 };
        float harmonicWeights[] = { 1.0f };

        //harmonics and weights for the Weierstrass Function, expressed as a sum of cos terms
        //uncomment to use, but remember that it sounds like shit

        //float harmonics[] = { 0.5f, 3.5f, 24.5f, 171.5f, 1200.5f, 8403.5f, 58824.5 };
        //float harmonicWeights[] = { 1.0f, 0.8160556f, 0.6659467f, 0.5434495f, 0.443485f, 0.3619084f, 0.2953374f };

        jassert(juce::numElementsInArray(harmonics) == juce::numElementsInArray(harmonicWeights));

        for (auto harmonic = 0; harmonic < juce::numElementsInArray(harmonics); ++harmonic)
        {
            auto angleDelta = juce::MathConstants<float>::twoPi / (float)(tableSize - 1) * harmonics[harmonic]; // [2]
            auto currentAngle = 0.0;

            for (unsigned int i = 0; i < tableSize; ++i)
            {
                //auto sample = std::sin(currentAngle);
                auto sample = functLambdaArray[waveType](currentAngle);
                samples[i] += (float)sample * harmonicWeights[harmonic];                           // [3]
                currentAngle += angleDelta;
                if (currentAngle > juce::MathConstants<float>::twoPi)
                    currentAngle -= juce::MathConstants<float>::twoPi;
            }
        }

        samples[tableSize] = samples[0];
    }

    void prepareToPlay(int, double sampleRate) override
    {
        auto numberOfOscillators = 1;

        for (auto i = 0; i < numberOfOscillators; ++i)
        {
            auto* oscillator = new WavetableOscillator(sineTable);

            //auto midiNote = juce::Random::getSystemRandom().nextDouble() * 36.0 + 48.0;
            //auto frequency = 440.0 * pow(2.0, (midiNote - 69.0) / 12.0);
            auto frequency = juce::Random::getSystemRandom().nextDouble() * (freqRangeRight - freqRangeLeft) + freqRangeLeft;

            oscillator->setCurrentSampleRate((float)sampleRate);
            oscillator->setFrequency((float)(frequency));
            oscillators.add(oscillator);
        }

        level = 0.25f / (float)numberOfOscillators;
    }

    void releaseResources() override {}

    void getNextAudioBlock(const juce::AudioSourceChannelInfo& bufferToFill) override
    {
        auto* leftBuffer = bufferToFill.buffer->getWritePointer(0, bufferToFill.startSample);
        auto* rightBuffer = bufferToFill.buffer->getWritePointer(1, bufferToFill.startSample);

        bufferToFill.clearActiveBufferRegion();

        for (auto oscillatorIndex = 0; oscillatorIndex < oscillators.size(); ++oscillatorIndex)
        {
            auto* oscillator = oscillators.getUnchecked(oscillatorIndex);

            for (auto sample = 0; sample < bufferToFill.numSamples; ++sample)
            {
                auto levelSample = oscillator->getNextSample() * level;

                leftBuffer[sample] += levelSample;
                rightBuffer[sample] += levelSample;
            }
        }
    }

private:
    juce::Label cpuUsageLabel;
    juce::Label cpuUsageText;

    juce::Label freqLabel;
    juce::Slider freqSlider;

    juce::ComboBox waveChooser;

    std::function<float(float)> functLambdaArray[4] = {
        [](float x) { return std::sin(x); },
        [](float x) { return x < juce::MathConstants<float>::pi ? 0 : 1; },
        [](float x) { return x > 3.0f * juce::MathConstants<float>::halfPi ?
                                4.0f - x / juce::MathConstants<float>::halfPi :
                             x > juce::MathConstants<float>::halfPi ?
                                x / juce::MathConstants<float>::halfPi - 2.0f :
                                - x / juce::MathConstants<float>::halfPi; },
        [](float x) { return x / juce::MathConstants<float>::pi - 1; }
    };

    const unsigned int tableSize = 1 << 7;
    float level = 0.0f;

    juce::AudioSampleBuffer sineTable;
    juce::OwnedArray<WavetableOscillator> oscillators;

    JUCE_DECLARE_NON_COPYABLE_WITH_LEAK_DETECTOR(MainContentComponent)
};
