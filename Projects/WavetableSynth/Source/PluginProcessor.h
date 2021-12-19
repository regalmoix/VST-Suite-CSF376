/*
  ==============================================================================

    This file contains the basic framework code for a JUCE plugin processor.

  ==============================================================================
*/

#pragma once

#include <JuceHeader.h>
#include <algorithm>
#define WAVETABLE_CNT 5
#define OSC_CNT 3
#define TABLESIZE 44100
#define LOGGING

#ifdef LOGGING
    #define LOG(txt) DBG("[" << __FUNCTION__ << "] " << txt)
#else
    #define LOG(txt) 
#endif

enum ADSRState
{
    Attack,
    Decay,
    Sustain,
    Release,
    Stopped
};

class WavetableSynthAudioProcessor;

// Stores values associated with a ADSR GUI Knobs
struct ADSRSettings
{
    // All durations are in terms of samples
    uint32  attackDuration  { 0 };
    uint32  decayDuration   { 0 };
    float   sustainGain     { 0.0f };
    uint32  releaseDuration { 0 };
};

class WavetableOscillator
{
public:
    /**
     * @brief Construct a new Wavetable Oscillator object
     */
    WavetableOscillator(AudioSampleBuffer);

    void    setCurrentSampleRate (float sampleRate);
    void    init ();
    void    setFrequency (float frequency);
    void    setPhase     (float normPhase);
    float   getNextSample () noexcept;
    void    changeWavetable (const AudioSampleBuffer& newWavetableToUse);
    
private:
    AudioSampleBuffer   wavetable;
    int                 tableSize;
    float               currentIndex    { 0.0f };
    float               tableDelta      { 0.0f };
    float               currentSampleRate;

    void    updateIndex();
};

class SineWaveSound : public SynthesiserSound 
{
public:
    bool    appliesToNote (int midiNoteNumber) override;
    bool    appliesToChannel (int midiChannel) override;
};

/**
 * @brief Voice that can be used by the Synth. Use multiple voices for polyphony
 * @link https://docs.juce.com/master/tutorial_wavetable_synth.html @endlink
 */
class SineWaveVoice :   public SynthesiserVoice,  
                        public AudioProcessorParameter::Listener, 
                        public Timer
{
private:
    OwnedArray<WavetableOscillator>         oscillators;
    const WavetableSynthAudioProcessor&     processor;

    /** @todo : Add Initial Phase and Random Phase Range (see Xfer Serum) */

    int             voices              { 1 };
    double          level               { 0.0f };
    double          centerFrequency     { 0.0f };
    ADSRState       adsrState           { ADSRState::Stopped };
    uint32          envelopeIndex       { 0 };                  // Sample Index used to query the envelope gain multiplier 
    Atomic<bool>    adsrParamsChanged   { false };
    Atomic<bool>    wavetableChanged    { false };
    Atomic<bool>    detuneChanged       { false };

    Array <double>  envelopeGainMultipliers[4];
    
public:
    SineWaveVoice (const WavetableSynthAudioProcessor& p);

    ~SineWaveVoice();

    void initOscillators(std::function<double(double)> wave);

    bool canPlaySound(SynthesiserSound* sound) override;

    void startNote (int midiNoteNumber, float velocity, SynthesiserSound*, int currentPitchWheelPosition) override;
    void stopNote  (float velocity, bool allowTailOff) override;

    void pitchWheelMoved (int newPitchWheelValue) override;
    void controllerMoved (int controllerNumber, int newControllerValue) override;

    /** 
       Renders the next block of data for this voice.

        The output audio data is added to the current contents of the buffer provided.
        Modifies the buffer between startSample and (startSample + numSamples)

        If the voice is currently silent, it should just return without doing anything.

        If the sound that the voice is playing finishes during the course of this rendered
        block, it must call clearCurrentNote(), to tell the synthesiser that it has finished.
    */
    void renderNextBlock (AudioSampleBuffer& outputBuffer, int startSample, int numSamples) override;

    /**
     * @brief forces an update of the ADSR Settings and the Wavetable Choice.
     */
    void forceRefresh    ();

private:

    /** 
     * @brief called every timer tick to update ADSR Envelope values which are used to call setEnvelopeGainMultiplier()  
     */
    void updateADSRSettings ();
    
    /**
     * @brief Causes a state transition Attack -> Decay -> Sustain -> Release -> Stopped -> Attack. 
              This also resets the envelope index to 0, and clearCurrentNote() if we transition to stopped phase
     */
    void changeADSRPhase    ();

    void parameterValueChanged   (int parameterIndex, float newValue) override;
    void parameterGestureChanged (int parameterIndex, bool gestureIsStarting) override;

    /**
     * @brief Called every time tick. Is used to update member param, often when parameters we listen to change
     */
    void timerCallback() override;

    /**
     * @brief Set the Envelope Gain Multipliers to modify volume conforming to the attack, decay, release durations and sustain level 
     * 
     * @param newEnvelope The new ADSR values
     */
    void setEnvelopeGainMultiplier(const ADSRSettings& newEnvelope);
    
    /**
     * @brief Get the Envelope Gain Multiplier according to the current phase (A/D/S/R) and the position in the phase
     * 
     * @return Gain Multiplier
     */
    double getEnvelopeGainMultiplier() const;

    /**
     * @brief Makes an AudioBuffer to hold the wavetable
     * 
     * @param waveFunction 
     * @return waveTable 
     */
    AudioSampleBuffer makeWaveTable(std::function<double(double)> waveFunction) const;

    void initialiseVoices(double frequency);
};

class WavetableSynthAudioProcessor  : public AudioProcessor
{
public:
    //==============================================================================
    WavetableSynthAudioProcessor ();
    ~WavetableSynthAudioProcessor() override;

    //==============================================================================
    void prepareToPlay      (double sampleRate, int samplesPerBlock)    override;
    void releaseResources   ()                                          override;

    //==============================================================================
#ifndef JucePlugin_PreferredChannelConfigurations
    bool isBusesLayoutSupported (const BusesLayout& layouts) const override;
#endif

    //==============================================================================
    void processBlock (AudioBuffer<float>&, MidiBuffer&) override;

    //==============================================================================
    AudioProcessorEditor* createEditor()    override;
    bool            hasEditor()       const override;

    //==============================================================================
    const String    getName()       const override;
    bool            acceptsMidi()   const override;
    bool            producesMidi()  const override;
    bool            isMidiEffect()  const override;
    double          getTailLengthSeconds() const override;

    //==============================================================================
    int     getNumPrograms()    override;
    int     getCurrentProgram() override;
    void    setCurrentProgram (int index)       override;
    const   String getProgramName (int index)   override;
    void    changeProgramName (int index, const String& newName) override;

    //==============================================================================
    void getStateInformation (MemoryBlock& destData)                override;
    void setStateInformation (const void* data, int sizeInBytes)    override;

    //==============================================================================
    using APVTS = AudioProcessorValueTreeState;
    static APVTS::ParameterLayout createParameterLayout();
    APVTS  apvts { *this, nullptr, "Parameters", createParameterLayout() };

    //==============================================================================
    std::function<double(double)> getWaveFunction() const;

private:
    //==============================================================================
    Synthesiser synth;
    std::array<std::function<double(double)>, WAVETABLE_CNT> waveLambdas;

    JUCE_DECLARE_NON_COPYABLE_WITH_LEAK_DETECTOR (WavetableSynthAudioProcessor)
};

/**
 * @brief Sets the value of sustain gain, and, attack, decay, release durations [in terms of number of samples]
 * 
 * @param apvts The Processor Value Tree
 * @param sampleRate 
 * @return ADSRSettings 
 */
ADSRSettings getADSRSettings (const AudioProcessorValueTreeState& apvts, const double sampleRate);