/*
  ==============================================================================

    This file contains the basic framework code for a JUCE plugin processor.

  ==============================================================================
*/

#include "PluginProcessor.h"
#include "PluginEditor.h"

//==============================================================================
NoteSliderAudioProcessor::NoteSliderAudioProcessor()
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
    speeds = new SpeedValues();
    speeds->speed_choice = 0;
    speeds->change_time = 1.0f;
    speeds->freq_speed_per_second = 20.0f;
    speeds->note_speed_per_second = 1.0f;
    speeds->vel_based_speed = 1.0f;
    speeds->pp_omega = 5.0f;
    speeds->pp_zeta = 0.25f;
    update_pingpong();

    synthVoices[0] = new SineWaveVoice(speeds);

    for( int i = 1; i < 5; i++ ){
        synthVoices[i] = new OtherSineWaveVoice(synthVoices[i - 1], speeds);
    }
    for( int i = 0; i < 5; i++ ){
        synth.addVoice(synthVoices[i]);
    }

    sineSound = new SineWaveSound();

    synth.addSound(sineSound);
}

NoteSliderAudioProcessor::~NoteSliderAudioProcessor()
{
    synth.clearSounds();
    synth.clearVoices();
    delete speeds;
    speeds = NULL;
}

//==============================================================================
const juce::String NoteSliderAudioProcessor::getName() const
{
    return JucePlugin_Name;
}

bool NoteSliderAudioProcessor::acceptsMidi() const
{
   #if JucePlugin_WantsMidiInput
    return true;
   #else
    return false;
   #endif
}

bool NoteSliderAudioProcessor::producesMidi() const
{
   #if JucePlugin_ProducesMidiOutput
    return true;
   #else
    return false;
   #endif
}

bool NoteSliderAudioProcessor::isMidiEffect() const
{
   #if JucePlugin_IsMidiEffect
    return true;
   #else
    return false;
   #endif
}

double NoteSliderAudioProcessor::getTailLengthSeconds() const
{
    return 0.0;
}

int NoteSliderAudioProcessor::getNumPrograms()
{
    return 1;   // NB: some hosts don't cope very well if you tell them there are 0 programs,
                // so this should be at least 1, even if you're not really implementing programs.
}

int NoteSliderAudioProcessor::getCurrentProgram()
{
    return 0;
}

void NoteSliderAudioProcessor::setCurrentProgram (int index)
{
}

const juce::String NoteSliderAudioProcessor::getProgramName (int index)
{
    return {};
}

void NoteSliderAudioProcessor::changeProgramName (int index, const juce::String& newName)
{
}

//==============================================================================
void NoteSliderAudioProcessor::prepareToPlay (double sampleRate, int samplesPerBlock)
{
    currSamplingRate = sampleRate;
    synth.setCurrentPlaybackSampleRate(sampleRate);
}

void NoteSliderAudioProcessor::releaseResources()
{
    // When playback stops, you can use this as an opportunity to free up any
    // spare memory, etc.
}

#ifndef JucePlugin_PreferredChannelConfigurations
bool NoteSliderAudioProcessor::isBusesLayoutSupported (const BusesLayout& layouts) const
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

void NoteSliderAudioProcessor::processBlock (juce::AudioBuffer<float>& buffer, juce::MidiBuffer& midiMessages)
{
    synth.renderNextBlock(buffer, midiMessages, 0, buffer.getNumSamples());
}

//==============================================================================
bool NoteSliderAudioProcessor::hasEditor() const
{
    return true; // (change this to false if you choose to not supply an editor)
}

juce::AudioProcessorEditor* NoteSliderAudioProcessor::createEditor()
{
    return new NoteSliderAudioProcessorEditor (*this);
}

//==============================================================================
void NoteSliderAudioProcessor::getStateInformation (juce::MemoryBlock& destData)
{
    // You should use this method to store your parameters in the memory block.
    // You could do that either as raw data, or use the XML or ValueTree classes
    // as intermediaries to make it easy to save and load complex data.
}

void NoteSliderAudioProcessor::setStateInformation (const void* data, int sizeInBytes)
{
    // You should use this method to restore your parameters from this memory block,
    // whose contents will have been created by the getStateInformation() call.
}

//==============================================================================
// This creates new instances of the plugin..
juce::AudioProcessor* JUCE_CALLTYPE createPluginFilter()
{
    return new NoteSliderAudioProcessor();
}
