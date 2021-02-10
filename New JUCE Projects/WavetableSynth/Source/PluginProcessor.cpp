/*
  ==============================================================================

    This file contains the basic framework code for a JUCE plugin processor.

  ==============================================================================
*/

#include "PluginProcessor.h"
#include "PluginEditor.h"

WavetableOscillator::WavetableOscillator(juce::AudioSampleBuffer wavetableToUse)
    : wavetable(wavetableToUse),
    tableSize(wavetable.getNumSamples() - 1)
{
    jassert(wavetable.getNumChannels() == 1);
}

void WavetableOscillator::setCurrentSampleRate(float sampleRate) {
    currentSampleRate = sampleRate;
}

void WavetableOscillator::setFrequency(float frequency)
{
    auto tableSizeOverSampleRate = (float)tableSize / currentSampleRate;
    tableDelta = frequency * tableSizeOverSampleRate;
}

void WavetableOscillator::init(){
    currentIndex = 0.0f;
}

float WavetableOscillator::getNextSample() noexcept
{
    auto index0 = (unsigned int)currentIndex;
    auto index1 = index0 + 1;

    auto frac = currentIndex - (float)index0;

    auto* table = wavetable.getReadPointer(0);
    auto value0 = table[index0];
    auto value1 = table[index1];

    auto currentSample = value0 + frac * (value1 - value0);

    if ((currentIndex += tableDelta) > (float)tableSize)
        currentIndex -= (float)tableSize;

    return currentSample;
}

void WavetableOscillator::changeWavetable(juce::AudioSampleBuffer& newWavetableToUse) {
    
    wavetable.setSize(1, tableSize, false, true, true);
    auto sampleSrc = newWavetableToUse.getReadPointer(0);
    auto sampleDst = wavetable.getWritePointer(0);
    for (int i = 0; i < tableSize - 1; i++) {
        sampleDst[i] = sampleSrc[i];
    }

    sampleDst[tableSize-1] = sampleDst[0];
}

//==============================================================================
WavetableSynthAudioProcessor::WavetableSynthAudioProcessor()
#ifndef JucePlugin_PreferredChannelConfigurations
     : AudioProcessor (BusesProperties()
                     #if ! JucePlugin_IsMidiEffect
                      #if ! JucePlugin_IsSynth
                       .withInput  ("Input",  juce::AudioChannelSet::stereo(), true)
                      #endif
                       .withOutput ("Output", juce::AudioChannelSet::stereo(), true)
                     #endif
                       )
#endif
{
    for (auto i = 0; i < 10; ++i)
        synth.addVoice(new SineWaveVoice());

    synth.addSound(new SineWaveSound());
}

WavetableSynthAudioProcessor::~WavetableSynthAudioProcessor() {}

//==============================================================================
const juce::String WavetableSynthAudioProcessor::getName() const
{
    return JucePlugin_Name;
}

bool WavetableSynthAudioProcessor::acceptsMidi() const
{
   #if JucePlugin_WantsMidiInput
    return true;
   #else
    return false;
   #endif
    //return true;
}

bool WavetableSynthAudioProcessor::producesMidi() const
{
   #if JucePlugin_ProducesMidiOutput
    return true;
   #else
    return false;
   #endif
    //return false;
}

bool WavetableSynthAudioProcessor::isMidiEffect() const
{
   #if JucePlugin_IsMidiEffect
    return true;
   #else
    return false;
   #endif
    //return true;
}

double WavetableSynthAudioProcessor::getTailLengthSeconds() const
{
    return 0.0;
}

int WavetableSynthAudioProcessor::getNumPrograms()
{
    return 1;   // NB: some hosts don't cope very well if you tell them there are 0 programs,
                // so this should be at least 1, even if you're not really implementing programs.
}

int WavetableSynthAudioProcessor::getCurrentProgram()
{
    return 0;
}

void WavetableSynthAudioProcessor::setCurrentProgram (int index)
{
}

const juce::String WavetableSynthAudioProcessor::getProgramName (int index)
{
    return {};
}

void WavetableSynthAudioProcessor::changeProgramName (int index, const juce::String& newName)
{
}

//==============================================================================
void WavetableSynthAudioProcessor::prepareToPlay (double sampleRate, int samplesPerBlock)
{
    synth.setCurrentPlaybackSampleRate(sampleRate);
}

void WavetableSynthAudioProcessor::releaseResources()
{
    // When playback stops, you can use this as an opportunity to free up any
    // spare memory, etc.
}

#ifndef JucePlugin_PreferredChannelConfigurations
bool WavetableSynthAudioProcessor::isBusesLayoutSupported (const BusesLayout& layouts) const
{
  #if JucePlugin_IsMidiEffect
    juce::ignoreUnused (layouts);
    return true;
  #else
    // This is the place where you check if the layout is supported.
    // In this template code we only support mono or stereo.
    if (layouts.getMainOutputChannelSet() != juce::AudioChannelSet::mono()
     && layouts.getMainOutputChannelSet() != juce::AudioChannelSet::stereo())
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

void WavetableSynthAudioProcessor::processBlock (juce::AudioBuffer<float>& buffer, juce::MidiBuffer& midiMessages)
{
    synth.renderNextBlock( buffer, midiMessages, 0, buffer.getNumSamples() );
}

//==============================================================================
bool WavetableSynthAudioProcessor::hasEditor() const
{
    return true; // (change this to false if you choose to not supply an editor)
}

juce::AudioProcessorEditor* WavetableSynthAudioProcessor::createEditor()
{
    return new WavetableSynthAudioProcessorEditor (*this);
}

//==============================================================================
void WavetableSynthAudioProcessor::getStateInformation (juce::MemoryBlock& destData)
{
    // You should use this method to store your parameters in the memory block.
    // You could do that either as raw data, or use the XML or ValueTree classes
    // as intermediaries to make it easy to save and load complex data.
}

void WavetableSynthAudioProcessor::setStateInformation (const void* data, int sizeInBytes)
{
    // You should use this method to restore your parameters from this memory block,
    // whose contents will have been created by the getStateInformation() call.
}

//==============================================================================
// This creates new instances of the plugin..
juce::AudioProcessor* JUCE_CALLTYPE createPluginFilter()
{
    return new WavetableSynthAudioProcessor();
}
