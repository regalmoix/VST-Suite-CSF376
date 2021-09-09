/*
  ==============================================================================

    This file was auto-generated!

    It contains the basic framework code for a JUCE plugin editor.

  ==============================================================================
*/

#include "PluginProcessor.h"
#include "PluginEditor.h"

template <typename Type>
Type mapToLog (Type base, Type value0To1, Type logRangeMin, Type logRangeMax)
{
    jassert (logRangeMin > 0);
    jassert (logRangeMax > 0);

    auto logMin = std::log10 (logRangeMin) / std::log10 (base);
    auto logMax = std::log10 (logRangeMax) / std::log10 (base);

    return std::pow ((Type) base, value0To1 * (logMax - logMin) + logMin);
}

template <typename Type>
Type mapFromLog (Type base, Type valueInLogRange, Type logRangeMin, Type logRangeMax)
{
    jassert (logRangeMin > 0);
    jassert (logRangeMax > 0);

    auto logMin = std::log10 (logRangeMin) / std::log10 (base);
    auto logMax = std::log10 (logRangeMax) / std::log10 (base);

    return ((std::log10 (valueInLogRange) / std::log10 (base)) - logMin) / (logMax - logMin);
}

//==============================================================================

void LookNFeel::drawRotarySlider (   
                                juce::Graphics& g,
                                int x, int y, int width, int height,
                                float sliderPosProportional,
                                float rotaryStartAngle,
                                float rotaryEndAngle,
                                juce::Slider& slider
                            )
{
    using namespace juce;

    bool isEnabled  = slider.isEnabled();
    auto bounds     = Rectangle<float>(x, y, width, height);
    g.setColour(isEnabled ? Colour(97u, 18u, 167u) : Colours::darkgrey);
    g.fillEllipse(bounds);
    g.setColour(isEnabled ? Colour(255u, 154u, 1u) : Colours::grey);    
    g.drawEllipse(bounds, 1.0f);


    // If slider is castable to RotarySliderWithLabels
    if (auto* rswl = dynamic_cast<RotarySliderWithLabels*>(&slider)) 
    {
        auto boundCenter = bounds.getCentre();

        Path p;
        Rectangle<float> rect;

        // 4 pixel thick clock hand, centered vertically with bottom being just above the center and top touching the circle
        rect.setLeft    (boundCenter.getX() - 2);
        rect.setRight   (boundCenter.getX() + 2);
        rect.setTop     (bounds.getY());
        rect.setBottom  (boundCenter.getY() - rswl->getTextHeight() * 1.5);

        // Draw and rotate the slider's clock hand.
        p.addRoundedRectangle(rect, 3.0f);
        auto sliderAng  = jmap(sliderPosProportional, 0.0f, 1.0f, rotaryStartAngle, rotaryEndAngle);
        p.applyTransform(AffineTransform().rotated(sliderAng, boundCenter.getX(), boundCenter.getY()));
        g.fillPath(p);

        g.setFont(rswl->getTextHeight());
        auto stringWidth = g.getCurrentFont().getStringWidth(rswl->getDisplayString());
        rect.setSize(stringWidth + 5, rswl->getTextHeight() + 3);

        // Center this rectangle to be at center of the bounds
        rect.setCentre(boundCenter);

        // White Text on Black Backgrounf
        g.setColour(Colours::black);
        g.fillRect(rect);
        g.setColour(Colours::white);
        g.drawFittedText(rswl->getDisplayString(), rect.toNearestInt(), juce::Justification::centred, 1);
    }     
}

void LookNFeel::drawToggleButton (   
                                juce::Graphics& g,
                                juce::ToggleButton& button,
                                bool shouldDrawButtonAsHighlighted,
                                bool shoulDrawButtonAsDown
                            )
{
    using namespace juce;

    auto bounds = button.getLocalBounds();

    if (auto* pbtn = dynamic_cast<PowerButton*>(&button))  
    {
        Path powerButton;

        auto size   = jmin(bounds.getHeight(), bounds.getWidth()) - 4;
        auto rect   = bounds.withSizeKeepingCentre(size, size).toFloat();
        float angle = degreesToRadians(30.0f);

        powerButton.addCentredArc(
                                    rect.getCentreX(), rect.getCentreY(), 
                                    size / 2 - 4, size / 2 - 4,
                                    0.0f, 
                                    angle, MathConstants<float>::twoPi - angle,
                                    true
                                );

        powerButton.startNewSubPath(rect.getCentreX(), rect.getY() + 2);
        powerButton.lineTo(rect.getCentre());

        PathStrokeType pathStrokeType(2.0f, PathStrokeType::JointStyle::curved);

        button.getToggleState() ? g.setColour(Colours::steelblue) : g.setColour(Colours::grey);

        g.strokePath(powerButton, pathStrokeType);
        g.drawEllipse(rect, 2.0f);
    }

    else if (auto* abtn = dynamic_cast<AnalyzerButton*>(&button))  
    {
        button.getToggleState() ? g.setColour(Colours::steelblue) : g.setColour(Colours::grey);
        g.drawRect(bounds, 2.0f);
        bounds = bounds.reduced(4);
        g.strokePath(abtn->getPath(), PathStrokeType(2.0f));
    }

    else
        jassertfalse;
}

//==============================================================================

juce::String RotarySliderWithLabels::getDisplayString() const
{
    juce::String labelText;

    // For checking if parameter is a choice type for slope 
    if (auto* choiceTypeParam = dynamic_cast<AudioParameterChoice*>(parameter))
        labelText = choiceTypeParam->getCurrentChoiceName();
    
    // For checking if parameter is a float type like gain or freq or quality 
    else if (auto* floatTypeParam = dynamic_cast<AudioParameterFloat*>(parameter))
    {
        // Only frequency param can exceed 1000 so direcly can use kHz as Unit
        if (getValue() >= 1000.0)
            labelText = juce::String(getValue() / 1000, 2) + " kHz";

        // General Case, append Unit to Value [adding space if unit is not empty string]
        else
            labelText = juce::String(getValue(), 0) + ((unit != "") ? " " : "") + unit;
        
    }
    // Should not reach here.
    else
        jassertfalse;

    return labelText;
}

void RotarySliderWithLabels::paint(juce::Graphics& g)
{
    using namespace juce;

    float startAngle= degreesToRadians(180.0f + 60.0f);
    float endAngle  = degreesToRadians(180.0f - 60.0f) + MathConstants<float>::twoPi;

    auto  range         = getRange();
    auto  sliderBounds  = getSliderBounds();

    // For Boundary Debugging
    // g.setColour(Colours::yellowgreen);
    // g.drawRect(sliderBounds);
    // g.setColour(Colours::red);
    // g.drawRect(getLocalBounds());

    getLookAndFeel().drawRotarySlider ( g, 
                                        sliderBounds.getX(), sliderBounds.getY(), 
                                        sliderBounds.getWidth(), sliderBounds.getHeight(), 
                                        jmap(getValue(), range.getStart(), range.getEnd(), 0.0, 1.0),
                                        startAngle,
                                        endAngle,
                                        *this
                                    );
    Point<float> center = sliderBounds.toFloat().getCentre();
    float        radius = sliderBounds.getWidth() / 2;

    g.setColour(Colours::hotpink);
    g.setFont  (getTextHeight());

    for (LabelWithPosition& label : labels)
    {

        float labelAngularPos   = label.getAngularPosition(startAngle, endAngle);
        Point labelCenter       = center.getPointOnCircumference(radius + getTextHeight(), labelAngularPos);

        drawLabelAtPosition(g, label, labelCenter);
    }

}

void RotarySliderWithLabels::drawLabelAtPosition(juce::Graphics& g, const LabelWithPosition& label, const Point<float>& labelCenter)
{
    juce::String text       = label.labelText;

    Rectangle<float> labelRect;

    labelRect.setSize  (g.getCurrentFont().getStringWidth(text), getTextHeight());
    labelRect.setCentre(labelCenter);   

    // Shift down slightly more to accomodate for height of string
    labelRect.setY(labelRect.getY() + getTextHeight());

    g.drawFittedText(text, labelRect.toNearestInt(), juce::Justification::centred, 1);
}

juce::Rectangle<int> RotarySliderWithLabels::getSliderBounds() const
{
    auto bounds =  getLocalBounds();

    auto size = juce::jmin(bounds.getWidth(), bounds.getHeight());

    size -= getTextHeight() * 2;

    juce::Rectangle<int> square;
    square.setSize(size, size);
    square.setCentre(bounds.getCentreX(), 0);

    // y = 0 => top of the component
    square.setY(getTextHeight() / 2);

    return square;
}


//==============================================================================

ResponseCurveComponent::ResponseCurveComponent(VstpluginAudioProcessor& p) 
    :   processor(p),
        leftPath (p.leftChannelFifo),
        rightPath(p.rightChannelFifo)
{
    // Register as a listener
    auto& parameters = processor.getParameters();
    for (auto& param : parameters)
    {
        param->addListener(this);
    }

    // Initial call to display response curve when plugin started
    updateChain();

    startTimerHz(60);
}

ResponseCurveComponent::~ResponseCurveComponent()
{
    // De register as the listener
    auto& parameters = processor.getParameters();
    for (auto& param : parameters)
    {
        param->removeListener(this);
    }
    stopTimer();
}

//==============================================================================

void ResponseCurveComponent::paint (Graphics& g)
{
    using namespace juce;

    // (Our component is opaque, so we must completely fill the background with a solid colour)
    g.fillAll (Colours::black);
    g.drawImage(backgroundGrid, getRenderArea().toFloat());

    auto frequencyResponseArea = getRenderArea();
    auto width          = frequencyResponseArea.getWidth();

    CutFilter& lowCut   = monoChain.get<ChainPostitions::LowCut>();
    Filter& peak        = monoChain.get<ChainPostitions::Peak>();
    CutFilter& highCut  = monoChain.get<ChainPostitions::HighCut>();

    auto sampleRate = processor.getSampleRate();

    // For each pixel in 0 to width - 1 index, we want the magnitude (the height of the graph) in dB
    std::vector<double> magnitudesDecibel(width);

    for (int i = 0; i < width; ++i)
    {
        // Initialise to 1 gain (ie no change of volume)
        double magnitude = 1.0f;
        // On log scale, get frequency corresponding to pixel #i given 13Hz is i = 0 and 22kHz is i = width - 1
        double frequency = mapToLog<double>(10.0 ,(double) i / (double) width, 15.0, 22000.0);

        if (!monoChain.isBypassed<ChainPostitions::Peak>())
            magnitude *= peak.coefficients->getMagnitudeForFrequency(frequency, sampleRate);

        if (!monoChain.isBypassed<ChainPostitions::LowCut>())
        {
            if (!lowCut.isBypassed<0>())
                magnitude *= lowCut.get<0>().coefficients->getMagnitudeForFrequency(frequency, sampleRate);
            if (!lowCut.isBypassed<1>())
                magnitude *= lowCut.get<1>().coefficients->getMagnitudeForFrequency(frequency, sampleRate);
            if (!lowCut.isBypassed<2>())
                magnitude *= lowCut.get<2>().coefficients->getMagnitudeForFrequency(frequency, sampleRate);
            if (!lowCut.isBypassed<3>())
                magnitude *= lowCut.get<3>().coefficients->getMagnitudeForFrequency(frequency, sampleRate);
        }

        if (!monoChain.isBypassed<ChainPostitions::HighCut>())
        {
            if (!highCut.isBypassed<0>())
                magnitude *= highCut.get<0>().coefficients->getMagnitudeForFrequency(frequency, sampleRate);
            if (!highCut.isBypassed<1>())
                magnitude *= highCut.get<1>().coefficients->getMagnitudeForFrequency(frequency, sampleRate);
            if (!highCut.isBypassed<2>())
                magnitude *= highCut.get<2>().coefficients->getMagnitudeForFrequency(frequency, sampleRate);
            if (!highCut.isBypassed<3>())
                magnitude *= highCut.get<3>().coefficients->getMagnitudeForFrequency(frequency, sampleRate);
        }

        magnitudesDecibel[i] = Decibels::gainToDecibels(magnitude);;
    }

    Path responseCurve;
    const double minY = frequencyResponseArea.getBottom();
    const double maxY = frequencyResponseArea.getY();

    auto  mapMagnitudeToPixel = [minY, maxY] (double input)
    {
        // 4dB headroom => 24 + 4 = 28dB
        double pxval = jmap(input, -28.0, +28.0, minY, maxY);
        return pxval;
    };

    responseCurve.startNewSubPath(frequencyResponseArea.getX(), mapMagnitudeToPixel(magnitudesDecibel.front()));

    for (size_t x = 1; x < magnitudesDecibel.size(); ++x)
    {
        responseCurve.lineTo(frequencyResponseArea.getX() + x, mapMagnitudeToPixel(magnitudesDecibel[x]));
    }

    if (showFFT)
    {

        auto leftFFTPath  = leftPath .getPath();
        auto rightFFTPath = rightPath.getPath();

        g.setColour(Colours::cyan);
        leftFFTPath.applyTransform(AffineTransform().translation(frequencyResponseArea.getX(), frequencyResponseArea.getY()));
        g.strokePath(leftFFTPath, PathStrokeType(1.0f));


        g.setColour(Colours::greenyellow);
        rightFFTPath.applyTransform(AffineTransform().translation(frequencyResponseArea.getX(), frequencyResponseArea.getY()));
        g.strokePath(rightFFTPath, PathStrokeType(1.0f));
    }

    g.setColour(Colours::orange);
    g.drawRoundedRectangle(frequencyResponseArea.toFloat(), 4.0f, 3.0f);

    g.setColour(Colours::white);
    g.strokePath(responseCurve, PathStrokeType(2.0f));
}

void ResponseCurveComponent::resized()
{
    using namespace juce;
    backgroundGrid = Image(Image::PixelFormat::RGB, getWidth(), getHeight(), true);

    Graphics g(backgroundGrid);

    Array<float> frequencies {
        20.0f,   50.0f,   100.0f, 
        200.0f,  500.0f,  1000.0f, 
        2000.0f, 5000.0f, 10000.0f, 20000.0f
    };

    Array<float> xValuesOfFreq;
    Array<float> yValuesOfGain;

    Array<float> gains {
        -24.0f, -12.0f, -6.0f, 0.0f, 6.0f, 12.0f, 24.0f
    };

    g.setColour(Colours::lightblue);
    
    for (float f : frequencies)
    {
        float x = mapFromLog<float>(10.0f, f, 13.0f, 22000.0f) * getWidth();
        xValuesOfFreq.add(x);
        g.drawVerticalLine(x, 0.0f, getHeight());
    }

    for (float gain : gains)
    {
        float y = jmap<float>(gain, -28.0f, +28.0f, getHeight(), 0.0f);
        // Yellow for 0dB, but grey otherwise
        yValuesOfGain.add(y);
        g.setColour(gain == 0.0f ? Colours::yellow : Colours::grey);
        g.drawHorizontalLine(y, 0.0f, getWidth());
    }

    const int fontHeight = 12;

    g.setFont(fontHeight);

    for (int i = 0; i < frequencies.size(); i++)
    {
        float freq  = frequencies[i];
        float x     = xValuesOfFreq[i];

        juce::String labelText;
        
        if (freq >= 1000.0)
            labelText = juce::String(freq / 1000, 2) + " kHz";

        else
            labelText = juce::String(freq, 0) + " Hz";
        
        Rectangle<int> labelRect;
        labelRect.setSize(g.getCurrentFont().getStringWidth(labelText), fontHeight);
        labelRect.setCentre(x, 0);
        labelRect.setY(2);

        g.setColour(Colours::black);
        g.fillRect(labelRect);

        g.setColour(Colours::lightpink);
        g.drawFittedText(labelText, labelRect, juce::Justification::centred, 1);
    }   

    for (int i = 0; i < gains.size(); i++)
    {
        float gain  = gains[i];
        float y     = yValuesOfGain[i];

        juce::String labelText;
        
        labelText = juce::String(gain, 0) + " dB";
        
        Rectangle<int> labelRect;
        labelRect.setSize(g.getCurrentFont().getStringWidth(labelText), fontHeight);
        labelRect.setCentre(0, y);
        labelRect.setX(2);

        g.setColour(Colours::black);
        g.fillRect(labelRect);

        g.setColour(Colours::lightpink);
        g.drawFittedText(labelText, labelRect, juce::Justification::centred, 1);
    } 
}

juce::Rectangle<int> ResponseCurveComponent::getRenderArea()
{
    auto bounds = getLocalBounds();
    return bounds;
}

void ResponseCurveComponent::parameterValueChanged(int parameterIndex, float newValue)
{
    paramsChanged.set(true);
}

void ResponseCurveComponent::timerCallback()
{
    if (showFFT)
    {

        const auto  fftBounds   = getRenderArea().toFloat();
        double      sampleRate  = processor.getSampleRate();

        leftPath .process(fftBounds, sampleRate);
        rightPath.process(fftBounds, sampleRate);
    }

    if (paramsChanged.compareAndSetBool(false, true))
    {
        // Update mono chain of editor
        updateChain();
    }

    // Do a repaint at each callback for drawing update FFT Path
    repaint();
}

void ResponseCurveComponent::updateChain()
{
    const double sampleRate         = processor.getSampleRate();

    ChainSettings  chainSettings        = getChainSettings(processor.apvts);

    monoChain.setBypassed<ChainPostitions::LowCut>  (chainSettings.lowCutBypass);
    monoChain.setBypassed<ChainPostitions::HighCut> (chainSettings.highCutBypass);
    monoChain.setBypassed<ChainPostitions::Peak>    (chainSettings.peakBypass);

    CoefficientPtr peakCoefficients     = makePeakFilter(chainSettings, sampleRate);
    CoefficientArr lowCutCoefficients   = makeLowCutFilter(chainSettings, sampleRate);
    CoefficientArr highCutCoefficients  = makeHighCutFilter(chainSettings, sampleRate);

    updateCoefficients  (monoChain.get<ChainPostitions::Peak>().coefficients, peakCoefficients); 
    updateCutFilter     (monoChain.get<ChainPostitions::LowCut>(),  lowCutCoefficients,  chainSettings.lowCutSlope);
    updateCutFilter     (monoChain.get<ChainPostitions::HighCut>(), highCutCoefficients, chainSettings.highCutSlope);
}

//==============================================================================

VstpluginAudioProcessorEditor::VstpluginAudioProcessorEditor (VstpluginAudioProcessor& p)
    :   AudioProcessorEditor (&p), 
        processor (p),

        peakFrequencySlider(*processor.apvts.getParameter("Peak Freq"), "Hz"),
        peakGainSlider(*processor.apvts.getParameter("Peak Gain"), "dB"),
        peakQualitySlider(*processor.apvts.getParameter("Peak Q"), ""),
        lowCutFrequencySlider(*processor.apvts.getParameter("LowCut Freq"), "Hz"),
        highCutFrequencySlider(*processor.apvts.getParameter("HighCut Freq"), "Hz"),
        lowCutSlopeSlider(*processor.apvts.getParameter("LowCut Slope"), "dB/Oct"),
        highCutSlopeSlider(*processor.apvts.getParameter("HighCut Slope"), "dB/Oct"),

        responseCurveComponent          (processor),
        peakFrequencySliderAttachment (processor.apvts, "Peak Freq", peakFrequencySlider),
        peakGainSliderAttachment      (processor.apvts, "Peak Gain", peakGainSlider),
        peakQualitySliderAttachment   (processor.apvts, "Peak Q",    peakQualitySlider),

        lowCutFrequencySliderAttachment (processor.apvts, "LowCut Freq",  lowCutFrequencySlider),
        highCutFrequencySliderAttachment(processor.apvts, "HighCut Freq", highCutFrequencySlider),

        lowCutSlopeSliderAttachment     (processor.apvts, "LowCut Slope",  lowCutSlopeSlider),
        highCutSlopeSliderAttachment    (processor.apvts, "HighCut Slope", highCutSlopeSlider),

        lowCutBypassToggleAttachment    (processor.apvts, "LowCut Bypass",  lowCutBypassToggle),
        highCutBypassToggleAttachment   (processor.apvts, "HighCut Bypass", highCutBypassToggle),
        peakBypassToggleAttachment      (processor.apvts, "Peak Bypass",    peakBypassToggle),
        analyzerToggleAttachment        (processor.apvts, "Analyzer Enable",analyzerToggle)
{
    // Make sure that before the constructor has finished, you've set the
    // editor's size to whatever you need it to be.
    peakFrequencySlider.labels.add({"20 Hz", 0.0f});
    peakFrequencySlider.labels.add({"20 kHz", 1.0f});

    lowCutFrequencySlider.labels.add({"20 Hz", 0.0f});
    lowCutFrequencySlider.labels.add({"20 kHz", 1.0f});

    highCutFrequencySlider.labels.add({"20 Hz", 0.0f});
    highCutFrequencySlider.labels.add({"20 kHz", 1.0f});

    peakQualitySlider.labels.add({"0.1", 0.0f});
    peakQualitySlider.labels.add({"10",  1.0f});

    peakGainSlider.labels.add({"-24 dB", 0.0f});
    peakGainSlider.labels.add({"24 dB",  1.0f});

    for (auto* component : getComponents())
    {
        addAndMakeVisible(component);
    }

    lowCutBypassToggle  .setLookAndFeel(&lnf);
    highCutBypassToggle .setLookAndFeel(&lnf);
    peakBypassToggle    .setLookAndFeel(&lnf);    
    analyzerToggle      .setLookAndFeel(&lnf);    

    juce::Component::SafePointer<VstpluginAudioProcessorEditor> editorPtr (this);

    lowCutBypassToggle.onClick = [editorPtr]()
    {
        // If the editor exisis [not deleted]
        if (auto* component = editorPtr.getComponent())
        {
            auto isBypassed = component->lowCutBypassToggle.getToggleState();
            component->lowCutSlopeSlider    .setEnabled(!isBypassed);
            component->lowCutFrequencySlider.setEnabled(!isBypassed);
        }
    };

    highCutBypassToggle.onClick = [editorPtr]()
    {
        // If the editor exisis [not deleted]
        if (auto* component = editorPtr.getComponent())
        {
            auto isBypassed = component->highCutBypassToggle.getToggleState();
            component->highCutSlopeSlider    .setEnabled(!isBypassed);
            component->highCutFrequencySlider.setEnabled(!isBypassed);
        }
    };

    peakBypassToggle.onClick = [editorPtr]()
    {
        // If the editor exisis [not deleted]
        if (auto* component = editorPtr.getComponent())
        {
            auto isBypassed = component->peakBypassToggle.getToggleState();
            component->peakQualitySlider.setEnabled(!isBypassed);
            component->peakFrequencySlider.setEnabled(!isBypassed);
            component->peakGainSlider   .setEnabled(!isBypassed);
        }   
    };

    analyzerToggle.onClick = [editorPtr]()
    {
        // If the editor exisis [not deleted]
        if (auto* component = editorPtr.getComponent())
        {
            auto isEnabled = component->analyzerToggle.getToggleState();
            component->responseCurveComponent.showFFTAnalysis(isEnabled);
        }
    };

    // Initialise enable states of all toggles
    // If this is not done, sliders will not be disabled on plugin reload [if buttons bypassed]
    lowCutBypassToggle.onClick();
    highCutBypassToggle.onClick();
    peakBypassToggle.onClick();
    analyzerToggle.onClick();

    setSize (800, 600);
}

VstpluginAudioProcessorEditor::~VstpluginAudioProcessorEditor()
{
    lowCutBypassToggle  .setLookAndFeel(nullptr);
    highCutBypassToggle .setLookAndFeel(nullptr);
    peakBypassToggle    .setLookAndFeel(nullptr); 
    analyzerToggle      .setLookAndFeel(nullptr);
}

//==============================================================================

void VstpluginAudioProcessorEditor::paint (Graphics& g)
{
    // (Our component is opaque, so we must completely fill the background with a solid colour)
    g.fillAll (juce::Colours::black);

}

void VstpluginAudioProcessorEditor::resized()
{
    // This is generally where you'll want to lay out the positions of any
    // subcomponents in your editor..

    auto bounds = getLocalBounds();

    // Reserves space of 40% for the frequency spectrum on top
    auto frequencyResponseArea  = bounds.removeFromTop(bounds.getHeight() * 0.40);
    // Reserves space of 33% for the low cut controls, 66% remains
    auto lowCutArea             = bounds.removeFromLeft(bounds.getWidth() * 0.33);
    // Reserves space of 33% for the frequency spectrum (0.5 * 66% = 33%)
    auto highCutArea            = bounds.removeFromRight(bounds.getWidth() * 0.50);

    auto lowCutToggleArea       = lowCutArea.removeFromTop(25);
    auto lowCutFreqSliderArea   = lowCutArea.removeFromTop(bounds.getHeight() * 0.50);
    auto lowCutSlopeSliderArea  = lowCutArea;

    auto highCutToggleArea      = highCutArea.removeFromTop(25);
    auto highCutFreqSliderArea  = highCutArea.removeFromTop(bounds.getHeight() * 0.50);
    auto highCutSlopeSliderArea = highCutArea;

    auto peakToggleArea         = bounds.removeFromTop(25);
    // Top 33% of the middle 33% block
    auto peakFrequencySliderArea= bounds.removeFromTop(bounds.getHeight() * 0.33);
    // Mid 33% of the middle 33% block
    auto peakGainSliderArea     = bounds.removeFromTop(bounds.getHeight() * 0.50);
    // Low 33% of the middle 33% block
    auto peakQualitySliderArea  = bounds;

    Rectangle<int> analyzerToggleArea (frequencyResponseArea.getRight() - 60, frequencyResponseArea.getY() + 24, 48, 27);
    analyzerToggle        .setBounds(analyzerToggleArea);

    lowCutBypassToggle    .setBounds(lowCutToggleArea);
    lowCutFrequencySlider .setBounds(lowCutFreqSliderArea);

    highCutBypassToggle   .setBounds(highCutToggleArea);
    highCutFrequencySlider.setBounds(highCutFreqSliderArea);

    peakBypassToggle      .setBounds(peakToggleArea);
    peakFrequencySlider   .setBounds(peakFrequencySliderArea);

    peakGainSlider        .setBounds(peakGainSliderArea);
    peakQualitySlider     .setBounds(peakQualitySliderArea);

    lowCutSlopeSlider     .setBounds(lowCutSlopeSliderArea);
    highCutSlopeSlider    .setBounds(highCutSlopeSliderArea);

    responseCurveComponent.setBounds(frequencyResponseArea);
}

std::vector<juce::Component*> VstpluginAudioProcessorEditor::getComponents()
{
    return {
        &peakFrequencySlider,
        &peakGainSlider,
        &peakQualitySlider,
        &lowCutFrequencySlider,
        &highCutFrequencySlider,
        &lowCutSlopeSlider,
        &highCutSlopeSlider,
        &responseCurveComponent,
        &lowCutBypassToggle,
        &highCutBypassToggle,
        &peakBypassToggle,
        &analyzerToggle
    };
}