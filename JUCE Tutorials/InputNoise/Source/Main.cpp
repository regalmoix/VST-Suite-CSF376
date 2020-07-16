/*
  ==============================================================================

    This file contains the basic startup code for a JUCE application.

  ==============================================================================
*/

#include <JuceHeader.h>
#include "MainComponent.h"

//==============================================================================
class ProcessingInputApplication : public juce::JUCEApplication
{
public:
    //==============================================================================
    ProcessingInputApplication() {}

    const juce::String getApplicationName() override { return "ProcessingAudioInputTutorial"; }
    const juce::String getApplicationVersion() override { return "1.0.0"; }


    //==============================================================================
    void initialise(const juce::String&) override
    {
        // This method is where you should put your application's initialisation code..

        mainWindow.reset(new MainWindow("ProcessingAudioInputTutorial", new MainComponent(), *this));
    }

    void shutdown() override
    {
        // Add your application's shutdown code here..

        mainWindow = nullptr; // (deletes our window)
    }

    //==============================================================================


    //==============================================================================
    /*
        This class implements the desktop window that contains an instance of
        our MainComponent class.
    */
    class MainWindow : public juce::DocumentWindow
    {
    public:
        MainWindow(const juce::String& name, Component* c, JUCEApplication& a)
            : DocumentWindow(name, juce::Desktop::getInstance().getDefaultLookAndFeel()
                .findColour(ResizableWindow::backgroundColourId),
                DocumentWindow::allButtons),
            app(a)
        {
            setUsingNativeTitleBar(true);
            setContentOwned(c, true);

#if JUCE_ANDROID || JUCE_IOS
            setFullScreen(true);
#else
            setResizable(true, false);
            setResizeLimits(300, 250, 10000, 10000);
            centreWithSize(getWidth(), getHeight());
#endif

            setVisible(true);
        }

        void closeButtonPressed() override
        {
            app.systemRequestedQuit();
        }

    private:
        JUCEApplication& app;

        //==============================================================================
        JUCE_DECLARE_NON_COPYABLE_WITH_LEAK_DETECTOR(MainWindow)
    };

private:
    std::unique_ptr<MainWindow> mainWindow;
};

//==============================================================================
// This macro generates the main() routine that launches the app.
START_JUCE_APPLICATION(ProcessingInputApplication)
