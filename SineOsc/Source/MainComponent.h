/*
  ==============================================================================

    This file was auto-generated!

  ==============================================================================
*/

#pragma once

//#include <JuceHeader.h>

//==============================================================================
/*
    This component lives inside our window, and this is where you should put all
    your controls and content.
*/
class MainContentComponent   : public juce::AudioAppComponent
{
public:
    //==============================================================================
	MainContentComponent() {
		addAndMakeVisible(frequencySlider);
		frequencySlider.setRange(30.0, 17000.0);
		frequencySlider.setSkewFactorFromMidPoint(500.0);
		frequencySlider.setValue(currentFrequency, juce::dontSendNotification);
		frequencySlider.onValueChange = [this] {targetFrequency = frequencySlider.getValue(); };
		setSize(600, 100);
		setAudioChannels(0, 2);
	
	};
	~MainContentComponent() override{
		shutdownAudio();
	};

    //==============================================================================
	void prepareToPlay(int, double sampleRate) override {
		currentSampleRate = sampleRate;
		updateAngleDelta();
	};

	inline void updateAngleDelta() {
		auto cyclesPerSample = currentFrequency / currentSampleRate;
		angleDelta = cyclesPerSample * 2.0 * juce::MathConstants<double>::pi;
	}

	void getNextAudioBlock(const juce::AudioSourceChannelInfo& bufferToFill) override {
		auto level = 0.125f;
		auto* leftBuffer = bufferToFill.buffer->getWritePointer(0, bufferToFill.startSample);
		auto* rightbuffer = bufferToFill.buffer->getWritePointer(1, bufferToFill.startSample);
		auto localTargetFrequency = targetFrequency;
		if (localTargetFrequency != currentFrequency) {
			auto frequencyIncrement = (localTargetFrequency - currentFrequency) / bufferToFill.numSamples;
			for (auto sample = 0; sample < bufferToFill.numSamples; sample++) {
				auto currentSample = (float)std::sin(currentAngle);
				currentFrequency += frequencyIncrement;
				updateAngleDelta();
				currentAngle += angleDelta;
				leftBuffer[sample] = currentSample * level;
				rightbuffer[sample] = currentSample * level;

			}
			currentFrequency = localTargetFrequency;
		}
		else {
			for (auto sample = 0; sample < bufferToFill.numSamples; sample++) {
				auto currentSample = (float)std::sin(currentAngle);
				currentAngle += angleDelta;
				leftBuffer[sample] = currentSample * level;
				rightbuffer[sample] = currentSample * level;
			}
		}
	};
	void releaseResources() override {
		
	};

    //==============================================================================

	void resized() override {
		frequencySlider.setBounds(10, 10, getWidth() - 20, 20);
	};

private:
    //==============================================================================
    // Your private member variables go here...
	juce::Slider frequencySlider;
	double currentSampleRate = 0.0, currentAngle = 0.0, angleDelta = 0.0;
	double currentFrequency = 500.0, targetFrequency = 500.0;

    JUCE_DECLARE_NON_COPYABLE_WITH_LEAK_DETECTOR (MainContentComponent)
};
