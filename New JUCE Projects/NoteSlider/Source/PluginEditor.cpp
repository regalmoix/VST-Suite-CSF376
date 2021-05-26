/*
  ==============================================================================

    This file contains the basic framework code for a JUCE plugin editor.

  ==============================================================================
*/

#include "PluginProcessor.h"
#include "PluginEditor.h"

//==============================================================================
NoteSliderAudioProcessorEditor::NoteSliderAudioProcessorEditor (NoteSliderAudioProcessor& p)
    : AudioProcessorEditor (&p), audioProcessor (p)
{

    speed_choice_box.setEditableText(false);
    speed_choice_box.setJustificationType(juce::Justification::centred);
    speed_choice_box.addItemList(
        {
            "Time Based",
            "Const Freq Step",
            "Cost Note Step",
            "Direct Vel Based",
            "Inv Vel Based",
            "PingPong"
        }, 1);
    speed_choice_box.setSelectedId(1, false);
    speed_choice_box.onChange = [this] {
        //kinda hacky but it works
        audioProcessor.speeds->speed_choice = speed_choice_box.getSelectedId();
    };
    addAndMakeVisible(&speed_choice_box);

    time_based_slider.setSliderStyle(juce::Slider::SliderStyle::LinearVertical);
    time_based_slider.setTextBoxStyle(juce::Slider::TextEntryBoxPosition::TextBoxBelow, true, 50, 50);
    time_based_slider.setRange(0.01f, 2.0f, 0.01f);
    time_based_slider.setValue(1.0f, juce::dontSendNotification);
    time_based_slider.onValueChange = [this] {
        audioProcessor.speeds->change_time = (double)time_based_slider.getValue();
    };
    addAndMakeVisible(&time_based_slider);

    const_freq_slider.setSliderStyle(juce::Slider::SliderStyle::LinearVertical);
    const_freq_slider.setTextBoxStyle(juce::Slider::TextEntryBoxPosition::TextBoxBelow, true, 50, 50);
    const_freq_slider.setRange(10.0f, 100.0f, 1.0f);
    const_freq_slider.setValue(20.0f, juce::dontSendNotification);
    const_freq_slider.onValueChange = [this] {
        audioProcessor.speeds->freq_speed_per_second = (double)const_freq_slider.getValue();
    };
    addAndMakeVisible(&const_freq_slider);

    const_note_slider.setSliderStyle(juce::Slider::SliderStyle::LinearVertical);
    const_note_slider.setTextBoxStyle(juce::Slider::TextEntryBoxPosition::TextBoxBelow, true, 50, 50);
    const_note_slider.setRange(0.1f, 5.0f, 0.1f);
    const_note_slider.setValue(1.0f, juce::dontSendNotification);
    const_note_slider.onValueChange = [this] {
        audioProcessor.speeds->note_speed_per_second = (double)const_note_slider.getValue();
    };
    addAndMakeVisible(&const_note_slider);

    vel_based_speed_slider.setSliderStyle(juce::Slider::SliderStyle::LinearVertical);
    vel_based_speed_slider.setTextBoxStyle(juce::Slider::TextEntryBoxPosition::TextBoxBelow, true, 50, 50);
    vel_based_speed_slider.setRange(0.1f, 200.0f, 0.1f);
    vel_based_speed_slider.setValue(1.0f, juce::dontSendNotification);
    vel_based_speed_slider.onValueChange = [this] {
        audioProcessor.speeds->vel_based_speed = (double)vel_based_speed_slider.getValue();
    };
    addAndMakeVisible(&vel_based_speed_slider);

    PP_A_slider.setSliderStyle(juce::Slider::SliderStyle::LinearVertical);
    PP_A_slider.setTextBoxStyle(juce::Slider::TextEntryBoxPosition::TextBoxBelow, false, 50, 50);
    PP_A_slider.setRange(0.01f, 50.0f, 0.01f);
    PP_A_slider.setValue(10.0f, juce::dontSendNotification);
    PP_A_slider.onValueChange = [this] {
        audioProcessor.speeds->pp_omega = (double)PP_A_slider.getValue();
        audioProcessor.update_pingpong();
    };
    addAndMakeVisible(&PP_A_slider);

    PP_B_slider.setSliderStyle(juce::Slider::SliderStyle::LinearVertical);
    PP_B_slider.setTextBoxStyle(juce::Slider::TextEntryBoxPosition::TextBoxBelow, false, 50, 50);
    PP_B_slider.setRange(0.001f, 1.0f, 0.001f);
    PP_B_slider.setValue(0.1f, juce::dontSendNotification);
    PP_B_slider.onValueChange = [this] {
        audioProcessor.speeds->pp_zeta = (double)PP_B_slider.getValue();
        audioProcessor.update_pingpong();
    };
    addAndMakeVisible(&PP_B_slider);

    const_freq_label.setJustificationType(juce::Justification::centred);
    const_note_label.setJustificationType(juce::Justification::centred);
    PP_A_label.setJustificationType(juce::Justification::centred);
    PP_B_label.setJustificationType(juce::Justification::centred);
    time_based_label.setJustificationType(juce::Justification::centred);
    vel_label.setJustificationType(juce::Justification::centred);
    addAndMakeVisible(&const_freq_label);
    addAndMakeVisible(&const_note_label);
    addAndMakeVisible(&PP_A_label);
    addAndMakeVisible(&PP_B_label);
    addAndMakeVisible(&time_based_label);
    addAndMakeVisible(&vel_label);

    setSize (700, 500);
}

NoteSliderAudioProcessorEditor::~NoteSliderAudioProcessorEditor()
{
}

//==============================================================================
void NoteSliderAudioProcessorEditor::paint (juce::Graphics& g)
{
    // (Our component is opaque, so we must completely fill the background with a solid colour)
    g.fillAll (getLookAndFeel().findColour (juce::ResizableWindow::backgroundColourId));

    g.setColour (juce::Colours::white);
    g.setFont (15.0f);
}

void NoteSliderAudioProcessorEditor::resized()
{
    // This is generally where you'll want to lay out the positions of any
    // subcomponents in your editor..

    int full_height = getHeight();
    int full_width = getWidth();

    int box_height = (int)(full_height * 0.1f);
    int slider_height = (int)(full_height - box_height);
    int label_height = (int)(slider_height * 0.1f);
    slider_height -= label_height;
    int slider_width = (int)(full_width / 6);
    
    speed_choice_box.setBounds(0, slider_height+label_height, full_width, box_height);

    time_based_slider.setBounds(0, 0, slider_width, slider_height);
    const_freq_slider.setBounds(slider_width, 0, slider_width, slider_height);
    const_note_slider.setBounds(2 * slider_width, 0, slider_width, slider_height);
    vel_based_speed_slider.setBounds(3 * slider_width, 0, slider_width, slider_height);
    PP_A_slider.setBounds(4 * slider_width, 0, slider_width, slider_height);
    PP_B_slider.setBounds(5 * slider_width, 0, slider_width, slider_height);

    time_based_label.setBounds(0, slider_height, slider_width, label_height);
    const_freq_label.setBounds(slider_width, slider_height, slider_width, label_height);
    const_note_label.setBounds(2 * slider_width, slider_height, slider_width, label_height);
    vel_label.setBounds(3 * slider_width, slider_height, slider_width, label_height);
    PP_A_label.setBounds(4 * slider_width, slider_height, slider_width, label_height);
    PP_B_label.setBounds(5 * slider_width, slider_height, slider_width, label_height);

}
