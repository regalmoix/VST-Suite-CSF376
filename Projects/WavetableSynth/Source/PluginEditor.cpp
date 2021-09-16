/*
  ==============================================================================

    This file contains the basic framework code for a JUCE plugin editor.

  ==============================================================================
*/

#include "PluginProcessor.h"
#include "PluginEditor.h"

//==============================================================================
WavetableSynthAudioProcessorEditor::WavetableSynthAudioProcessorEditor (WavetableSynthAudioProcessor& p)
    :   AudioProcessorEditor (&p), 
        audioProcessor (p),
        attackKnob  (*p.apvts.getParameter("Attack"),  "ms"),
        decayKnob   (*p.apvts.getParameter("Decay"),   "ms"),
        sustainKnob (*p.apvts.getParameter("Sustain"), ""),
        releaseKnob (*p.apvts.getParameter("Release"), "ms"),
        wavetableChoice("WaveTable Choice"),

        attackAttachment    (p.apvts, "Attack",  attackKnob),
        decayAttachment     (p.apvts, "Decay",   decayKnob),
        sustainAttachment   (p.apvts, "Sustain", sustainKnob),
        releaseAttachment   (p.apvts, "Release", releaseKnob),
        wavetableAttachment (p.apvts, "WaveTable Choice", wavetableChoice)
{
    /** @TODO: If possible, send this code to Slider constructor, adding labels by getting max and min value */
    attackKnob.labels.add({ "0ms", 0.0f });
    attackKnob.labels.add({ "10s", 1.0f});

    decayKnob.labels.add({ "0ms", 0.0f });
    decayKnob.labels.add({ "10s", 1.0f});
    
    sustainKnob.labels.add({ "0", 0.0f });
    sustainKnob.labels.add({ "1", 1.0f});

    releaseKnob.labels.add({ "0ms", 0.0f });
    releaseKnob.labels.add({ "10s", 1.0f});

    /** @brief Add items at index 1 as 0 => no selection */
    /** @todo  Add wave logos and customise LnF of the Menu */
    StringArray wavetableChoices = {"Sine Wave", "Square Wave", "Triangle Wave", "Saw Wave", "Silence"};
    wavetableChoice.addItemList(wavetableChoices, 1);
    
    for (Component* component : getComponents())
    {
        addAndMakeVisible(component);
    }

    setSize (900, 300);
}

WavetableSynthAudioProcessorEditor::~WavetableSynthAudioProcessorEditor()
{
}

//==============================================================================
void WavetableSynthAudioProcessorEditor::paint (Graphics& g)
{
    // (Our component is opaque, so we must completely fill the background with a solid colour)
    g.fillAll (getLookAndFeel().findColour (ResizableWindow::backgroundColourId));
}

void WavetableSynthAudioProcessorEditor::resized()
{
    // This is generally whqere you'll want to lay out the positions of any
    // subcomponents in your editor..
    auto bounds         = getLocalBounds();
    auto wavetableBounds= bounds.removeFromTop (bounds.getHeight()* 0.15);
    auto attackBounds   = bounds.removeFromLeft(bounds.getWidth() * 0.25);
    auto decayBounds    = bounds.removeFromLeft(bounds.getWidth() * 0.33);
    auto sustainBounds  = bounds.removeFromLeft(bounds.getWidth() * 0.50);
    auto releaseBounds  = bounds.removeFromLeft(bounds.getWidth() * 1.00);

    wavetableChoice.setBounds(wavetableBounds);

    attackKnob .setBounds(attackBounds);
    decayKnob  .setBounds(decayBounds);
    sustainKnob.setBounds(sustainBounds);
    releaseKnob.setBounds(releaseBounds);
}

std::vector<Component*> WavetableSynthAudioProcessorEditor::getComponents()
{
    return {
        &attackKnob,
        &decayKnob,
        &sustainKnob,
        &releaseKnob,
        &wavetableChoice
    };
}