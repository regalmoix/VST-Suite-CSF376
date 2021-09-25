/*
    ==============================================================================

    GraphComponent.cpp
    Created: 25 Sep 2021 7:38:09pm
    Author:  regalmoix

    ==============================================================================
*/

#include "PluginEditor.h"
#include "PluginProcessor.h"

GraphComponent::GraphComponent(WavetableSynthAudioProcessor& p) :
    processor(p)
{
    // Register as a listener
    auto& parameters = processor.getParameters();
    for (auto& param : parameters)
    {
        param->addListener(this);
    }
    // Initial call to display response curve when plugin started
    updateGraph();

    startTimerHz(60);
}

GraphComponent::~GraphComponent()
{
    // De register as the listener
    auto& parameters = processor.getParameters();
    for (auto& param : parameters)
    {
        param->removeListener(this);
    }
    stopTimer();
}

Array<float> GraphComponent::updateGraph()
{
    std::function<double(double)> inputFunction = processor.getWaveFunction();
    Rectangle<int> bounds   = getLocalBounds();
    int width               = bounds.getWidth();
    
    Array<float> yValues;

    /** @brief If width = 100 then we want 2pi/99 delta so that yValues[99] = yValues[2pi] */
    auto angleDelta = MathConstants<float>::twoPi / (float)(width - 1);
    auto currentAngle = 0.0;

    // Starts at angle = 0, ends at angle = 2*pi
    for (unsigned int i = 0; i < width; ++i)
    {   
        yValues.add(inputFunction(currentAngle));
        currentAngle += angleDelta;
    }
    
    return yValues;
}

void GraphComponent::paint(Graphics& g)
{
    g.fillAll (Colour(0xff323538));

    Path responseCurve;
    auto bounds = getLocalBounds();

    const double minY = bounds.getBottom();
    const double maxY = bounds.getY();

    const Array<float>  yValues = updateGraph();

    auto  mapMagnitudeToPixel = [minY, maxY] (double input)
    {
        double pxval = jmap(input, -1.5, +1.5, minY, maxY);
        return pxval;
    };

    responseCurve.startNewSubPath(bounds.getX(), mapMagnitudeToPixel(yValues[0]));
    
    for (size_t x = 1; x < yValues.size(); ++x)
    {
        responseCurve.lineTo(bounds.getX() + x, mapMagnitudeToPixel(yValues[x]));
    }

    g.setColour(Colours::mediumseagreen);
    g.drawRoundedRectangle(bounds.toFloat(), 4.0f, 3.0f);

    g.setColour(Colours::darkorange);
    g.strokePath(responseCurve, PathStrokeType(2.0f));
}

void GraphComponent::resized()
{
    /* 
        Do Nothing.
    */   
}

void GraphComponent::parameterGestureChanged (int parameterIndex, bool gestureIsStarting) 
{ 
    /* 
        Do Nothing.
    */ 
}

void GraphComponent::parameterValueChanged(int parameterIndex, float newValue)
{
    /** @note : parameterIndex = 0 for wavetable choice menu */
    if (parameterIndex == 0)
    {
        waveChanged.set(true);   
    }
}

void GraphComponent::timerCallback()
{    
    if (waveChanged.compareAndSetBool(false, true))
    {
        repaint();
    }
}
