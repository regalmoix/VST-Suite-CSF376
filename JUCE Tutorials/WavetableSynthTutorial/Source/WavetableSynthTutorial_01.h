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

#define freqRangeLeft 0.0f
#define freqRangeRight 1000.0f
#define freqStep 0.1f
#define WAVETABLE_CNT 5
#define OSC_CNT 3

//#include <algorithm>

//==============================================================================
class WavetableOscillator
{
public:
    WavetableOscillator(juce::AudioSampleBuffer wavetableToUse)
        : wavetable(wavetableToUse),
        tableSize(wavetable.getNumSamples())// fft_engine(12)
    {
        jassert(wavetable.getNumChannels() == 1);
        
        freqSpecData = new float[65536];

    }

    void setCurrentSampleRate(float sampleRate) {
        currentSampleRate = sampleRate;
    }

    void setLowestFrequency() {
        tableDelta = 1.0f;
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

        if ((currentIndex += tableDelta) > (float)tableSize-1)
            currentIndex -= (float)(tableSize-1);

        return currentSample;
    }

    void changeWavetable(juce::AudioSampleBuffer& newWavetableToUse) {
        //jassert(newWavetableToUse.getNumSamples() == wavetable.getNumSamples());
        //tableSize = newWavetableToUse.getNumSamples() + 1;
        tableSize = newWavetableToUse.getNumSamples()+1;
        wavetable.setSize(1, tableSize, false, true, true);
        auto sampleSrc = newWavetableToUse.getReadPointer(0);
        auto sampleDst = wavetable.getWritePointer(0);

        //deepcopying
        for (int i = 0; i < tableSize-1; i++) {
            sampleDst[i] = sampleSrc[i];
        }

        sampleDst[tableSize] = sampleDst[0];

        currentIndex = 0;
    }

private:
    juce::AudioSampleBuffer wavetable;
    //dsp::FFT fft_engine;
    int tableSize;
    float currentIndex = 0.0f, tableDelta = 0.0f, currentSampleRate, currentFrequency;
    float* freqSpecData;
};

//==============================================================================
class MainContentComponent : public juce::AudioAppComponent
{
public:
    MainContentComponent()
    {

        freqSlider1.setSliderStyle(juce::Slider::SliderStyle::LinearVertical);
        freqSlider1.setTextBoxStyle(juce::Slider::TextEntryBoxPosition::TextBoxBelow, true, 50, 50);
        freqSlider1.setRange(freqRangeLeft, freqRangeRight, freqStep);
        freqSlider1.setValue(100.0f, juce::dontSendNotification);
        freqSlider1.onValueChange = [this] {
            oscillators[0]->setFrequency((float)freqSlider1.getValue());
        };
        addAndMakeVisible(&freqSlider1);

        freqSlider2.setSliderStyle(juce::Slider::SliderStyle::LinearVertical);
        freqSlider2.setTextBoxStyle(juce::Slider::TextEntryBoxPosition::TextBoxBelow, true, 50, 50);
        freqSlider2.setRange(freqRangeLeft, freqRangeRight, freqStep);
        freqSlider2.setValue(100.0f, juce::dontSendNotification);
        freqSlider2.onValueChange = [this] {
            oscillators[1]->setFrequency((float)freqSlider2.getValue());
        };
        addAndMakeVisible(&freqSlider2);

        freqSlider3.setSliderStyle(juce::Slider::SliderStyle::LinearVertical);
        freqSlider3.setTextBoxStyle(juce::Slider::TextEntryBoxPosition::TextBoxBelow, true, 50, 50);
        freqSlider3.setRange(freqRangeLeft, freqRangeRight, freqStep);
        freqSlider3.setValue(100.0f, juce::dontSendNotification);
        freqSlider3.onValueChange = [this] {
            oscillators[2]->setFrequency((float)freqSlider3.getValue());
        };
        addAndMakeVisible(&freqSlider3);

        waveChooser1.setEditableText(false);
        waveChooser1.setJustificationType(juce::Justification::centred);
        waveChooser1.addItemList({ "Sine", "Square", "Triangle", "Saw", "Custom" }, 1);
        waveChooser1.onChange = [this] {
            if( waveChooser1.getSelectedId() < 5 )
                oscillators[0]->changeWavetable(waveTables[waveChooser1.getSelectedId() - 1]);
            else {
                loadNewWavetableInto(oscillators[0]);
            }
        };
        addAndMakeVisible(&waveChooser1);

        waveChooser2.setEditableText(false);
        waveChooser2.setJustificationType(juce::Justification::centred);
        waveChooser2.addItemList({ "Sine", "Square", "Triangle", "Saw", "Custom" }, 1);
        waveChooser2.onChange = [this] {
            if( waveChooser2.getSelectedId() < 5 )
                oscillators[1]->changeWavetable(waveTables[waveChooser2.getSelectedId() - 1]);
            else {
                loadNewWavetableInto(oscillators[1]);
            }
        };
        addAndMakeVisible(&waveChooser2);
        
        waveChooser3.setEditableText(false);
        waveChooser3.setJustificationType(juce::Justification::centred);
        waveChooser3.addItemList({ "Sine", "Square", "Triangle", "Saw", "Custom" }, 1);
        waveChooser3.onChange = [this] {
            if (waveChooser3.getSelectedId() < 5)
                oscillators[2]->changeWavetable(waveTables[waveChooser3.getSelectedId() - 1]);
            else {
                loadNewWavetableInto(oscillators[2]);
            }
        };
        addAndMakeVisible(&waveChooser3);

        createWavetables();
        initOscillators();

        formatManager.registerBasicFormats();

        setSize(700, 300);
        setAudioChannels(0, 2); // no inputs, two outputs
    }

    ~MainContentComponent() override
    {
        shutdownAudio();
    }

    void resized() override
    {

        int draw_step = getWidth() / OSC_CNT;
        int box_height = 70;

        freqSlider1.setBounds(0, 0, draw_step, getHeight() - box_height);
        freqSlider2.setBounds(draw_step, 0, draw_step, getHeight() - box_height);
        freqSlider3.setBounds(draw_step*2, 0, draw_step, getHeight() - box_height);
        waveChooser1.setBounds(0, getHeight() - box_height, draw_step, box_height);
        waveChooser2.setBounds(draw_step, getHeight() - box_height, draw_step, box_height);
        waveChooser3.setBounds(draw_step*2, getHeight() - box_height, draw_step, box_height);
    }

    void createWavetables()
    {
        for (int w = 0; w < WAVETABLE_CNT; w++) {
            waveTables[w].setSize(1, (int)tableSize + 1);
            waveTables[w].clear();

            auto* samples = waveTables[w].getWritePointer(0);

            //int harmonics[] = { 1, 3, 5, 6, 7, 9, 13, 15 };
            //float harmonicWeights[] = { 0.5f, 0.1f, 0.05f, 0.125f, 0.09f, 0.005f, 0.002f, 0.001f };     // [1]
            int harmonics[] = { 1 };
            float harmonicWeights[] = { 1.0f };

            jassert(juce::numElementsInArray(harmonics) == juce::numElementsInArray(harmonicWeights));

            for (auto harmonic = 0; harmonic < juce::numElementsInArray(harmonics); ++harmonic)
            {
                auto angleDelta = juce::MathConstants<float>::twoPi / (float)(tableSize - 1) * harmonics[harmonic]; // [2]
                auto currentAngle = 0.0;

                for (unsigned int i = 0; i < tableSize; ++i)
                {
                    //auto sample = std::sin(currentAngle);
                    auto sample = functLambdaArray[w](currentAngle);
                    samples[i] += (float)sample * harmonicWeights[harmonic];                           // [3]
                    currentAngle += angleDelta;
                    if (currentAngle > juce::MathConstants<float>::twoPi)
                        currentAngle -= juce::MathConstants<float>::twoPi;
                }
            }

            samples[tableSize] = samples[0];
        }
    }

    void loadNewWavetableInto(WavetableOscillator *oscill) {
        juce::FileChooser chooser("Select a wave file", {}, "*.wav;*.mp3");
        if (chooser.browseForFileToOpen()) {
            auto file = chooser.getResult();
            auto* reader = formatManager.createReaderFor(file);
            reader->read(&waveTables[WAVETABLE_CNT - 1], 0, waveTables[WAVETABLE_CNT - 1].getNumSamples(), 0, true, false);

            oscill->changeWavetable(waveTables[WAVETABLE_CNT - 1]);
            //oscill->setLowestFrequency();
            oscill->setFrequency(100.0f);
        }
    }

    void initOscillators() {

        for (auto i = 0; i < OSC_CNT; ++i)
        {
            auto* oscillator = new WavetableOscillator(waveTables[0]);
            oscillators.add(oscillator);
        }
    }

    void prepareToPlay(int samplesPerBlockExpected, double sampleRate) override
    {
        
        //auto frequency = juce::Random::getSystemRandom().nextDouble() * (freqRangeRight - freqRangeLeft) + freqRangeLeft;

        for (auto i = 0; i < OSC_CNT; ++i)
        {
            oscillators[i]->setCurrentSampleRate((float)sampleRate);
            oscillators[i]->setFrequency(100.0f);
        }

        highPassFilter = new dsp::IIR::Filter<float>(highPassCoeffs.makeHighPass(sampleRate, 500.0f));

        highPassFilter->reset();
        dsp::ProcessSpec thisProcess = { sampleRate, (uint32)samplesPerBlockExpected, (uint32)1 };
        highPassFilter->prepare(thisProcess);

    }

    void releaseResources() override {}

    void getNextAudioBlock(const juce::AudioSourceChannelInfo& bufferToFill) override
    {
        auto* leftBuffer = bufferToFill.buffer->getWritePointer(0, bufferToFill.startSample);
        auto* rightBuffer = bufferToFill.buffer->getWritePointer(1, bufferToFill.startSample);

        bufferToFill.clearActiveBufferRegion();

        for (auto sample = 0; sample < bufferToFill.numSamples; ++sample)
        {
            for (int i = 0; i < OSC_CNT; i++) {

                auto levelSample = oscillators[i]->getNextSample();

                leftBuffer[sample] += levelSample;
                rightBuffer[sample] += levelSample;
            }
        }

        //dsp::AudioBlock<float> block(&leftBuffer, 1, bufferToFill.numSamples);

        dsp::AudioBlock<float> block(*bufferToFill.buffer, bufferToFill.startSample);

        dsp::ProcessContextReplacing<float> processContext_(block);

        highPassFilter->process(processContext_);

        //highPassFilter->reset();

        //dsp::AudioBlock<float> block2(&rightBuffer, 1, bufferToFill.numSamples);
        //dsp::ProcessContextReplacing<float> processContext_2(block);

        //highPassFilter->process(processContext_2);

        //highPassFilter->process(leftBuffer);
        //highPassFilter->process(rightBuffer);

    }

private:

    juce::Slider freqSlider1, freqSlider2, freqSlider3;
    juce::ComboBox waveChooser1, waveChooser2, waveChooser3;

    std::function<float(float)> functLambdaArray[WAVETABLE_CNT] = {
        //sine wave
        [](float x) { return std::sin(x); },
        //square wave
        [](float x) { return x < juce::MathConstants<float>::pi ? -1 : 1; },
        //triangle wave
        [](float x) { return x > juce::MathConstants<float>::pi ? -2 * x / juce::MathConstants<float>::pi + 3
                        : 2 * x / juce::MathConstants<float>::pi - 1; },
        //sawtooth wave
        [](float x) { return x / juce::MathConstants<float>::pi - 1; },
        //silence, will be filled up with custom wave
        [](float x) { return 0; }
    };

    const unsigned int tableSize = 1 << 12;

    juce::AudioFormatManager formatManager;

    juce::AudioSampleBuffer waveTables[WAVETABLE_CNT];
    juce::OwnedArray<WavetableOscillator> oscillators;

    dsp::IIR::Coefficients<float> highPassCoeffs;
    dsp::IIR::Filter<float> *highPassFilter;

    JUCE_DECLARE_NON_COPYABLE_WITH_LEAK_DETECTOR(MainContentComponent)
};
