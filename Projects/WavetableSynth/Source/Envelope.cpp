/*
    ==============================================================================

    Envelope.cpp
    Created: 11 Oct 2021 5:08:09pm
    Author:  regalmoix

    ==============================================================================
*/

#include "PluginEditor.h"
#include "PluginProcessor.h"

using std::vector;

ADSREnvelopeEditor::ADSREnvelopeEditor(
                                        WavetableSynthAudioProcessor& p, 
                                        RotarySlider& attackKnob, 
                                        RotarySlider& decayKnob, 
                                        RotarySlider& sustainKnob, 
                                        RotarySlider& releaseKnob
                                    )
    : processor     (p),
      attackKnob    (attackKnob), 
      decayKnob     (decayKnob), 
      sustainKnob   (sustainKnob), 
      releaseKnob   (releaseKnob)      
{
    // Register as a listener
    auto& parameters = processor.getParameters();
    for (auto& param : parameters)
    {
        param->addListener(this);
    }
    /** @todo initial call to display response curve when plugin started */
    // TODO

    startTimerHz(60);
}

ADSREnvelopeEditor::~ADSREnvelopeEditor()
{
    // De register as the listener
    auto& parameters = processor.getParameters();
    for (auto& param : parameters)
    {
        param->removeListener(this);
    }
    stopTimer();
}


void ADSREnvelopeEditor::parameterGestureChanged (int parameterIndex, bool gestureIsStarting) 
{ 
    /* 
        Do Nothing.
    */ 
}

void ADSREnvelopeEditor::parameterValueChanged(int parameterIndex, float newValue)
{
    /** @note : parameterIndex = 1..4 for A,D,S,R knobs */
    if (parameterIndex >= 1 && parameterIndex <= 4)
    {
        adsrChanged.set(true);   
    }
}

void ADSREnvelopeEditor::timerCallback()
{   
    if (adsrChanged.compareAndSetBool(false, true))
    {
        /** @todo : recompute the adsr envelope table and save it in some array in Processor. */
        /** @todo : When knobs change value we redraw segments on the graph and vice versa */
        /** @todo : Add Listener Broadcast feature, or, overload mouse drag, call baseclass mousedrag and manually update "listeners" */
        ADSRSettings settings       = getADSRSettings(processor.apvts, processor.getSampleRate());
        size_t totalLengthInSamples = processor.getSampleRate() * 30;       // attack, decay, release capped at 10 secs each, so sum = 30
        
        // Attack, Decay and Release fractions
        float af    = float(settings.attackDuration)  / float(totalLengthInSamples); 
        float df    = float(settings.decayDuration)   / float(totalLengthInSamples); 
        float rf    = float(settings.releaseDuration) / float(totalLengthInSamples); 

        float s     = settings.sustainGain;

        envelopeDescriptor[ADSRState::Attack].finalX    = af;

        envelopeDescriptor[ADSRState::Decay].initialX   = af;
        envelopeDescriptor[ADSRState::Decay].finalX     = af + df;
        envelopeDescriptor[ADSRState::Decay].finalValue = s;

        // Subtracted 1 since there is no "sustain" segment, hence indexing shifts by 1
        envelopeDescriptor[ADSRState::Release - 1].initialValue = s;
        envelopeDescriptor[ADSRState::Release - 1].initialX = af + df;
        envelopeDescriptor[ADSRState::Release - 1].finalX   = af + df + rf;

        envelopeDescriptor[ADSRState::Stopped - 1].initialX = af + df + rf;

        repaint();
    }
}

void ADSREnvelopeEditor::mouseDown(const MouseEvent& evt)
{
    EnvelopeEditor::mouseDown(evt);
}
void ADSREnvelopeEditor::mouseDrag(const MouseEvent& evt)
{
    EnvelopeEditor::mouseDrag(evt);

    /** @brief 
     * 3 segments to envelope => max norm length for any phase = 0.33
     * multiply by 3 to scale 0.33 -> 1
     * Note that 10000 is total length of the normalisable range, ie each duration is at max 10000 msec long
     */
    double attackValue  = (envelopeDescriptor[0].getNormLengthSamples() * 3) * 10000;
    double decayValue   = (envelopeDescriptor[1].getNormLengthSamples() * 3) * 10000;
    double sustainValue =  envelopeDescriptor[1].finalValue * 1.0;
    double releaseValue = (envelopeDescriptor[2].getNormLengthSamples() * 3) * 10000;

    attackKnob .setValue(attackValue );
    decayKnob  .setValue(decayValue  );
    sustainKnob.setValue(sustainValue);
    releaseKnob.setValue(releaseValue);
}

bool ADSREnvelopeEditor::allowHorizontalDrag()
{
    switch (selectedSegmentIndex)
    {
        // Start Point
        case 0:
            return false;
        // Attack Point
        case 1:
            return true; 
        // Decay, Sustain Point
        case 2:
            return true; 
        // Release Point
        case 3:
            return true;
        // Ghost Stop Point
        case 4 : 
            return false;

        default:
            return false;
    }
}

bool ADSREnvelopeEditor::allowVerticalDrag()
{
    switch (selectedSegmentIndex)
    {
        // Start Point
        case 0:
            return false;
        // Attack Point
        case 1:
            return false; 
        // Decay, Sustain Point
        case 2:
            return true; 
        // Release Point
        case 3:
            return false;
        // Ghost Stop Point
        case 4 : 
            return false;

        default:
            return false;
    }
}