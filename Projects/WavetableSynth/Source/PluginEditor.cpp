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


        attackAttachment    (p.apvts, "Attack",  attackKnob),
        decayAttachment     (p.apvts, "Decay",   decayKnob),
        sustainAttachment   (p.apvts, "Sustain", sustainKnob),
        releaseAttachment   (p.apvts, "Release", releaseKnob)
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
void WavetableSynthAudioProcessorEditor::paint (juce::Graphics& g)
{
    // (Our component is opaque, so we must completely fill the background with a solid colour)
    g.fillAll (getLookAndFeel().findColour (juce::ResizableWindow::backgroundColourId));
}

void WavetableSynthAudioProcessorEditor::resized()
{
    // This is generally where you'll want to lay out the positions of any
    // subcomponents in your editor..
    auto bounds         = getLocalBounds();

    auto attackBounds   = bounds.removeFromLeft(bounds.getWidth() * 0.25);
    auto decayBounds    = bounds.removeFromLeft(bounds.getWidth() * 0.33);
    auto sustainBounds  = bounds.removeFromLeft(bounds.getWidth() * 0.50);
    auto releaseBounds  = bounds.removeFromLeft(bounds.getWidth() * 1.00);

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
        &releaseKnob
    };
}