/*
  ==============================================================================

    This file contains the basic framework code for a JUCE plugin editor.

  ==============================================================================
*/

#pragma once

#include <JuceHeader.h>
#include "PluginProcessor.h"

class LookNFeel : public LookAndFeel_V4
{
// LookAndFeel examples : 
// https://github.com/audioplastic/Juce-look-and-feel-examples/blob/master/Source/lookandfeel/LookAndFeelCustom.cpp
public:
    void drawKnob(  Graphics&,
                    int x, int y, int width, int height,
                    float sliderPosProportional,
                    float rotaryStartAngle,
                    float rotaryEndAngle,
                    Slider&
                );
};

struct LabelWithPosition
{
    String            labelText;
    float             sliderPos;

    float getAngularPosition(const float startAngle, const float endAngle) const;
};

class RotarySlider : public Slider
{
public:
    // Delegating superclass constructor to initialise the super class members according to the below parameters
    // If we did not do this, Slider would have been default constructed
    RotarySlider (RangedAudioParameter& param, const String& unitSuffix);
    ~RotarySlider();

    Array<LabelWithPosition>    labels;
    bool                        showMinMaxLabels    {false};
    
    void    drawLabelAtPosition (Graphics& g, const LabelWithPosition& label, const Point<float>& labelCenter);
    void    drawMinMaxLabels    (Graphics& g, float startAngle, float endAngle);
    void    paint               (Graphics& g) override;
    int     getTextHeight       () const;
    String  getDisplayString    ()              const;


    Rectangle<int> getSliderBounds  () const;

private:
    LookNFeel lnf;
    RangedAudioParameter* parameter;
    String                unit;
};

//==============================================================================

class WavetableSynthAudioProcessorEditor  : public AudioProcessorEditor
{
public:
    WavetableSynthAudioProcessorEditor (WavetableSynthAudioProcessor&);
    ~WavetableSynthAudioProcessorEditor() override;

    //==============================================================================
    void paint (Graphics&) override;
    void resized() override;

private:
    // This reference is provided as a quick way for your editor to
    // access the processor object that created it.
    using APVTS = AudioProcessorValueTreeState;

    WavetableSynthAudioProcessor&   audioProcessor;

    RotarySlider                    attackKnob;
    RotarySlider                    decayKnob;
    RotarySlider                    sustainKnob;
    RotarySlider                    releaseKnob;

    APVTS::SliderAttachment         attackAttachment;
    APVTS::SliderAttachment         decayAttachment;
    APVTS::SliderAttachment         sustainAttachment;
    APVTS::SliderAttachment         releaseAttachment;
    
    std::vector<Component*>         getComponents();

    JUCE_DECLARE_NON_COPYABLE_WITH_LEAK_DETECTOR (WavetableSynthAudioProcessorEditor)
};
