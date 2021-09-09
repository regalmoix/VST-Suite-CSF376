#include "MainComponent.h"

//==============================================================================
MainComponent::MainComponent()
{
    // Make sure you set the size of the component after
    // you add any child components.
    setSize (500, 400);

    slider.setSliderStyle(juce::Slider::SliderStyle::LinearHorizontal);
    slider.setRange(20.0f, 500.0f, 0.1f);
    slider.setValue(250.0f, juce::dontSendNotification);
    slider.onValueChange = [this]
    {
        /*targetLevel = (float)levelSlider.getValue();
        samplesToTarget = rampLengthSamples;*/
        currentFrequency = (double)slider.getValue();
    };
    addAndMakeVisible(slider);

    volSlider.setSliderStyle(juce::Slider::SliderStyle::LinearHorizontal);
    volSlider.setRange(0.0f, 100.0f, 0.1f);
    volSlider.setValue(50.0f, juce::dontSendNotification);
    volSlider.onValueChange = [this] {
        currentVolume = (double)(volSlider.getValue() / 100.0f);
    };
    addAndMakeVisible(volSlider);

    // Some platforms require permissions to open input channels so request that here
    if (juce::RuntimePermissions::isRequired (juce::RuntimePermissions::recordAudio)
        && ! juce::RuntimePermissions::isGranted (juce::RuntimePermissions::recordAudio))
    {
        juce::RuntimePermissions::request (juce::RuntimePermissions::recordAudio,
                                           [&] (bool granted) { setAudioChannels (granted ? 2 : 0, 2); });
    }
    else
    {
        // Specify the number of input and output channels that we want to open
        setAudioChannels (0, 2);
    }
}

MainComponent::~MainComponent()
{
    // This shuts down the audio device and clears the audio source.
    shutdownAudio();
}

//==============================================================================
void MainComponent::prepareToPlay (int samplesPerBlockExpected, double sampleRate)
{
    // This function will be called when the audio device is started, or when
    // its settings (i.e. sample rate, block size, etc) are changed.

    // You can use this function to initialise any resources you might need,
    // but be careful - it will be called on the audio thread, not the GUI thread.

    // For more details, see the help for AudioProcessor::prepareToPlay()
    currentFrequency = 250.0f;
    phase = 0.0f;
    currentSampleRate = sampleRate;
    currentVolume = 0.5f;
}

void MainComponent::getNextAudioBlock (const juce::AudioSourceChannelInfo& bufferToFill)
{
    // Your audio-processing code goes here!

    // For more details, see the help for AudioProcessor::getNextAudioBlock()

    // Right now we are not producing any data, in which case we need to clear the buffer
    // (to prevent the output of random noise)
    //bufferToFill.clearActiveBufferRegion();

    auto* leftBuffer = bufferToFill.buffer->getWritePointer(0, bufferToFill.startSample);
    auto* rightBuffer = bufferToFill.buffer->getWritePointer(1, bufferToFill.startSample);

    for (auto sample = 0; sample < bufferToFill.numSamples; sample++) {
        leftBuffer[sample] = std::sin(phase) * currentVolume;
        rightBuffer[sample] = std::sin(phase) * currentVolume;
        
        phase += (juce::MathConstants<double>::twoPi*currentFrequency)/currentSampleRate;
        
        if (phase >= juce::MathConstants<double>::twoPi) {
            phase -= juce::MathConstants<double>::twoPi;
        }
    }
}

void MainComponent::releaseResources()
{
    // This will be called when the audio device stops, or when it is being
    // restarted due to a setting change.

    // For more details, see the help for AudioProcessor::releaseResources()
}

//==============================================================================
void MainComponent::paint (juce::Graphics& g)
{
    // (Our component is opaque, so we must completely fill the background with a solid colour)
    g.fillAll (getLookAndFeel().findColour (juce::ResizableWindow::backgroundColourId));

    // You can add your drawing code here!
}

void MainComponent::resized()
{
    // This is called when the MainContentComponent is resized.
    // If you add any child components, this is where you should
    // update their positions.
    slider.setBounds((getWidth()>>1)-200, (getHeight()>>1)-200, 400, 200);
    volSlider.setBounds((getWidth() >> 1) - 200, (getHeight() >> 1), 400, 200);
}
