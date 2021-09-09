/*
  ==============================================================================

    This file contains the basic framework code for a JUCE plugin editor.

  ==============================================================================
*/

#pragma once

#include <JuceHeader.h>
#include "PluginProcessor.h"

//==============================================================================
/**
*/
class NoteSliderAudioProcessorEditor  : public juce::AudioProcessorEditor
{
public:
    NoteSliderAudioProcessorEditor (NoteSliderAudioProcessor&);
    ~NoteSliderAudioProcessorEditor() override;

    //==============================================================================
    void paint (juce::Graphics&) override;
    void resized() override;

private:
    // This reference is provided as a quick way for your editor to
    // access the processor object that created it.
    NoteSliderAudioProcessor& audioProcessor;

    juce::Slider time_based_slider, const_freq_slider, const_note_slider, vel_based_speed_slider, PP_A_slider, PP_B_slider;
    juce::Label time_based_label{"", "Time to slide"};
    juce::Label const_freq_label{"", "Const Freq Step"};
    juce::Label const_note_label{ "", "Const Note Step" };
    juce::Label vel_label{"", "Velocity based"};
    juce::Label PP_A_label{"", "PingPong Freq"};
    juce::Label PP_B_label{"", "PingPong Damp"};
    juce::ComboBox speed_choice_box;

    JUCE_DECLARE_NON_COPYABLE_WITH_LEAK_DETECTOR (NoteSliderAudioProcessorEditor)
};
