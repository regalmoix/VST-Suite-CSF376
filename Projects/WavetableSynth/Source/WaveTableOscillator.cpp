/*
    ==============================================================================

    WaveTableOscillator.cpp
    Created: 11 Sep 2021 11:13:53pm
    Author:  regalmoix

    ==============================================================================
*/

#include "PluginProcessor.h"

WavetableOscillator::WavetableOscillator(AudioSampleBuffer wavetableToUse)
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

/**
 * @brief Sets oscillator frequency by modifying tableDelta 
 * @details for delta d = 1, we process r samples per sec = r / T cycles per second [r = sampleRate, T = tableSize]
 *          so  for   d = f * T / r, we process f cycles per second giving f Hz sound
 * 
 * @param frequency 
 */
void WavetableOscillator::setFrequency(float frequency)
{
    const float tableSizeOverSampleRate = (float) tableSize / currentSampleRate;
    tableDelta = frequency * tableSizeOverSampleRate;
}

/**
 * @brief Gets the next sample from the table and increments the currentIndex.
 * @return the sample needed
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
 * @brief Update the current index, wrapping around the table.
 */
void WavetableOscillator::updateIndex()
{
    currentIndex += tableDelta;

    if (currentIndex > (float)tableSize)
        currentIndex -= (float)tableSize; 
}

void WavetableOscillator::changeWavetable(const AudioSampleBuffer& newWavetableToUse) 
{
    /**
     *  change table size to new tablesize
     *  Divide by the original tableSize, set the new tablesize and multiply by it
     */
    tableDelta /= tableSize;
    tableSize   = newWavetableToUse.getNumSamples() - 1;
    tableDelta *= tableSize;

    wavetable = newWavetableToUse;
    // Set the last sample to be same as the 0th sample to ensure "continuity"
    wavetable.setSample(0, tableSize, wavetable.getSample(0, 0));
}
