/*
    ==============================================================================

    LookNFeel.cpp
    Created: 12 Sep 2021 8:25:37pm
    Author:  regalmoix

    ==============================================================================
*/

#include "PluginEditor.h"
#include "PluginProcessor.h"


void LookNFeel::drawKnob (   
                            juce::Graphics& g,
                            int x, int y, int width, int height,
                            float sliderPosProportional,
                            float rotaryStartAngle,
                            float rotaryEndAngle,
                            juce::Slider& slider
                        )
{
    setColour (Slider::thumbColourId, Colours::red);
    setColour (Slider::rotarySliderOutlineColourId, Colours::darkslateblue);
    setColour (Slider::rotarySliderFillColourId, Colours::orange);

    drawRotarySlider(g, x, y, width, height, sliderPosProportional, rotaryStartAngle, rotaryEndAngle, slider);
}
