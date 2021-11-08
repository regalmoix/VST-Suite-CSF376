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

SineWaveVoice::SineWaveVoice(const WavetableSynthAudioProcessor& p) : processor(p)
{
    // Register as a listener
    auto& parameters = processor.getParameters();

    for (auto& param : parameters)
        param->addListener(this);
    
    // Initial call to update ADSR when plugin started
    updateADSRSettings();
    startTimerHz(60);

    // +1 because combobox 0 means no selection so practically indexing begins at 1
    voices = processor.apvts.getRawParameterValue("Voices")->load() + 1;

    initOscillators(processor.getWaveFunction());
}

SineWaveVoice::~SineWaveVoice() 
{
    // De register as the listener
    auto& parameters = processor.getParameters();
    
    for (auto& param : parameters)
        param->removeListener(this);
    
    stopTimer();
}

// ============================================================================== //
AudioSampleBuffer SineWaveVoice::makeWaveTable(std::function<double(double)> waveFunction) const
{
    AudioSampleBuffer wavetable;
    // To accomodate for inclusive range [0, 2pi] we need 1 + Tablesize
    wavetable.setSize(1, TABLESIZE + 1);
    wavetable.clear();

    auto* samples = wavetable.getWritePointer(0);

    auto angleDelta = MathConstants<float>::twoPi / (float)(TABLESIZE);
    auto currentAngle = 0.0;

    // Starts at angle = 0, ends at angle = 2*pi
    for (unsigned int i = 0; i <= TABLESIZE; ++i)
    {   
        samples[i] = (float) waveFunction(currentAngle);
        currentAngle += angleDelta;
        if (currentAngle > MathConstants<float>::twoPi)
            currentAngle -= MathConstants<float>::twoPi;
    }
    
    return wavetable;
}

void SineWaveVoice::initOscillators(std::function<double(double)> waveFunction) 
{
    AudioSampleBuffer wavetable = makeWaveTable(waveFunction);
    /** @todo To give user slider control of the harmonics to play, assign 1 wavetable to 1 oscillator */
    for (int i = 0; i < voices; i++)
        oscillators.add(new WavetableOscillator(wavetable));
}

void SineWaveVoice::startNote(int midiNoteNumber, float velocity, SynthesiserSound*, int currentPitchWheelPosition)
{
    level           = velocity * 0.15;
    adsrState       = ADSRState::Attack;
    envelopeIndex   = 0;
    
    centerFrequency = MidiMessage::getMidiNoteInHertz(midiNoteNumber);

    initialiseVoices(centerFrequency);    
}

void SineWaveVoice::stopNote(float velocity, bool allowTailOff)
{
    if (allowTailOff && adsrState != ADSRState::Release)
        adsrState = ADSRState::Release;
    
    else
    {
        clearCurrentNote();
        adsrState = ADSRState::Stopped;
    }

    envelopeIndex = 0;
    centerFrequency = 0;
}

void SineWaveVoice::renderNextBlock(AudioSampleBuffer& outputBuffer, int startSample, int numSamples) 
{
    for (int sampleNumber = startSample; sampleNumber < startSample + numSamples; sampleNumber++) 
    {
        // If the note is being played.
        if (adsrState == ADSRState::Stopped)
            return;

        /** Not a @bug: https://forums.acoustica.com/viewtopic.php?t=23475
         * Previously thought that : Some logic somwhere is causing voice's gain to be proportional to detune. Low voices are thus lower amplitude*
         * But was later realised that most spectrum analysers have a spectrum slope of 3 - 4.5 dBFS/oct to mimic the frequency response of ears
         * Human tend to hear -6dBFS @ 200Hz to be quieter than -6dBFS @ 2000Hz
        **/
        for (WavetableOscillator* oscillator : oscillators) 
        {
            double currentSample    = oscillator->getNextSample() * level * getEnvelopeGainMultiplier();
            for (int channel = 0; channel < outputBuffer.getNumChannels(); ++channel)
                outputBuffer.addSample(channel, sampleNumber, currentSample);            
        }

        if (adsrState != ADSRState::Sustain)
        {
            envelopeIndex++;

            /** @brief check if index update causes a ADSR phase change */
            if (envelopeIndex > envelopeGainMultipliers[adsrState].size())
                changeADSRPhase();       
        }
    }
}

bool SineWaveVoice::canPlaySound(SynthesiserSound* sound)
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

void SineWaveVoice::parameterGestureChanged (int parameterIndex, bool gestureIsStarting) 
{ 
    /*
        Do Nothing
    */ 
}

void SineWaveVoice::parameterValueChanged(int parameterIndex, float newValue)
{
    /** @note : parameterIndex = 0 for wavetable choice menu */
    if (parameterIndex == 0)
        wavetableChanged.set(true);
    /** @note : parameterIndex = 5 for detune knob */
    else if (parameterIndex == 5)
    {
        LOG("Osc re tuned for new detune");
        detuneChanged.set(true);
    }
    else
        adsrParamsChanged.set(true);
}

void SineWaveVoice::timerCallback()
{
    voices = processor.apvts.getRawParameterValue("Voices")->load() + 1;
    
    /** @brief New voices added or existing ones removed */
    if (voices != oscillators.size())
    {
        if (voices > oscillators.size())
        {
            AudioSampleBuffer wavetable = makeWaveTable(processor.getWaveFunction());

            for (int i = 0; i < voices - oscillators.size(); ++i)
                oscillators.add(new WavetableOscillator(wavetable));
            
            initialiseVoices(centerFrequency);
        }

        else
            oscillators.removeLast(oscillators.size() - voices);
        
        LOG("Voices are now : " << oscillators.size());
    }

    if (detuneChanged.compareAndSetBool(false, true))
        initialiseVoices(centerFrequency);
    
    if (adsrParamsChanged.compareAndSetBool(false, true))
        updateADSRSettings();

    if (wavetableChanged.compareAndSetBool(false, true))
    {
        AudioSampleBuffer wavetable  = makeWaveTable(processor.getWaveFunction());

        for (WavetableOscillator* osc : oscillators)
            osc->changeWavetable(wavetable);
    }
}

void SineWaveVoice::initialiseVoices(double frequency)
{
    float sampleRate    = getSampleRate();

    /** @brief Add new oscillators, with detuning and @todo stereo separatation */
    /** @todo  Change the relationship between detuneRatio and detuneKnob value */
    double detuneRatio = exp (processor.apvts.getRawParameterValue("Detune")->load());

    /** @todo Currently assuming an odd number of voices. extend later for even number of voices 
     *  @brief The following line ensures that the leftmost voice is most detuned
     *         ex if 7 voices, leftmost freq = center / detuneRatio^3 and rightmost freq = center * detuneRatio^3 
    **/
    frequency   /= std::pow(detuneRatio, float(voices - 1) / 2.0); 

    for (WavetableOscillator* oscillator : oscillators) 
    {
        oscillator->init();
        oscillator->setCurrentSampleRate (sampleRate);
        oscillator->setFrequency (frequency);
        frequency *= detuneRatio;
    }
}

void SineWaveVoice::updateADSRSettings()
{
    const double sampleRate         = processor.getSampleRate();
    ADSRSettings envelope           = getADSRSettings(processor.apvts, sampleRate);
    
    setEnvelopeGainMultiplier(envelope);
}

void SineWaveVoice::setEnvelopeGainMultiplier(const ADSRSettings& envelope)
{
    envelopeGainMultipliers[ADSRState::Attack] .clear();
    envelopeGainMultipliers[ADSRState::Decay]  .clear();
    envelopeGainMultipliers[ADSRState::Sustain].clear();
    envelopeGainMultipliers[ADSRState::Release].clear();

    for (uint32 smpl = 0; smpl < envelope.attackDuration; smpl++)
    {
        envelopeGainMultipliers[ADSRState::Attack].add (jmap<double>(smpl, 0, envelope.attackDuration - 1, 0, 1));   
    }

    for (uint32 smpl = 0; smpl < envelope.decayDuration; smpl++)
    {
        envelopeGainMultipliers[ADSRState::Decay].add (jmap<double>(smpl, 0, envelope.decayDuration - 1, 1, envelope.sustainGain));   
    }

    envelopeGainMultipliers[ADSRState::Sustain].add (envelope.sustainGain);

    for (uint32 smpl = 0; smpl < envelope.releaseDuration; smpl++)
    {
        envelopeGainMultipliers[ADSRState::Release].add (jmap<double>(smpl, 0, envelope.releaseDuration - 1, envelope.sustainGain, 0));   
    }
}

double SineWaveVoice::getEnvelopeGainMultiplier() const
{
    return std::max(envelopeGainMultipliers[adsrState][envelopeIndex], 0.0);
}

void SineWaveVoice::changeADSRPhase()
{
    envelopeIndex = 0;

    switch (adsrState)
    {
        case ADSRState::Attack : 
        {
            adsrState = ADSRState::Decay; 
            break;
        }

        case ADSRState::Decay : 
        {
            adsrState = ADSRState::Sustain; 
            break;
        }

        case ADSRState::Sustain : 
        {
            adsrState = ADSRState::Release; 
            break;
        }

        case ADSRState::Release : 
        {
            adsrState = ADSRState::Stopped; 
            clearCurrentNote();
            break;
        }

        case ADSRState::Stopped : 
        {
            adsrState = ADSRState::Attack; 
            break;
        }
    }
}

void SineWaveVoice::forceRefresh()
{
    DBG("Forced a complete refresh of ADSR and Wavetable State");
    adsrParamsChanged.set(true);
    wavetableChanged .set(true);

    /** @brief Force updating which timer callback does. Possible "abuse" */
    timerCallback();
}