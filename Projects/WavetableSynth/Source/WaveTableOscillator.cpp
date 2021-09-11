/*
  ==============================================================================

    WaveTableOscillator.cpp
    Created: 11 Sep 2021 11:13:53pm
    Author:  regalmoix

  ==============================================================================
*/

#include "PluginProcessor.h"

WavetableOscillator::WavetableOscillator(juce::AudioSampleBuffer wavetableToUse)
    :   wavetable (wavetableToUse),
        tableSize (wavetableToUse.getNumSamples() - 1)
{
    jassert(wavetable.getNumChannels() == 1);
}

void WavetableOscillator::init()
{
    currentIndex = 0.0f;
}

void WavetableOscillator::setCurrentSampleRate(float sampleRate) 
{
    currentSampleRate = sampleRate;
}

void WavetableOscillator::setFrequency(float frequency)
{
    const float tableSizeOverSampleRate = (float) tableSize / currentSampleRate;
    tableDelta = frequency * tableSizeOverSampleRate;
}

/**
 * Gets the next sample from the table and increments the currentIndex.
*/
float WavetableOscillator::getNextSample() noexcept
{
    const float* table  = wavetable.getReadPointer(0);

    // Interpolate 2 samples : ex, if current index = 3.21
    uint32 index0       = (uint32) currentIndex;
    uint32 index1       = index0 + 1;

    // idx0 = 3, idx1 = 4, frac = 0.21
    float  frac         = currentIndex - (float)index0;

    // Read sample 3, sample 4
    float  value0       = table[index0];
    float  value1       = table[index1];

    // Take weighted average between samples 3 and sample 4
    // 79% of 3rd sample, 21% of 4th sample 
    float currentSample = (1 - frac) * value0 + frac * value1;

    updateIndex(); 

    return currentSample;
}

/**
 * Updates the current index, wrapping around the table.
 */
void WavetableOscillator::updateIndex()
{
    currentIndex += tableDelta;

    if (currentIndex > (float)tableSize)
        currentIndex -= (float)tableSize; 
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
