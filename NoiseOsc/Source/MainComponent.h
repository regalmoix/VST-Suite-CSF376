#pragma once
/*
  ==============================================================================

   This file is part of the JUCE tutorials.
   Copyright (c) 2017 - ROLI Ltd.

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
#include <JuceHeader.h>

//==============================================================================
/*
    This component lives inside our window, and this is where you should put all
    your controls and content.
*/
class MainComponent  : public juce::AudioAppComponent
{
public:
    //==============================================================================
    MainComponent()
    {
        targetLevel = 0.125f;

        levelSlider.setRange(0.0, 0.25);
        levelSlider.setValue(targetLevel, juce::dontSendNotification);
        levelSlider.setTextBoxStyle(juce::Slider::TextBoxRight, false, 100, 20);
        levelSlider.onValueChange = [this]
        {
            targetLevel = (float)levelSlider.getValue();
            samplesToTarget = rampLengthSamples;
        };

        levelLabel.setText("Noise Level", juce::dontSendNotification);

        addAndMakeVisible(&levelSlider);
        addAndMakeVisible(&levelLabel);
        // Make sure you set the size of the component after
        // you add any child components.
        setSize (800, 100);

        // Some platforms require permissions to open input channels so request that here
        
            // Specify the number of input and output channels that we want to open
            setAudioChannels (0, 2);
        
    }

    ~MainComponent() override
    {
        // This shuts down the audio device and clears the audio source.
        shutdownAudio();
    }

    //==============================================================================
    void prepareToPlay (int samplesPerBlockExpected, double sampleRate) override
    {
        // This function will be called when the audio device is started, or when
        // its settings (i.e. sample rate, block size, etc) are changed.

        // You can use this function to initialise any resources you might need,
        // but be careful - it will be called on the audio thread, not the GUI thread.

        // For more details, see the help for AudioProcessor::prepareToPlay()
        resetParameters();
    }

    void getNextAudioBlock (const juce::AudioSourceChannelInfo& bufferToFill) override
    {
        // Your audio-processing code goes here!

        // For more details, see the help for AudioProcessor::getNextAudioBlock()

        // Right now we are not producing any data, in which case we need to clear the buffer
        // (to prevent the output of random noise)
        auto numSamplesRemaining = bufferToFill.numSamples;
        auto offset = 0;

        if (samplesToTarget > 0)
        {
            auto levelIncrement = (targetLevel - currentLevel) / samplesToTarget;
            auto numSamplesThisTime = juce::jmin(numSamplesRemaining, samplesToTarget);

            for (auto sample = 0; sample < numSamplesThisTime; ++sample)
            {
                for (auto channel = 0; channel < bufferToFill.buffer->getNumChannels(); ++channel)
                    bufferToFill.buffer->setSample(channel, sample, random.nextFloat() * currentLevel);

                currentLevel += levelIncrement;
                --samplesToTarget;
            }

            offset = numSamplesThisTime;
            numSamplesRemaining -= numSamplesThisTime;

            if (samplesToTarget == 0)
                currentLevel = targetLevel;
        }

        if (numSamplesRemaining > 0)
        {
            for (auto channel = 0; channel < bufferToFill.buffer->getNumChannels(); ++channel)
            {
                auto* buffer = bufferToFill.buffer->getWritePointer(channel, bufferToFill.startSample + offset);

                for (auto sample = 0; sample < numSamplesRemaining; ++sample)
                    *buffer++ = random.nextFloat() * currentLevel;
            }
        }
    }

    void releaseResources() override
    {
        // This will be called when the audio device stops, or when it is being
        // restarted due to a setting change.

        // For more details, see the help for AudioProcessor::releaseResources()
    }

    //==============================================================================
    void resetParameters() {
        currentLevel = targetLevel;
        samplesToTarget = 0;
    }

    void resized() override
    {
        // This is called when the MainContentComponent is resized.
        // If you add any child components, this is where you should
        // update their positions.
        levelLabel.setBounds(10, 10, 90, 20);
        levelSlider.setBounds(100, 10, getWidth() - 110, 20);
    }


private:
    //==============================================================================
    // Your private member variables go here...
    juce::Random random;
    juce::Slider levelSlider;
    juce::Label levelLabel;
    float currentLevel;
    float targetLevel;
    int samplesToTarget;

    static constexpr auto rampLengthSamples = 128;

    JUCE_DECLARE_NON_COPYABLE_WITH_LEAK_DETECTOR (MainComponent)
};
