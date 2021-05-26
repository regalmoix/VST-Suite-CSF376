/*
  ==============================================================================

    This file contains the basic framework code for a JUCE plugin editor.

  ==============================================================================
*/

#include "PluginProcessor.h"
#include "PluginEditor.h"

//==============================================================================
DistortAudioProcessorEditor::DistortAudioProcessorEditor (DistortAudioProcessor& p)
    : AudioProcessorEditor (&p), audioProcessor (p)
{
    // Make sure that before the constructor has finished, you've set the
    // editor's size to whatever you need it to be.

    gainSlider.setSliderStyle(juce::Slider::SliderStyle::RotaryHorizontalDrag);
    gainSlider.setTextBoxStyle(juce::Slider::TextEntryBoxPosition::TextBoxLeft, true, 50, 50);
    gainSlider.setRange(1.0f, 4.0f, 0.01f);
    gainSlider.setValue(1.0f, juce::dontSendNotification);
    gainSlider.onValueChange = [this] {
        audioProcessor.gainValue = (double)gainSlider.getValue();
    };
    addAndMakeVisible(gainSlider);

    ceilSlider.setSliderStyle(juce::Slider::SliderStyle::RotaryHorizontalDrag);
    ceilSlider.setTextBoxStyle(juce::Slider::TextEntryBoxPosition::TextBoxLeft, true, 50, 50);
    ceilSlider.setRange(0.0001f, 1.0f, 0.0001f);
    ceilSlider.setValue(0.5f, juce::dontSendNotification);
    ceilSlider.onValueChange = [this] {
        audioProcessor.ceilValue = (double)ceilSlider.getValue();
    };
    addAndMakeVisible(ceilSlider);

    /*waveChooser.setEditableText(false);
    waveChooser.setJustificationType(juce::Justification::centred);
    waveChooser.addItemList({ "Sine", "Custom" }, 1);
    waveChooser.onChange = [this] {
        int selId = waveChooser.getSelectedId();
        if (selId == 2)
            audioProcessor.waveType = DistortAudioProcessor::WaveType::custom;
        else audioProcessor.waveType = DistortAudioProcessor::WaveType::sine;
    };
    addAndMakeVisible(waveChooser);*/

    gainLabel.setJustificationType(juce::Justification::centred);
    ceilLabel.setJustificationType(juce::Justification::centred);
    addAndMakeVisible(&gainLabel);
    addAndMakeVisible(&ceilLabel);

    setSize ( 400, 300);
}

DistortAudioProcessorEditor::~DistortAudioProcessorEditor()
{
}

//==============================================================================
void DistortAudioProcessorEditor::paint (juce::Graphics& g)
{
    // (Our component is opaque, so we must completely fill the background with a solid colour)
    g.fillAll (getLookAndFeel().findColour (juce::ResizableWindow::backgroundColourId));

    g.setColour (juce::Colours::white);
    g.setFont (15.0f);
    //g.drawFittedText ("Hello World!", getLocalBounds(), juce::Justification::centred, 1);
}

void DistortAudioProcessorEditor::resized()
{
    // This is generally where you'll want to lay out the positions of any
    // subcomponents in your editor..
    gainSlider.setBounds(getWidth() / 2 - 100, getHeight() / 4 - 50, 200, 100);
    ceilSlider.setBounds(getWidth() / 2 - 100, 3 * getHeight() / 4 - 50, 200, 100);
    gainLabel.setBounds(getWidth()/2 + 100, getHeight() / 4 - 25, 70, 50);
    ceilLabel.setBounds(getWidth()/2 + 100, 3 * getHeight() / 4 - 25, 70, 50);
    //waveChooser.setBounds(getWidth() / 2 - 100, 3 * getHeight() / 4, 200, 50);
}
