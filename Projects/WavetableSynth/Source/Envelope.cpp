/*
    ==============================================================================

    Envelope.cpp
    Created: 11 Oct 2021 5:08:09pm
    Author:  regalmoix

    ==============================================================================
*/

#include "PluginEditor.h"

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