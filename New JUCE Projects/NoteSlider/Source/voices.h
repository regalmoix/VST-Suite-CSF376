//Custom Voices and Sound for Synthesizer

#include <JuceHeader.h>

#define SPEED 10000

struct SineWaveSound : public juce::SynthesiserSound {
    SineWaveSound() {}
    bool appliesToNote(int) override {
        return true;
    }
    bool appliesToChannel(int) override {
        return true;
    }
};

//5 speed choices can be made:
//  First one is default: note finishes sliding in 10000 samples
//  Select how much time it should take for portamento (note sliding)
//  Select a constant frequency step per sample
//  Select a constant note number step per sample
//  Select a frequency step based on the velocity of the second note
//      --can be inversely or directly related to the velocity
//      --you can change the sensitivity
//  Use a differential equation to change frequency to desired frequency
enum class SpeedChoice {
    CONST_SAMPLES = 0,
    SLIDER_TIME,
    SLIDER_CONST_FREQ_STEP,
    SLIDER_CONST_NOTE_STEP,
    DIRECT_VEL_BASED,
    INVERSE_VEL_BASED,
    PINGPONG
};

//generalization of the JUCE implementation to any note number
//not just integer note numbers
//returns frequency corresp. to the note number
forcedinline double getMidiNote( double noteNumber ){
    return 440*std::pow(2.0f, (noteNumber - 69)/12);
}

struct SpeedValues {

    //int version of enum SpeedChoice representing one of the choices
    int speed_choice;

    //change_time is the time it should take for one note to slide to the next
    //freq_speed is the per sample speed of change in frequency
    //note_speed is the per sample speed of change in note number (diff from frequency)
    //vel_based_speed is to vary the base speed for vel based speed change
    double change_time, freq_speed_per_second, note_speed_per_second, vel_based_speed;

    //the two harmonic oscillator differential equation variables
    //used in creating a pingpong effect
    double pp_omega, pp_zeta;

    //these represent the three coefficients used to calculate the
    //frequency for the next sample acc. to the diff equation
    double pp_K1, pp_K2, pp_K3;
};

struct SineWaveVoice : public juce::SynthesiserVoice {

    SineWaveVoice(struct SpeedValues* speeds) : speeds(speeds) {}

    ~SineWaveVoice() {
		speeds = NULL;
	}

    bool canPlaySound(juce::SynthesiserSound* sound) override
    {
        return dynamic_cast<SineWaveSound*> (sound) != nullptr;
    }

    void startNote(int midiNoteNumber, float velocity,
        juce::SynthesiserSound*, int /*currentPitchWheelPosition*/) override
    {
        currSamplingRate = getSampleRate();
        level = velocity * 0.15;

        tailOff = 0.0f;
        playNote = true;

        //The voice will target the currently pressed note
        this->setTarget(midiNoteNumber);

        currFreq = juce::MidiMessage::getMidiNoteInHertz(lastNote);
        targetFreq = juce::MidiMessage::getMidiNoteInHertz(currentNote);

        playingNote = -1;

        changeFreqTo(currFreq);

        //change speed based on slider value - time based or velocity of second note
        //constant frequency step, can be set with slider
        //--DONE
        switch ((SpeedChoice)(speeds->speed_choice)) {
        case SpeedChoice::SLIDER_TIME:
            freqStep = (targetFreq - currFreq) / (currSamplingRate * speeds->change_time);
            break;

        case SpeedChoice::SLIDER_CONST_FREQ_STEP:
            freqStep = speeds->freq_speed_per_second / currSamplingRate;
            break;

        case SpeedChoice::SLIDER_CONST_NOTE_STEP:
            playingNote = lastNote;
            noteStep = speeds->note_speed_per_second / currSamplingRate;
            break;

        case SpeedChoice::DIRECT_VEL_BASED:
            freqStep = speeds->vel_based_speed * velocity * 128 / currSamplingRate;
            break;
		//TODO: reduce volume as velocity reduces
        case SpeedChoice::INVERSE_VEL_BASED:
            freqStep = speeds->vel_based_speed / ((velocity > 0.1 ? velocity : 0.1) * currSamplingRate);
            break;

        case SpeedChoice::PINGPONG:
            //it is assumed that the synth and editor using this voice keeps track of the
            //variables for the diff equation. Their calculation is not done every time
            //a note is pressed, but every time the knobs/sliders are moved.
            pp_f1 = currFreq;
            break;
        case SpeedChoice::CONST_SAMPLES:
        default:
            freqStep = (targetFreq - currFreq) / SPEED;
        };
    }

    void stopNote(float /*velocity*/, bool allowTailOff) override
    {
        if (allowTailOff)
        {
            if (tailOff == 0.0)
                tailOff = 1.0;
        }
        else
        {
            //should remember last note pressed only when two notes are pressed at the same time
            //timer based approach - NOT NECESSARY
            setMemory();
            currFreq = currAng = angStep = 0.0;
            playNote = false;
            clearCurrentNote();
        }
    }

    void pitchWheelMoved(int) override {}
    void controllerMoved(int, int) override {}

    void renderNextBlock(juce::AudioSampleBuffer& outputBuffer, int startSample, int numSamples) override {
        if (playNote) {
            if (tailOff > 0.0) {
                for (int i = startSample; i < startSample + numSamples; i++) {
                    float currentSample = (float)(std::sin(currAng) * level * tailOff);

                    for (auto j = outputBuffer.getNumChannels() - 1; j >= 0; j--) {
                        outputBuffer.addSample(j, i, currentSample);
                    }

                    updateOscillator();

                    tailOff *= 0.99;

                    if (tailOff <= 0.05) {
                        tailOff = 0.0;
                        playNote = false;
                        setMemory();
                        currFreq = currAng = angStep = 0.0;
                        clearCurrentNote();
                    }
                }
            }
            else {
                for (int i = startSample; i < startSample + numSamples; i++) {
                    float currentSample = (float)(std::sin(currAng) * level);

                    for (auto j = outputBuffer.getNumChannels() - 1; j >= 0; j--) {
                        outputBuffer.addSample(j, i, currentSample);
                    }

                    updateOscillator();
                }
            }
        }
    }

    //frequency change speed values shared among all the voices
    struct SpeedValues* speeds;
    int lastNote = -1;
protected:

    //set the target frequency to the note currently pressed
    virtual void setTarget(int midiNoteNumber) {
        if (lastNote == -1) {
            lastNote = currentNote = midiNoteNumber;
        }
        else {
            //lastNote += 1;
            currentNote = midiNoteNumber;
        }
    }

    //set memory only for this voice (the first voice)
    virtual void setMemory() {
        lastNote = currentNote;
        currentNote = -1;
    }

    //level is multiplicative volume adjustment
    double level = 0.0f;
    //tailOff controls volume tail off after note ends playing,
    //applies exponential envelope
    double tailOff = 0.0f;
    //currFreq is the current freq of the sound
    double currFreq = 0.0;
    //targetFreq is the freq to which the note is slid on to
    double targetFreq = 0.0;
    //freqStep is the change in frequency per sample
    double freqStep = 0.0f;
    //noteStep is the change in note number per sample
    double noteStep = 0.0f;
    //currAng is the angle of the sine oscillator
    double currAng = 0.0;
    //angStep is the angle step taken by the oscillator per sample
    double angStep = 0.0;
    //currSamplingRate is the current sampling rate
    double currSamplingRate = 0.0;

    //lastNote is the last pressed midi note number (remembered by the synth)
    //int lastNote = -1;
    //currentNote is the currently pressed midi note number
    int currentNote = -1;
    //which note is playing right now.
    //ONLY USED WHEN NOTE BASED SLIDING IS ACTIVE
    double playingNote = -1;

    //whether to play a note or not
    bool playNote = false;

    //frequencies are previous two samples,
    //needed to calculate the frequency at the current sample
    double pp_f1 = 0.0;

    //change the frequency of the oscillator to the given freq
    void changeFreqTo(double newFreq) {
        angStep = juce::MathConstants<float>::twoPi * newFreq / currSamplingRate;
    }

    //update the state of the oscillator - called every sample
    void updateOscillator(){
        double newFreq;
        switch( (SpeedChoice)speeds->speed_choice ){
            case SpeedChoice::PINGPONG:
                //first find the frequency intended for the current sample
                newFreq = speeds->pp_K1 * targetFreq + speeds->pp_K2 * currFreq + speeds->pp_K3 * pp_f1;
                //then update these values for the next sample
                pp_f1 = currFreq;
                currFreq = newFreq;
                break;
            
            case SpeedChoice::SLIDER_CONST_NOTE_STEP:
                playingNote += noteStep;

                if (noteStep > 0.0) {
                    if (playingNote >= currentNote)
                        noteStep = 0;
                }
                else if (noteStep < 0.0) {
                    if (playingNote <= currentNote)
                        noteStep = 0.0;
                }
                currFreq = getMidiNote(playingNote);
                break;

            case SpeedChoice::SLIDER_CONST_FREQ_STEP:
            case SpeedChoice::DIRECT_VEL_BASED:
            case SpeedChoice::INVERSE_VEL_BASED:
            case SpeedChoice::SLIDER_TIME:
            case SpeedChoice::CONST_SAMPLES:
            default:
                currFreq += freqStep;

                if (freqStep > 0.0) {
                    if (currFreq >= targetFreq)
                        freqStep = 0.0;
                }
                else if (freqStep < 0.0) {
                    if (currFreq <= targetFreq)
                        freqStep = 0.0;
                }
        }

        changeFreqTo(currFreq);
        currAng += angStep;
        if (currAng >= juce::MathConstants<float>::twoPi)
            currAng -= juce::MathConstants<float>::twoPi;
    }
};

struct OtherSineWaveVoice : public SineWaveVoice {
public:

    OtherSineWaveVoice(struct SineWaveVoice* prev, struct SpeedValues* speeds)
        : SineWaveVoice(speeds), prevVoice(prev) {}

    ~OtherSineWaveVoice() {
        prevVoice = NULL;
    }

private:

    //this voice's start note is the target note of the previous voice in the list
    //its target note is the currently pressed note
    void setTarget(int midiNoteNumber) override {
        lastNote = prevVoice->getCurrentlyPlayingNote();
        currentNote = midiNoteNumber;
    }

    //no memory for these voices
    void setMemory() override {
        lastNote = currentNote = -1;
    }

    SineWaveVoice* prevVoice;
};