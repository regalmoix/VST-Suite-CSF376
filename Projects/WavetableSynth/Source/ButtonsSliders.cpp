/*
    ==============================================================================

    ButtonsSliders.cpp
    Created: 12 Sep 2021 8:25:10pm
    Author:  regalmoix

    ==============================================================================
*/

#include "PluginEditor.h"
#include "PluginProcessor.h"

// ======================== LabelWithPosition =======================================

float LabelWithPosition::getAngularPosition(const float startAngle, const float endAngle) const
{
    return jmap (sliderPos, 0.0f, 1.0f, startAngle, endAngle);
}

// =============== RotarySlider Constructor / Destructor ==============================

RotarySlider::RotarySlider (RangedAudioParameter& param, const String& unitSuffix) 
        : Slider  (Slider::SliderStyle::RotaryHorizontalVerticalDrag, Slider::TextEntryBoxPosition::NoTextBox),
          parameter     (&param),
          unit          (unitSuffix)
{
    /** @TODO: Customise pop up : https://forum.juce.com/t/slider-setpopupdisplayenabled-popup-position/12611 */
    setPopupDisplayEnabled(true, true, getParentComponent());
    /** @TODO: Possible to use velocity sensitive dragging */
    setLookAndFeel(&lnf);
}

RotarySlider::~RotarySlider()
{
    setLookAndFeel(nullptr);
}

// =================== RotarySlider Member Functions ==============================

String RotarySlider::getDisplayString() const
{
    String labelText;
    double value = getValue();
    // For checking if parameter is a choice type for slope 
    if (auto* choiceTypeParam = dynamic_cast<AudioParameterChoice*>(parameter))
        labelText = choiceTypeParam->getCurrentChoiceName();
    
    // For checking if parameter is a float type like gain or freq or quality 
    else if (auto* floatTypeParam = dynamic_cast<AudioParameterFloat*>(parameter))
    {
        // Time param using ms
        if (value >= 1000.0)
            labelText = String(value / 1000, 2) + "s";

        // General Case, append Unit to Value [adding space if unit is not empty string]
        else
            labelText = String(value, 0) + ((unit != "") ? " " : "") + unit;
        
    }
    // Should not reach here.
    else
        jassertfalse;

    return labelText;
}

void RotarySlider::paint(Graphics& g)
{
    float startAngle= degreesToRadians(180.0f + 60.0f);
    float endAngle  = degreesToRadians(180.0f - 60.0f) + MathConstants<float>::twoPi;

    auto  range         = getRange();
    auto  sliderBounds  = getSliderBounds();

    if (auto* lookfeel = dynamic_cast<LookNFeel*>(&getLookAndFeel()))
    {
        lookfeel->drawKnob( g, 
                                sliderBounds.getX(), sliderBounds.getY(), 
                                sliderBounds.getWidth(), sliderBounds.getHeight(), 
                                jmap(getValue(), range.getStart(), range.getEnd(), 0.0, 1.0),
                                startAngle,
                                endAngle,
                                *this
                            );
    }
    /** @TODO: Switch to constexpr if, when we finalize to show the label or not at compile time */
    if (showMinMaxLabels)
        drawMinMaxLabels(g, startAngle, endAngle);
}

int RotarySlider::getTextHeight() const
{
   return 14;
}

void RotarySlider::drawMinMaxLabels(Graphics& g, float startAngle, float endAngle)
{
    Point<float> center = getSliderBounds().toFloat().getCentre();
    float        radius = getSliderBounds().getWidth() / 2;

    g.setColour(Colours::hotpink);
    g.setFont  (getTextHeight());

    for (LabelWithPosition& label : labels)
    {

        float labelAngularPos   = label.getAngularPosition(startAngle, endAngle);
        Point labelCenter       = center.getPointOnCircumference(radius + getTextHeight(), labelAngularPos);

        drawLabelAtPosition(g, label, labelCenter);
    }
}

void RotarySlider::drawLabelAtPosition(Graphics& g, const LabelWithPosition& label, const Point<float>& labelCenter)
{
    String text       = label.labelText;

    Rectangle<float> labelRect;

    labelRect.setSize  (g.getCurrentFont().getStringWidth(text), getTextHeight());
    labelRect.setCentre(labelCenter);   

    // Shift down slightly more to accomodate for height of string
    labelRect.setY(labelRect.getY() + getTextHeight());

    g.drawFittedText(text, labelRect.toNearestInt(), Justification::centred, 1);
}

Rectangle<int> RotarySlider::getSliderBounds() const
{
    auto bounds =  getLocalBounds();

    auto size = jmin(bounds.getWidth(), bounds.getHeight());

    size -= getTextHeight() * 2;

    Rectangle<int> square;
    square.setSize(size, size);
    square.setCentre(bounds.getCentreX(), 0);

    // y = 0 => top of the component
    square.setY(getTextHeight() / 2);

    return square;
}
