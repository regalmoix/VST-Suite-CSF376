/*
==============================================================================

SineWaveVoice.cpp
Created: 11 Sep 2021 9:37:54pm
Author:  regalmoix

==============================================================================
*/

#include "PluginProcessor.h"

bool SineWaveSound::appliesToNote(int midiNoteNumber)
{
    return true;
}

bool SineWaveSound::appliesToChannel(int midiChannel)
{
    return true;
}

// ============================================================================== //

SineWaveVoice::SineWaveVoice() 
{
    initOscillators();
}

SineWaveVoice::~SineWaveVoice() 
{
    /*
        Do Nothing
    */
}

// ============================================================================== //

void SineWaveVoice::initOscillators() 
{
    juce::AudioBuffer<float> wavetable;
    // To accomodate for inclusive range [0, 2pi] we need 1 + Tablesize
    wavetable.setSize(1, TABLESIZE + 1);
    wavetable.clear();

    auto* samples = wavetable.getWritePointer(0);

    auto angleDelta = juce::MathConstants<float>::twoPi / (float)(TABLESIZE);
    auto currentAngle = 0.0;

    // Starts at angle = 0, ends at angle = 2*pi
    for (unsigned int i = 0; i <= TABLESIZE; ++i)
    {   
        /** @todo : replace std::sin with a function parameter to init oscillator */
        samples[i] = (float)std::sin(currentAngle);
        currentAngle += angleDelta;
        if (currentAngle > juce::MathConstants<float>::twoPi)
            currentAngle -= juce::MathConstants<float>::twoPi;
    }

    /** @brief To give user slider control of the harmonics to play, assign 1 wavetable to 1 oscillator */
    oscillators.add(new WavetableOscillator(wavetable));
}

void SineWaveVoice::startNote(int midiNoteNumber, float velocity, juce::SynthesiserSound*, int currentPitchWheelPosition)
{
    level       = velocity * 0.15;
    tailOff     = 0.0;
    playNote    = true;

    float sampleRate    = getSampleRate();
    float oscFrequency  = MidiMessage::getMidiNoteInHertz(midiNoteNumber);

    for (WavetableOscillator* oscillator : oscillators) 
    {
        oscillator->init();
        oscillator->setCurrentSampleRate (sampleRate);
        oscillator->setFrequency (oscFrequency);
    }
}

void SineWaveVoice::stopNote(float velocity, bool allowTailOff)
{
    if (allowTailOff && tailOff == 0.0)
        tailOff = 1.0;
    
    else
    {
        clearCurrentNote();
        playNote = false;
    }
}

void SineWaveVoice::renderNextBlock(juce::AudioSampleBuffer& outputBuffer, int startSample, int numSamples) 
{
    if (!playNote)
        return;

    // If the note is being played.
    for (int sampleNumber = startSample; sampleNumber < startSample + numSamples; sampleNumber++) 
    {
        for (WavetableOscillator* oscillator : oscillators) 
        {
            double currentSample    = oscillator->getNextSample() * level * (tailOff > 0.0 ? tailOff : 1.0);

            for (int channel = 0; channel < outputBuffer.getNumChannels(); ++channel)
                outputBuffer.addSample(channel, sampleNumber, currentSample);            
        }

        // If tailoff is 0 => Note is playing in normal mode.
        // If tailoff != 0 => Note is stopped and now we are in tailoff mode
        // We keep decreasing gain by factor of 0.99 every sample, and cut off the volumne when level falls below  0.5%

        if (tailOff > 0.0) 
        {   
            tailOff *= 0.99;
            if (tailOff <= 0.005) 
            {
                clearCurrentNote();
                playNote = false;
                break;
            }
        }
    }
}

bool SineWaveVoice::canPlaySound(juce::SynthesiserSound* sound)
{
    return dynamic_cast<SineWaveSound*> (sound) != nullptr;
}

void SineWaveVoice::pitchWheelMoved (int)
{
    /*
        Do Nothing
    */
}

void SineWaveVoice::controllerMoved (int, int)
{
    /*
        Do Nothing
    */
}
