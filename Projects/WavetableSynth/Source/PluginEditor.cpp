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
        attackKnob  (*p.apvts.getParameter("Attack"),  "ms", "Attack"),
        decayKnob   (*p.apvts.getParameter("Decay"),   "ms", "Decay"),
        sustainKnob (*p.apvts.getParameter("Sustain"), "",   "Sustain"),
        releaseKnob (*p.apvts.getParameter("Release"), "ms", "Release"),
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
    wavetableChoice.setSelectedItemIndex(0);
    
    attackKnob .setTextValueSuffix(" ms");
    decayKnob  .setTextValueSuffix(" ms");
    releaseKnob.setTextValueSuffix(" ms");

    /** @todo : Set Label Text Colour in LNF to reduce duplication */
    wavetableLabel.attachToComponent(&wavetableChoice, true);
    wavetableLabel.setText("WaveTable", dontSendNotification);
    wavetableLabel.setColour(Label::textColourId, Colours::cyan);
    wavetableLabel.setJustificationType(Justification::centred);

    for (Component* component : getComponents())
    {
        addAndMakeVisible(component);
    }

    setSize (400, 150);
}

WavetableSynthAudioProcessorEditor::~WavetableSynthAudioProcessorEditor()
{
}

//==============================================================================
void WavetableSynthAudioProcessorEditor::paint (Graphics& g)
{
    // (Our component is opaque, so we must completely fill the background with a solid colour)
    g.fillAll (Colours::black);
}

void WavetableSynthAudioProcessorEditor::resized()
{
    FlexBox editor;
    editor.flexDirection = FlexBox::Direction::column;

    FlexBox topMenu (   
                        FlexBox::Direction::row, 
                        FlexBox::Wrap::wrap, 
                        FlexBox::AlignContent::center, 
                        FlexBox::AlignItems::center, 
                        FlexBox::JustifyContent::flexEnd
                    );

    FlexBox adsrMenu(   
                        FlexBox::Direction::row, 
                        FlexBox::Wrap::wrap, 
                        FlexBox::AlignContent::center, 
                        FlexBox::AlignItems::center, 
                        FlexBox::JustifyContent::spaceBetween
                    );

    auto bounds     = getLocalBounds().reduced(5);
    int diameter    = 80;

    for (auto& component : getComponents())
    {
        if (auto* knob = dynamic_cast<RotarySlider*>(component))
            adsrMenu.items.add(FlexItem(*knob).withMinWidth(diameter).withMinHeight(diameter));
    }
    topMenu .items.add(FlexItem(wavetableChoice).withMinWidth(bounds.getWidth() * 0.35).withMinHeight(bounds.getHeight() * 0.20));

    editor.items.add(FlexItem(bounds.getWidth(), bounds.getHeight() * 0.25, topMenu ));
    editor.items.add(FlexItem(bounds.getWidth(), bounds.getHeight() * 0.75, adsrMenu));
    
    editor.performLayout (bounds.toFloat());
}

std::vector<Component*> WavetableSynthAudioProcessorEditor::getComponents()
{
    return {
        &attackKnob,
        &decayKnob,
        &sustainKnob,
        &releaseKnob,
        &wavetableChoice,
        &wavetableLabel
    };
}