/*
  ==============================================================================

    This file was auto-generated!

    It contains the basic framework code for a JUCE plugin processor.

  ==============================================================================
*/

#include "PluginProcessor.h"
#include "PluginEditor.h"

//==============================================================================
VstpluginAudioProcessor::VstpluginAudioProcessor()
#ifndef JucePlugin_PreferredChannelConfigurations
     : AudioProcessor (BusesProperties()
                     #if ! JucePlugin_IsMidiEffect
                      #if ! JucePlugin_IsSynth
                       .withInput  ("Input",  AudioChannelSet::stereo(), true)
                      #endif
                       .withOutput ("Output", AudioChannelSet::stereo(), true)
                     #endif
                       )
#endif
{
}

VstpluginAudioProcessor::~VstpluginAudioProcessor()
{
}

//==============================================================================
const String VstpluginAudioProcessor::getName() const
{
    return JucePlugin_Name;
}

bool VstpluginAudioProcessor::acceptsMidi() const
{
   #if JucePlugin_WantsMidiInput
    return true;
   #else
    return false;
   #endif
}

bool VstpluginAudioProcessor::producesMidi() const
{
   #if JucePlugin_ProducesMidiOutput
    return true;
   #else
    return false;
   #endif
}

bool VstpluginAudioProcessor::isMidiEffect() const
{
   #if JucePlugin_IsMidiEffect
    return true;
   #else
    return false;
   #endif
}

double VstpluginAudioProcessor::getTailLengthSeconds() const
{
    return 0.0;
}

int VstpluginAudioProcessor::getNumPrograms()
{
    return 1;   // NB: some hosts don't cope very well if you tell them there are 0 programs,
                // so this should be at least 1, even if you're not really implementing programs.
}

int VstpluginAudioProcessor::getCurrentProgram()
{
    return 0;
}

void VstpluginAudioProcessor::setCurrentProgram (int index)
{
}

const String VstpluginAudioProcessor::getProgramName (int index)
{
    return {};
}

void VstpluginAudioProcessor::changeProgramName (int index, const String& newName)
{
}

//==============================================================================
void VstpluginAudioProcessor::prepareToPlay (double sampleRate, int samplesPerBlock)
{
    // Use this method as the place to do any pre-playback
    // initialisation that you need..

    juce::dsp::ProcessSpec processSpec;

    // Since we are preparing Mono Filters.
    processSpec.numChannels         = 1;
    processSpec.maximumBlockSize    = samplesPerBlock;
    processSpec.sampleRate          = sampleRate;

    leftChain. prepare(processSpec);
    rightChain.prepare(processSpec);

    updateFilters();

    leftChannelFifo .prepare(samplesPerBlock);
    rightChannelFifo.prepare(samplesPerBlock);
}

void VstpluginAudioProcessor::releaseResources()
{
    // When playback stops, you can use this as an opportunity to free up any
    // spare memory, etc.
}

#ifndef JucePlugin_PreferredChannelConfigurations
bool VstpluginAudioProcessor::isBusesLayoutSupported (const BusesLayout& layouts) const
{
  #if JucePlugin_IsMidiEffect
    ignoreUnused (layouts);
    return true;
  #else
    // This is the place where you check if the layout is supported.
    // In this template code we only support mono or stereo.
    if (layouts.getMainOutputChannelSet() != AudioChannelSet::mono()
     && layouts.getMainOutputChannelSet() != AudioChannelSet::stereo())
        return false;

    // This checks if the input layout matches the output layout
   #if ! JucePlugin_IsSynth
    if (layouts.getMainOutputChannelSet() != layouts.getMainInputChannelSet())
        return false;
   #endif

    return true;
  #endif
}
#endif

void VstpluginAudioProcessor::processBlock (AudioBuffer<float>& buffer, MidiBuffer& midiMessages)
{
    ScopedNoDenormals noDenormals;
    auto totalNumInputChannels  = getTotalNumInputChannels();
    auto totalNumOutputChannels = getTotalNumOutputChannels();

    // In case we have more outputs than inputs, this code clears any output
    // channels that didn't contain input data, (because these aren't
    // guaranteed to be empty - they may contain garbage).
    // This is here to avoid people getting screaming feedback
    // when they first compile a plugin, but obviously you don't need to keep
    // this code if your algorithm always overwrites all the output channels.
    for (auto i = totalNumInputChannels; i < totalNumOutputChannels; ++i)
        buffer.clear (i, 0, buffer.getNumSamples());

    // wrapping raw buffer data in the audio block
    juce::dsp::AudioBlock<float> audioBlock(buffer);

    auto leftBlock  = audioBlock.getSingleChannelBlock(0);
    auto rightBlock = audioBlock.getSingleChannelBlock(1);

    juce::dsp::ProcessContextReplacing<float> leftContext (leftBlock);
    juce::dsp::ProcessContextReplacing<float> rightContext(rightBlock);

    // Run audio through the Filters on both channels
    updateFilters();

    leftChain .process(leftContext);
    rightChain.process(rightContext);

    // After current buffer processed we update FIFO with new buffer.
    leftChannelFifo .update(buffer);
    rightChannelFifo.update(buffer);
}

//==============================================================================
bool VstpluginAudioProcessor::hasEditor() const
{
    return true; // (change this to false if you choose to not supply an editor)
}

AudioProcessorEditor* VstpluginAudioProcessor::createEditor()
{
    return new VstpluginAudioProcessorEditor (*this);
}

//==============================================================================
void VstpluginAudioProcessor::getStateInformation (MemoryBlock& destData)
{
    // You should use this method to store your parameters in the memory block.
    // You could do that either as raw data, or use the XML or ValueTree classes
    // as intermediaries to make it easy to save and load complex data.
    juce::MemoryOutputStream memoryOutputStream(destData, true);
    apvts.state.writeToStream(memoryOutputStream);
}

void VstpluginAudioProcessor::setStateInformation (const void* data, int sizeInBytes)
{
    // You should use this method to restore your parameters from this memory block,
    // whose contents will have been created by the getStateInformation() call.
    juce::ValueTree tree = juce::ValueTree::readFromData(data, sizeInBytes);
    if (tree.isValid())
    {
        apvts.replaceState(tree);
        updateFilters();
    }
}

//==============================================================================

juce::AudioProcessorValueTreeState::ParameterLayout VstpluginAudioProcessor::createParameterLayout()
{
    juce::AudioProcessorValueTreeState::ParameterLayout layout;

    float freqSkewFactor = 0.4;

    // Adding LowCut Frequency Slider with appropriate range and defaults
    layout.add(std::make_unique<juce::AudioParameterFloat>
        (
            "LowCut Freq",
            "LowCut Freq",
            juce::NormalisableRange(20.0f, 20000.0f, 1.0f, freqSkewFactor),
            20.0f
        )
    );

    // Adding HighCut Frequency Slider with appropriate range and defaults
    layout.add(std::make_unique<juce::AudioParameterFloat>
        (
            "HighCut Freq",
            "HighCut Freq",
            juce::NormalisableRange(20.0f, 20000.0f, 1.0f, freqSkewFactor),
            20000.0f
        )
    );

    // Adding Peak Frequency Slider with appropriate range and defaults
    layout.add(std::make_unique<juce::AudioParameterFloat>
        (
            "Peak Freq",
            "Peak Freq",
            juce::NormalisableRange(20.0f, 20000.0f, 1.0f, freqSkewFactor),
            1000.0f
        )
    );

    // Adding Peak Gain Slider with appropriate range and defaults
    layout.add(std::make_unique<juce::AudioParameterFloat>
        (
            "Peak Gain",
            "Peak Gain",
            juce::NormalisableRange(-24.0f, +24.0f, 0.5f, 1.0f),
            0.0f
        )
    );

    // Adding Peak Quality Slider with appropriate range and defaults
    layout.add(std::make_unique<juce::AudioParameterFloat>
        (
            "Peak Q",
            "Peak Q",
            juce::NormalisableRange(0.1f, 10.0f, 0.05f, 1.0f),
            1.0f
        )
    );

    juce::StringArray slopeChoices {"12 db/oct", "24 db/oct", "36 db/oct", "48 db/oct"};

    // Adding Low Cut Slope Gain dropdown with appropriate range and defaults
    layout.add(std::make_unique<juce::AudioParameterChoice>
        (
            "LowCut Slope",
            "LowCut Slope",
            slopeChoices,
            0
        )
    );    
    
    // Adding High Cut Slope dropdown with appropriate range and defaults
    layout.add(std::make_unique<juce::AudioParameterChoice>
        (
            "HighCut Slope",
            "HighCut Slope",
            slopeChoices,
            0
        )
    );

    layout.add(std::make_unique<AudioParameterBool>
        (
            "LowCut Bypass",
            "LowCut Bypass",
            false
        )
    );

    layout.add(std::make_unique<AudioParameterBool>
        (
            "HighCut Bypass",
            "HighCut Bypass",
            false
        )
    );

    layout.add(std::make_unique<AudioParameterBool>
        (
            "Peak Bypass",
            "Peak Bypass",
            false
        )
    );

    layout.add(std::make_unique<AudioParameterBool>
        (
            "Analyzer Enable",
            "Analyzer Enable",
            true
        )
    );

    return layout;
}

ChainSettings getChainSettings (const juce::AudioProcessorValueTreeState& apvts)
{
    ChainSettings settings;

    settings.peakQ          = apvts.getRawParameterValue("Peak Q")->load();
    settings.peakGain       = apvts.getRawParameterValue("Peak Gain")->load();
    settings.peakFreq       = apvts.getRawParameterValue("Peak Freq")->load();

    settings.lowCutFreq     = apvts.getRawParameterValue("LowCut Freq")->load();
    settings.highCutFreq    = apvts.getRawParameterValue("HighCut Freq")->load();
    
    settings.lowCutSlope    = static_cast<Slope>(apvts.getRawParameterValue("LowCut Slope")->load());
    settings.highCutSlope   = static_cast<Slope>(apvts.getRawParameterValue("HighCut Slope")->load());

    settings.lowCutBypass   = apvts.getRawParameterValue("LowCut Bypass")->load() > 0.5f;
    settings.highCutBypass  = apvts.getRawParameterValue("HighCut Bypass")->load() > 0.5f;
    settings.peakBypass     = apvts.getRawParameterValue("Peak Bypass")->load() > 0.5f;

    return settings;
}

void updateCoefficients(CoefficientPtr& old, const CoefficientPtr& replacement) 
{
    *old = *replacement;
}

CoefficientPtr makePeakFilter(const ChainSettings& settings, const double sampleRate)
{

    return juce::dsp::IIR::Coefficients<float>::makePeakFilter (sampleRate, 
                                                                settings.peakFreq, 
                                                                settings.peakQ, 
                                                                juce::Decibels::decibelsToGain(settings.peakGain));
}

CoefficientArr makeLowCutFilter(const ChainSettings& settings, const double sampleRate)
{
    /** Order   Slope
     *    2     12db/oct
     *    4     24db/oct
     *    6     36db/oct
     *    8     48db/oct
     */
    const int lowCutOrder = 2 * settings.lowCutSlope + 2; 
    return juce::dsp::FilterDesign<float>::designIIRHighpassHighOrderButterworthMethod(settings.lowCutFreq, sampleRate, lowCutOrder);
}

CoefficientArr makeHighCutFilter(const ChainSettings& settings, const double sampleRate)
{
    /** Order   Slope
     *    2     12db/oct
     *    4     24db/oct
     *    6     36db/oct
     *    8     48db/oct
     */
    const int highCutOrder = 2 * settings.highCutSlope + 2;
    return juce::dsp::FilterDesign<float>::designIIRLowpassHighOrderButterworthMethod (settings.highCutFreq, sampleRate, highCutOrder);
}

template<typename FilterChainType, typename CoefficientType>
void updateCutFilter(FilterChainType& lowCut, const CoefficientType& cutCoefficients, const Slope slope)
{
    // Bypass all chains by default and only enable ones necessary based on slope
    lowCut.template setBypassed<0>(true);
    lowCut.template setBypassed<1>(true);
    lowCut.template setBypassed<2>(true);
    lowCut.template setBypassed<3>(true);

    // Do not add break since each case intentionally wants all below cases to be run
    switch (slope)
    {
        case Slope::Slope48:
        {
            updateCoefficients(lowCut.template get<3>().coefficients, cutCoefficients[3]);
            lowCut.template setBypassed<3>(false);
        }

        case Slope::Slope36:
        {
            updateCoefficients(lowCut.template get<2>().coefficients, cutCoefficients[2]);
            lowCut.template setBypassed<2>(false);
        }

        case Slope::Slope24:
        {
            updateCoefficients(lowCut.template get<1>().coefficients, cutCoefficients[1]);
            lowCut.template setBypassed<1>(false);
        }

        case Slope::Slope12:
        {
            updateCoefficients(lowCut.template get<0>().coefficients, cutCoefficients[0]);
            lowCut.template setBypassed<0>(false);
        }

        default:
        {
            break;
        }
    }
}
//==============================================================================

void VstpluginAudioProcessor::updatePeakFilter (const ChainSettings& settings)
{
    CoefficientPtr peakCoeff = makePeakFilter(settings, getSampleRate());

    leftChain .setBypassed<ChainPostitions::Peak>(settings.peakBypass);
    rightChain.setBypassed<ChainPostitions::Peak>(settings.peakBypass);
    // Get the reference to the processing peak filter of Left chain
    // TODO: Possible to do different processing for both channels 
    updateCoefficients(leftChain .get<ChainPostitions::Peak>().coefficients, peakCoeff);
    updateCoefficients(rightChain.get<ChainPostitions::Peak>().coefficients, peakCoeff);

}

void VstpluginAudioProcessor::updateLowCutFilter(const ChainSettings& settings)
{   
    auto  lowCutCoefficients    = makeLowCutFilter(settings, getSampleRate());

    auto& rightLowCut           = rightChain.get<ChainPostitions::LowCut> ();
    auto& leftLowCut            = leftChain .get<ChainPostitions::LowCut> ();

    leftChain .setBypassed<ChainPostitions::LowCut>(settings.lowCutBypass);
    rightChain.setBypassed<ChainPostitions::LowCut>(settings.lowCutBypass);

    updateCutFilter (rightLowCut, lowCutCoefficients, settings.lowCutSlope);
    updateCutFilter (leftLowCut , lowCutCoefficients, settings.lowCutSlope);
}

void VstpluginAudioProcessor::updateHighCutFilter(const ChainSettings& settings)
{
    auto  highCutCoefficients   = makeHighCutFilter(settings, getSampleRate());

    auto& rightHighCut          = rightChain.get<ChainPostitions::HighCut>();
    auto& leftHighCut           = leftChain .get<ChainPostitions::HighCut>();

    leftChain .setBypassed<ChainPostitions::HighCut>(settings.highCutBypass);
    rightChain.setBypassed<ChainPostitions::HighCut>(settings.highCutBypass);

    updateCutFilter (rightHighCut, highCutCoefficients, settings.highCutSlope);
    updateCutFilter (leftHighCut , highCutCoefficients, settings.highCutSlope);
}

void VstpluginAudioProcessor::updateFilters()
{
    ChainSettings settings  = getChainSettings(apvts);

    updateLowCutFilter  (settings);
    updatePeakFilter    (settings);
    updateHighCutFilter (settings);
}
//==============================================================================
// This creates new instances of the plugin..
AudioProcessor* JUCE_CALLTYPE createPluginFilter()
{
    return new VstpluginAudioProcessor();
}
