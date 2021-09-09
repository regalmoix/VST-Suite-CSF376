/*
  ==============================================================================

    This file was auto-generated!

    It contains the basic framework code for a JUCE plugin editor.

  ==============================================================================
*/

#pragma once

#include <JuceHeader.h>
#include "PluginProcessor.h"

//==============================================================================
enum FFTOrder
{
    order2048 = 11,
    order4096 = 12,
    order8192 = 13
};

template<typename BlockType>
class FFTDataGenerator
{
private:
    FFTOrder order;
    BlockType fftData;
    std::unique_ptr<juce::dsp::FFT> forwardFFT;
    std::unique_ptr<juce::dsp::WindowingFunction<float>> window;
    
    Fifo<BlockType> fftDataFifo;

public:
    void produceFFTDataForRendering(const juce::AudioBuffer<float>& audioData, const float negativeInfinity)
    {
        const auto fftSize = getFFTSize();
        
        fftData.assign(fftData.size(), 0);
        auto* readIndex = audioData.getReadPointer(0);
        std::copy(readIndex, readIndex + fftSize, fftData.begin());
        
        // first apply a windowing function to our data
        window->multiplyWithWindowingTable (fftData.data(), fftSize);       // [1]
        
        // then render our FFT data..
        forwardFFT->performFrequencyOnlyForwardTransform (fftData.data());  // [2]
        
        int numBins = (int)fftSize / 2;
        
        //normalize the fft values.
        for( int i = 0; i < numBins; ++i )
        {
            auto v = fftData[i];
            // fftData[i] /= (float) numBins;
            if( !std::isinf(v) && !std::isnan(v) )
            {
                v /= float(numBins);
            }
            else
            {
                v = 0.f;
            }
            fftData[i] = v;
        }
        
        //convert them to decibels
        for( int i = 0; i < numBins; ++i )
        {
            fftData[i] = juce::Decibels::gainToDecibels(fftData[i], negativeInfinity);
        }
        
        fftDataFifo.push(fftData);
    }
    
    void changeOrder(FFTOrder newOrder)
    {
        //when you change order, recreate the window, forwardFFT, fifo, fftData
        //also reset the fifoIndex
        //things that need recreating should be created on the heap via std::make_unique<>
        
        order = newOrder;
        auto fftSize = getFFTSize();
        
        forwardFFT = std::make_unique<juce::dsp::FFT>(order);
        window = std::make_unique<juce::dsp::WindowingFunction<float>>(fftSize, juce::dsp::WindowingFunction<float>::blackmanHarris);
        
        fftData.clear();
        fftData.resize(fftSize * 2, 0);

        fftDataFifo.prepare(fftData.size());
    }
    //==============================================================================
    int getFFTSize() const { return 1 << order; }
    int getNumAvailableFFTDataBlocks() const { return fftDataFifo.getNumAvailableForReading(); }
    //==============================================================================
    bool getFFTData(BlockType& fftData) { return fftDataFifo.pull(fftData); }
};

template<typename PathType>
class AnalyzerPathGenerator
{
private:
    Fifo<PathType> pathFifo;

public:
    /*
     converts 'renderData[]' into a juce::Path
     */
    void generatePath(const std::vector<float>& renderData,
                      juce::Rectangle<float> fftBounds,
                      int fftSize,
                      float binWidth,
                      float negativeInfinity)
    {
        auto top = fftBounds.getY();
        auto bottom = fftBounds.getHeight();
        auto width = fftBounds.getWidth();

        int numBins = (int)fftSize / 2;

        PathType p;
        p.preallocateSpace(3 * (int)fftBounds.getWidth());

        auto map = [bottom, top, negativeInfinity](float input)
        {
            return juce::jmap(input,
                              negativeInfinity, 0.f,
                              float(bottom+10),   top);
        };

        auto y = map(renderData[0]);

        if( std::isnan(y) || std::isinf(y) )
            y = bottom;
        
        p.startNewSubPath(0, y);

        const int pathResolution = 2; //you can draw line-to's every 'pathResolution' pixels.

        for( int binNum = 1; binNum < numBins; binNum += pathResolution )
        {
            y = map(renderData[binNum]);

            if( !std::isnan(y) && !std::isinf(y) )
            {
                auto binFreq = binNum * binWidth;
                auto normalizedBinX = juce::mapFromLog10(binFreq, 20.f, 20000.f);
                int binX = std::floor(normalizedBinX * width);
                p.lineTo(binX, y);
            }
        }

        pathFifo.push(p);
    }

    int getNumPathsAvailable() const
    {
        return pathFifo.getNumAvailableForReading();
    }

    bool getPath(PathType& path)
    {
        return pathFifo.pull(path);
    }

};

class FFTPathProducer
{
private:
    using BufferType    = juce::AudioBuffer<float>;
    using SCSF          = SingleChannelSampleFifo<BufferType>;

    SCSF*                                   scsf;
    juce::AudioBuffer<float>                monoBuffer;

    FFTDataGenerator<std::vector<float>>    fftDataGen;
    AnalyzerPathGenerator<juce::Path>       pathProducer;
    juce::Path                              fftPath;

public:
    FFTPathProducer(SCSF& fifo)
        :   scsf(&fifo)
    {
        fftDataGen.changeOrder(FFTOrder::order4096);
        monoBuffer.setSize(1, fftDataGen.getFFTSize());
    }

    void process(const juce::Rectangle<float>& fftBounds, double sampleRate)
    {
        juce::AudioBuffer<float> incomingBuffer;

        while (scsf->getNumCompleteBuffersAvailable() > 0)
        {
            // Try to read the buffer into incomingBuffer
            if (scsf->getAudioBuffer(incomingBuffer))
            {
                int size        = incomingBuffer.getNumSamples();
                int channelNum  = 0;                                // We have only one channel hence index 0

                // Shift left the samples by "size" and put new buffer at the end [newest buffer rightmost]
                // ex : monoBuffer [1 2 3 4 5 6 7] and new data [11 12 13] => size = 3
                // Then after below op, mono buffer [4 5 6 7 5 6 7]
                juce::FloatVectorOperations::copy(
                        monoBuffer.getWritePointer(channelNum, 0),
                        monoBuffer.getReadPointer(channelNum, size),
                        monoBuffer.getNumSamples() - size
                );

                // We now copy in the new buffer at the end
                // Now after below op, mono buffer [4 5 6 7 11 12 13]
                juce::FloatVectorOperations::copy(
                    monoBuffer.getWritePointer(channelNum, monoBuffer.getNumSamples() - size),
                    incomingBuffer.getReadPointer(channelNum, 0),
                    size
                );

                fftDataGen.produceFFTDataForRendering(monoBuffer, -48.0f);
            }
        }

        const auto fftSize      = fftDataGen.getFFTSize();
        const auto binWidth     = sampleRate / (double)fftSize;

        while (fftDataGen.getNumAvailableFFTDataBlocks() > 0)
        {
            std::vector<float> fftData;
            if (fftDataGen.getFFTData(fftData))
            {
                pathProducer.generatePath(fftData, fftBounds, fftSize, binWidth, -48.0f);
            }
        }
        // While there are paths that can be pulled, get as many and use most recent
        while (pathProducer.getNumPathsAvailable())
        {
            pathProducer.getPath(fftPath);
        }
    }

    juce::Path getPath()
    {
        return fftPath;
    }

};

class LookNFeel : public juce::LookAndFeel_V4
{
public:
    void drawRotarySlider(  juce::Graphics&,
                            int x, int y, int width, int height,
                            float sliderPosProportional,
                            float rotaryStartAngle,
                            float rotaryEndAngle,
                            juce::Slider&
                        ) override;

    void drawToggleButton(  juce::Graphics&,
                            juce::ToggleButton& button,
                            bool shouldDrawButtonAsHighlighted,
                            bool shoulDrawButtonAsDown
                        ) override; 
};

class RotarySliderWithLabels : public juce::Slider
{
public:
    // Delegating superclass constructor to initialise the super class members according to the below parameters
    // If we did not do this, juce::Slider would have been default constructed
    RotarySliderWithLabels(juce::RangedAudioParameter& param, const juce::String& unitSuffix) 
        : juce::Slider  (juce::Slider::SliderStyle::RotaryHorizontalVerticalDrag, juce::Slider::TextEntryBoxPosition::NoTextBox),
          parameter     (&param),
          unit          (unitSuffix)
    {
        setLookAndFeel(&lnf);
    }

    ~RotarySliderWithLabels()
    {
        setLookAndFeel(nullptr);
    }

    struct LabelWithPosition
    {
        juce::String            labelText;
        float                   sliderPos;

        float getAngularPosition(const float startAngle, const float endAngle) const
        {
            return jmap (sliderPos, 0.0f, 1.0f, startAngle, endAngle);
        }
    };

    juce::Array<LabelWithPosition>  labels;

    void drawLabelAtPosition(juce::Graphics& g, const LabelWithPosition& label, const Point<float>& labelCenter);
    void paint(Graphics& g) override;
    juce::Rectangle<int> getSliderBounds() const;
    int getTextHeight() const { return 14;}
    juce::String getDisplayString() const;


private:
    LookNFeel lnf;
    juce::RangedAudioParameter* parameter;
    juce::String                unit;
};

class PowerButton : public juce::ToggleButton
{

};

class AnalyzerButton : public juce::ToggleButton
{
private:
    juce::Path randomPath { };

public:
    AnalyzerButton()
    {
        randomPath.clear();
    }

    void resized() override
    {
        juce::Random  randomVal;
        Rectangle<int> bounds = getLocalBounds().reduced(4);
        randomPath.startNewSubPath(bounds.getX(), bounds.getY() + bounds.getHeight() * randomVal.nextFloat());
        for (int x = bounds.getX() + 2; x < bounds.getRight(); x += 2)
        {
            int y = bounds.getY() + bounds.getHeight() * randomVal.nextFloat();

            randomPath.lineTo(x, y);
        }
    } 

    juce::Path getPath()
    {
        return randomPath;
    }
};

class ResponseCurveComponent :  public juce::AudioProcessorParameter::Listener, 
                                public juce::Timer,
                                public juce::Component
{
public:

    ResponseCurveComponent(VstpluginAudioProcessor&);
    ~ResponseCurveComponent();

    void parameterValueChanged   (int parameterIndex, float newValue) override;
    void parameterGestureChanged (int parameterIndex, bool gestureIsStarting) override { /* No Op */ }

    void timerCallback() override;

    void paint(Graphics&) override;
    void resized() override;
    void showFFTAnalysis(bool isEnabled)
    {
        showFFT = isEnabled;   
    }

private:
    // This reference is provided as a quick way for your editor to
    // access the processor object that created it.
    VstpluginAudioProcessor&    processor;
    juce::Atomic<bool>          paramsChanged { false };
    MonoChain                   monoChain;
    juce::Image                 backgroundGrid;
    FFTPathProducer             leftPath;
    FFTPathProducer             rightPath;
    bool                        showFFT { true };
    void updateChain();
    juce::Rectangle<int> getRenderArea();
};

class VstpluginAudioProcessorEditor  :  public AudioProcessorEditor
{
public:
    VstpluginAudioProcessorEditor (VstpluginAudioProcessor&);
    ~VstpluginAudioProcessorEditor();

    //==============================================================================
    void paint (Graphics&) override;
    void resized() override;

private:
    // This reference is provided as a quick way for your editor to
    // access the processor object that created it.
    VstpluginAudioProcessor& processor;

    RotarySliderWithLabels peakFrequencySlider;
    RotarySliderWithLabels peakGainSlider;
    RotarySliderWithLabels peakQualitySlider;

    RotarySliderWithLabels lowCutFrequencySlider;
    RotarySliderWithLabels highCutFrequencySlider;

    RotarySliderWithLabels lowCutSlopeSlider;
    RotarySliderWithLabels highCutSlopeSlider;

    ResponseCurveComponent responseCurveComponent;


    using APVTS = juce::AudioProcessorValueTreeState;

    APVTS::SliderAttachment peakFrequencySliderAttachment;
    APVTS::SliderAttachment peakGainSliderAttachment;
    APVTS::SliderAttachment peakQualitySliderAttachment;

    APVTS::SliderAttachment lowCutFrequencySliderAttachment;
    APVTS::SliderAttachment highCutFrequencySliderAttachment;

    APVTS::SliderAttachment lowCutSlopeSliderAttachment;
    APVTS::SliderAttachment highCutSlopeSliderAttachment;

    PowerButton             lowCutBypassToggle;
    PowerButton             highCutBypassToggle;
    PowerButton             peakBypassToggle;
    AnalyzerButton          analyzerToggle;

    APVTS::ButtonAttachment lowCutBypassToggleAttachment;
    APVTS::ButtonAttachment highCutBypassToggleAttachment;
    APVTS::ButtonAttachment peakBypassToggleAttachment;
    APVTS::ButtonAttachment analyzerToggleAttachment;

    LookNFeel               lnf;

    std::vector<juce::Component*> getComponents();

    JUCE_DECLARE_NON_COPYABLE_WITH_LEAK_DETECTOR (VstpluginAudioProcessorEditor)
};
