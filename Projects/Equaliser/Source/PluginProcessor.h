/*
  ==============================================================================

    This file was auto-generated!

    It contains the basic framework code for a JUCE plugin processor.

  ==============================================================================
*/

#pragma once

#include <JuceHeader.h>
#include <array>

/**  ENUMS **/
enum Slope
{
    Slope12, 
    Slope24, 
    Slope36, 
    Slope48
};

// Enum for easy access to Chain Elements of a Mono Chain
enum ChainPostitions
{
    LowCut,
    Peak,
    HighCut
};

enum Channel
{
    Right, //effectively 0
    Left //effectively 1
};


/**  TYPE ALIASES **/
using Filter    = juce::dsp::IIR::Filter<float>;
using CutFilter = juce::dsp::ProcessorChain <Filter, Filter, Filter, Filter>;

using MonoChain = juce::dsp::ProcessorChain <CutFilter, Filter, CutFilter>;
using CoefficientPtr = Filter::CoefficientsPtr;
using CoefficientArr = juce::ReferenceCountedArray<juce::dsp::IIR::Coefficients<float>>;

struct ChainSettings
{
    float peakFreq      { 0.0f };
    float peakGain      { 0.0f };
    float peakQ         { 0.0f };
    
    float lowCutFreq    { 0.0f };
    float highCutFreq   { 0.0f };

    Slope lowCutSlope   { Slope::Slope12 };
    Slope highCutSlope  { Slope::Slope12 };

    bool  lowCutBypass  { false };
    bool  highCutBypass { false };
    bool  peakBypass    { false };
};

ChainSettings getChainSettings (const juce::AudioProcessorValueTreeState& apvts);

void updateCoefficients (CoefficientPtr& old, const CoefficientPtr& replacement);

CoefficientPtr makePeakFilter   (const ChainSettings& settings, const double sampleRate);
CoefficientArr makeLowCutFilter (const ChainSettings& settings, const double sampleRate);
CoefficientArr makeHighCutFilter(const ChainSettings& settings, const double sampleRate);

template<typename FilterChainType, typename CoefficientType>
void updateCutFilter (FilterChainType& lowCut, const CoefficientType& cutCoefficients, const Slope slope);
  
template<typename T>
class Fifo
{
public:
    void prepare(int numChannels, int numSamples)
    {
        static_assert( std::is_same_v<T, juce::AudioBuffer<float>>,
                      "prepare(numChannels, numSamples) should only be used when the Fifo is holding juce::AudioBuffer<float>");
        for( auto& buffer : buffers)
        {
            buffer.setSize(numChannels,
                           numSamples,
                           false,   //clear everything?
                           true,    //including the extra space?
                           true);   //avoid reallocating if you can?
            buffer.clear();
        }
    }
    
    void prepare(size_t numElements)
    {
        static_assert (std::is_same_v<T, std::vector<float>>,
                      "prepare(numElements) should only be used when the Fifo is holding std::vector<float>");
        for(auto& buffer : buffers)
        {
            buffer.clear();
            buffer.resize(numElements, 0);
        }
    }
    
    bool push(const T& t)
    {
        auto write = fifo.write(1);
        if( write.blockSize1 > 0 )
        {
            buffers[write.startIndex1] = t;
            return true;
        }
        
        return false;
    }
    
    bool pull(T& t)
    {
        auto read = fifo.read(1);
        if( read.blockSize1 > 0 )
        {
            t = buffers[read.startIndex1];
            return true;
        }
        
        return false;
    }
    
    int getNumAvailableForReading() const
    {
        return fifo.getNumReady();
    }

private:
    static constexpr int Capacity = 30;
    std::array<T, Capacity> buffers;
    juce::AbstractFifo fifo {Capacity};
};

template<typename BlockType>
class SingleChannelSampleFifo
{
public:
    SingleChannelSampleFifo(Channel ch) : channelToUse(ch)
    {
        prepared.set(false);
    }
    
    void update(const BlockType& buffer)
    {
        jassert(prepared.get());
        jassert(buffer.getNumChannels() > channelToUse );
        auto* channelPtr = buffer.getReadPointer(channelToUse);
        
        for( int i = 0; i < buffer.getNumSamples(); ++i )
        {
            pushNextSampleIntoFifo(channelPtr[i]);
        }
    }

    void prepare(int bufferSize)
    {
        prepared.set(false);
        size.set(bufferSize);
        bufferToFill.setSize(1,             //channel
                             bufferSize,    //num samples
                             false,         //keepExistingContent
                             true,          //clear extra space
                             true);         //avoid reallocating
        audioBufferFifo.prepare(1, bufferSize);
        fifoIndex = 0;
        prepared.set(true);
    }
    //==============================================================================
    int getNumCompleteBuffersAvailable() const { return audioBufferFifo.getNumAvailableForReading(); }
    bool isPrepared() const { return prepared.get(); }
    int getSize() const { return size.get(); }
    //==============================================================================
    bool getAudioBuffer(BlockType& buf) { return audioBufferFifo.pull(buf); }
private:
    Channel channelToUse;
    int fifoIndex = 0;
    Fifo<BlockType> audioBufferFifo;
    BlockType bufferToFill;
    juce::Atomic<bool> prepared = false;
    juce::Atomic<int> size = 0;
    
    void pushNextSampleIntoFifo(float sample)
    {
        if (fifoIndex == bufferToFill.getNumSamples())
        {
            auto ok = audioBufferFifo.push(bufferToFill);

            juce::ignoreUnused(ok);
            
            fifoIndex = 0;
        }
        
        bufferToFill.setSample(0, fifoIndex, sample);
        ++fifoIndex;
    }
};

class VstpluginAudioProcessor  : public AudioProcessor
{
public:
    //==============================================================================
    VstpluginAudioProcessor();
    ~VstpluginAudioProcessor();

    //==============================================================================
    void prepareToPlay (double sampleRate, int samplesPerBlock) override;
    void releaseResources() override;

   #ifndef JucePlugin_PreferredChannelConfigurations
    bool isBusesLayoutSupported (const BusesLayout& layouts) const override;
   #endif

    void processBlock (AudioBuffer<float>&, MidiBuffer&) override;

    //==============================================================================
    AudioProcessorEditor* createEditor() override;
    bool hasEditor() const override;

    //==============================================================================
    const String getName() const override;

    bool acceptsMidi() const override;
    bool producesMidi() const override;
    bool isMidiEffect() const override;
    double getTailLengthSeconds() const override;

    //==============================================================================
    int getNumPrograms() override;
    int getCurrentProgram() override;
    void setCurrentProgram (int index) override;
    const String getProgramName (int index) override;
    void changeProgramName (int index, const String& newName) override;

    //==============================================================================
    void getStateInformation (MemoryBlock& destData) override;
    void setStateInformation (const void* data, int sizeInBytes) override;
    
    // Since the function doesnt depend on any member variables.
    static juce::AudioProcessorValueTreeState::ParameterLayout createParameterLayout();

    juce::AudioProcessorValueTreeState apvts { *this, nullptr, "Parameters", createParameterLayout() };
    
    SingleChannelSampleFifo<juce::AudioBuffer<float>>   leftChannelFifo  {Channel::Left};
    SingleChannelSampleFifo<juce::AudioBuffer<float>>   rightChannelFifo {Channel::Right};

private:
    // To accomodate for stereo we need both left and right chains
    MonoChain leftChain;
    MonoChain rightChain;


    void updateFilters      ();
    void updateLowCutFilter (const ChainSettings& settings);
    void updateHighCutFilter(const ChainSettings& settings);
    void updatePeakFilter   (const ChainSettings& settings);

  

    //==============================================================================
    JUCE_DECLARE_NON_COPYABLE_WITH_LEAK_DETECTOR (VstpluginAudioProcessor)
};
