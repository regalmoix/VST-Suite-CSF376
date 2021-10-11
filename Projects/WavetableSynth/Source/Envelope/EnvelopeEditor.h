#pragma once

#include <JuceHeader.h>
#include "SegmentGenerator.h"

#define LOGGING

#ifdef LOGGING
    #define LOG(txt) DBG("[" << __FUNCTION__ << "] " << txt)
#else
    #define LOG(txt) 
#endif

using std::vector;
using std::tuple;

enum DragType
{
    none,
    draggingLeftmostControlPoint,
    draggingRightmostControlPoint,
    draggingInteriorControlPoint,
    draggingSegmentBody
};

class EnvelopeEditor : public Component
{
public:
    EnvelopeEditor ();

    /**
    * @brief Construct a new Envelope Editor Editor object
    * @param segmentDescriptors : vector of [initNormY, finalNormY, curvature, normLength]
    */
    EnvelopeEditor (const vector<SegmentDescriptor>& segmentDescriptors);
    ~EnvelopeEditor();

    void paint  (Graphics&) override;
    void resized()          override;

    void mouseDown(const MouseEvent&) override;
    void mouseDrag(const MouseEvent&) override;

    float getEnvelopeMultiplier (const float normX) const;

    float getEnvelopeMultiplierForSegmentIndex (size_t segmentIndex, const float normX) const;

    void  setDotRadius(int radius);

    void  setLineThickness(int thickness);

    void  setFixedControlPoints(bool controlPointsAreFixed);

    size_t getNumberOfControlPoints() const;

    void  setNewEnvelope(const vector<SegmentDescriptor>& newEnvelopeDescriptor);

    void  setBackgroundColour   (const Colour newColor);

    void  setLineColour         (const Colour newColor);

    void  setDotColour          (const Colour newColor);

protected:
    /** @brief object responsible for backend modification of internal state */
    Envelope env;
    
    /** @brief  vector of segment descriptors to describe the complete envelope */
    vector<SegmentDescriptor> envelopeDescriptor;

    /** @brief Stores the Radius of the control point dot */
    int         dotRadius       { 8 };

    /** @brief Stores the thickness of the envelope graph curve */
    int         lineThickness   { 2 };    

    /** @brief Stores the total length in samples of the entire envelope */
    int         totalLength;

    /** @brief Stores the Index of Clicked Segment/Control Point, checked by mouse drag */
    int         selectedSegmentIndex;

    /** @brief Stores the Curvature of Clicked Segment, checked by mouse drag to modify  */
    float       selectedSegmentCurvature;

    /** @brief To allow dynamic addition/deletion of control points or not */
    bool        allowAddingOrDeletingControlPoints  { true };

    /** @todo  Implement this, disable dragging of last segment if ghost etc*/
    /** @brief To enable/disable display of the last segment, control point */
    bool        isLastSegmentGhost                  { false };

    /** @brief Set to none if not being dragged, else the type of drag happening (control point / segment) */
    DragType    actionType; 

    /** @todo Add function to set colors from LnF */
    Colour      backgroundColour    { Colours::black };
    Colour      lineColour          { Colours::white };
    Colour      dotColour           { Colours::white };

    /** @todo  Add function to set LnF */
    /** @brief Menu to display the curve type selection */
    PopupMenu   curveMenu;

    /** @brief To paint the Envelope Graph. */
    void    paintGraph          (Graphics&);

    /**
     * @brief   Get the Segment Index corresponding to the x coordinate
     * @param x The x coordinate
     * @return  Segment Index 
     */
    int     getSegmentIndexFor  (int x);

    /**
     * @brief Get the Control Point Index
     * @param x coordinate
     * @param y coordinate
     * @return Control Point Index if x,y is within a control point, and -1 otherwise 
     */
    int     getControlPointIndex(int x, int y);

    /** @brief Allows horizontal dragging for the selected  controlPoint */
    virtual bool    allowHorizontalDrag ();

    /** @brief Allows vertical dragging for the selected  controlPoint */
    virtual bool    allowVerticalDrag   ();

    /** @brief Draws the control point circle at segment ends */
    void    drawControlPoint(Graphics& g, Rectangle<float> dotRect);   

private:
    JUCE_DECLARE_NON_COPYABLE_WITH_LEAK_DETECTOR (EnvelopeEditor)
};
