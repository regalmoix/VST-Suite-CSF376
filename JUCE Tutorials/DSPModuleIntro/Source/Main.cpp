/*
  ==============================================================================

    This file contains the basic startup code for a JUCE application.

  ==============================================================================
*/

#include <JuceHeader.h>
#include "MainComponent.h"

//==============================================================================
juce::AudioProcessor* JUCE_CALLTYPE createPluginFilter() {
    return new MainComponent();
}

