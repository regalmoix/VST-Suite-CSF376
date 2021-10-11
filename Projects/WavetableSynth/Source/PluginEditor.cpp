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
        wavetableAttachment (p.apvts, "WaveTable Choice", wavetableChoice),
        waveGraph           (p)
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
    wavetableChoice.setSelectedItemIndex(p.apvts.getRawParameterValue("WaveTable Choice")->load());
    
    attackKnob .setTextValueSuffix(" ms");
    decayKnob  .setTextValueSuffix(" ms");
    releaseKnob.setTextValueSuffix(" ms");

    /** @todo : Set Label Text Colour in LNF to reduce duplication */
    wavetableLabel.attachToComponent(&wavetableChoice, true);
    wavetableLabel.setText("WaveTable", dontSendNotification);
    wavetableLabel.setColour(Label::textColourId, Colours::cyan);
    wavetableLabel.setJustificationType(Justification::centred);


    /**
     * @brief Initialise the Draggable ADSR Envelope
     */

    adsrEnvelopeDraggable.setNewEnvelope({
                                { 0.0f, 1.0f, 0.0f, 0.00, 0.15 }, 
                                { 1.0f, 0.5f, 0.0f, 0.15, 0.35 }, 
                                { 0.5f, 0.0f, 0.0f, 0.35, 0.60 },
                                { 0.0f, 0.0f, 0.0f, 0.60, 1.00 }    // GHOST SEGMENT fixed at Bottom Edge
                            });
    adsrEnvelopeDraggable.setFixedControlPoints(true);

    adsrEnvelopeDraggable.setLineColour(Colours::orange);
    adsrEnvelopeDraggable.setDotColour (Colours::red);
    adsrEnvelopeDraggable.setBackgroundColour(Colour(0xff323538));

    adsrEnvelopeDraggable.setDotRadius(6);
    adsrEnvelopeDraggable.setLineThickness(2);


    for (Component* component : getComponents())
    {
        addAndMakeVisible(component);
    }

    setSize (600, 600);
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
                        FlexBox::JustifyContent::spaceBetween
                    );

    FlexBox adsrMenu(   
                        FlexBox::Direction::row, 
                        FlexBox::Wrap::wrap, 
                        FlexBox::AlignContent::center, 
                        FlexBox::AlignItems::center, 
                        FlexBox::JustifyContent::spaceBetween
                    );
    
    FlexBox adsrEnvArea (   
                        FlexBox::Direction::row, 
                        FlexBox::Wrap::wrap, 
                        FlexBox::AlignContent::center, 
                        FlexBox::AlignItems::center, 
                        FlexBox::JustifyContent::center
                    );

    auto bounds     = getLocalBounds().reduced(12);
    int diameter    = 80;

    for (auto& component : getComponents())
    {
        if (auto* knob = dynamic_cast<RotarySlider*>(component))
            adsrMenu.items.add(FlexItem(*knob)  .withMinWidth(diameter).withMinHeight(diameter));
    }

    topMenu.items.add(FlexItem(waveGraph)       .withMinWidth(bounds.getWidth() * 0.30).withMinHeight(bounds.getHeight() * 0.10));
    topMenu.items.add(FlexItem(wavetableChoice) .withMinWidth(bounds.getWidth() * 0.35).withMinHeight(bounds.getHeight() * 0.10));

    adsrEnvArea.items.add(FlexItem(adsrEnvelopeDraggable)
                            .withMinWidth(bounds.getWidth() * 0.95)
                            .withMinHeight(bounds.getHeight() * 0.45)
                        );

    editor.items.add(FlexItem(bounds.getWidth(), bounds.getHeight() * 0.1, topMenu ));
    editor.items.add(FlexItem(bounds.getWidth(), bounds.getHeight() * 0.4, adsrMenu));
    editor.items.add(FlexItem(bounds.getWidth(), bounds.getHeight() * 0.5, adsrEnvArea));
    
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
        &wavetableLabel,
        &waveGraph,
        &adsrEnvelopeDraggable
    };
}