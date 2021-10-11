#include "EnvelopeEditor.h"
#include "PopupMenuSearch.h"

EnvelopeEditor::EnvelopeEditor()
{
    backgroundColour    = getLookAndFeel().findColour(ResizableWindow::backgroundColourId);
    envelopeDescriptor  = {{ 0.0f, 1.0f, 0.0f, 1.0 }};
}

EnvelopeEditor::EnvelopeEditor (const vector<SegmentDescriptor>& segmentDescriptors)
{
    backgroundColour    = getLookAndFeel().findColour(ResizableWindow::backgroundColourId);
    envelopeDescriptor  = segmentDescriptors;
}   

EnvelopeEditor::~EnvelopeEditor ()
{
}

void  EnvelopeEditor::setDotRadius (int radius)
{
    dotRadius       = radius;
    repaint();
}

void  EnvelopeEditor::setLineThickness (int thickness)
{
    lineThickness   = thickness;
    repaint();
}

size_t EnvelopeEditor::getNumberOfControlPoints() const
{
    return 1 + envelopeDescriptor.size();
}

void  EnvelopeEditor::setNewEnvelope(const vector<SegmentDescriptor>& newEnvelopeDescriptor)
{
    envelopeDescriptor = newEnvelopeDescriptor;
    repaint();
}

void  EnvelopeEditor::setFixedControlPoints (bool controlPointsAreFixed)
{
    allowAddingOrDeletingControlPoints  = !controlPointsAreFixed;
    repaint();
}

void  EnvelopeEditor::setBackgroundColour (const Colour newColor)
{
    backgroundColour    = newColor;
    repaint();
}

void  EnvelopeEditor::setLineColour (const Colour newColor)
{
    lineColour          = newColor;
}

void  EnvelopeEditor::setDotColour (const Colour newColor)
{
    dotColour           = newColor;
}

void  EnvelopeEditor::paintGraph (Graphics& g)
{
    auto bounds     = getLocalBounds().reduced(2 * dotRadius);
    const float x0  = float(2 * dotRadius);
    const float y0  = float(2 * dotRadius);
    int height      = bounds.getHeight();
    int width       = bounds.getWidth();

    g.setColour(lineColour);
    Path p;
    double  xValue;
    double  yValue;

    // getNextSample returns true when no further samples remain => end is reached
    bool    endOfEnvelope = env.getNextSample(yValue);

    // Initial Point
    p.startNewSubPath (x0 + 0.0f, y0 + (1.0f - yValue) * height);

    for (int ix = 1; !endOfEnvelope && ix < width; ix++)
    {
        /** @bug Below line is most accurate however computationally expensive */
        // yValue          = env.getValue(float(ix)/float(totalLength));
        endOfEnvelope   = env.getNextSample(yValue);
        xValue          = float(ix);

        /** @bug  Unexpected functioning when last control point isnt at 1.0 */
        /** @todo this is a potential feature for the future */
        if (isLastSegmentGhost && (xValue > totalLength * envelopeDescriptor.back().initialX))
            break;
        else
            p.lineTo(x0 + xValue, y0 + (1.0f - yValue) * height);
    }
    g.strokePath(p, PathStrokeType(lineThickness));

    // put a circle at every segment start point
    Rectangle<float> dotRect (2 * dotRadius, 2 * dotRadius);

    for (auto& segment : envelopeDescriptor)
    {
        xValue = x0 + (0.0f + segment.initialX) * width;
        yValue = y0 + (1.0f - segment.initialValue) * height;

        drawControlPoint(g, dotRect.withCentre(Point<float>(xValue, yValue)));

    }

    xValue = x0 + (0.0f + envelopeDescriptor.back().finalX)     * width;
    yValue = y0 + (1.0f - envelopeDescriptor.back().finalValue) * height;

    if (!isLastSegmentGhost)
        drawControlPoint (g, dotRect.withCentre(Point<float>(xValue, yValue)));
}

void  EnvelopeEditor::drawControlPoint (Graphics& g, Rectangle<float> dotRect)
{
    g.setColour     (dotColour);
    g.drawEllipse   (dotRect, 2.0f);
    g.fillEllipse   (dotRect);
}

void  EnvelopeEditor::paint (Graphics& g)
{
    g.fillAll(backgroundColour);
    env.reset(envelopeDescriptor);
    paintGraph(g);
    for (auto& abc : envelopeDescriptor)
    {
        LOG("x= " << abc.initialX);
    }
    LOG("--------");
}

void  EnvelopeEditor::resized ()
{
    if (envelopeDescriptor.size() == 0) 
        return;

    auto bounds = getLocalBounds().reduced(2 * dotRadius);

    totalLength = bounds.getWidth();

    env.changeTotalLength(totalLength);
}

int   EnvelopeEditor::getSegmentIndexFor (int sampleIndex)
{
    int segIdx = 0;
    for (auto& segment : envelopeDescriptor)
    {   
        if (sampleIndex >= segment.initialX * totalLength && sampleIndex <= segment.finalX * totalLength) 
        {
            return segIdx;
        } 

        segIdx++;
    }

    return -1;
}

int   EnvelopeEditor::getControlPointIndex (int x, int y)
{
    int allowance   = dotRadius;
    auto bounds     = getLocalBounds().reduced(2 * dotRadius);
    x -= 2 * dotRadius;
    y -= 2 * dotRadius;

    for (int i = 0; i < envelopeDescriptor.size(); i++)
    {
        int segmentStart    = envelopeDescriptor[i].initialX * totalLength;
        int segmentEnd      = envelopeDescriptor[i].finalX   * totalLength;

        if (abs(x - segmentStart) < allowance)
        {
            if (abs(y - (1.0f - envelopeDescriptor[i].initialValue) * bounds.getHeight()) < allowance)
            {
                return i;
            }
        }

        if (abs(x - segmentEnd) < allowance)
        {
            if (abs(y - (1.0f - envelopeDescriptor[i].finalValue) * bounds.getHeight()) < allowance)
            {
                return i + 1;
            }
        }
    }

    // No control point for the given x, y
    return -1;
}

void  EnvelopeEditor::mouseDown (const MouseEvent& evt)
{
    auto bounds = getLocalBounds() .reduced(2 * dotRadius);
    int  xValue = evt.getPosition().getX();
    int  yValue = evt.getPosition().getY();

    float fractionalY   = 1.0f - (yValue - 2 * dotRadius) / float(bounds.getHeight());
    float fractionalX   =        (xValue - 2 * dotRadius) / float(bounds.getWidth ());

    actionType          = none;

    // Note that segment index = control point index
    selectedSegmentIndex = getControlPointIndex(xValue, yValue);

    if (selectedSegmentIndex == 0)
        actionType = draggingLeftmostControlPoint;
    
    else if (selectedSegmentIndex == envelopeDescriptor.size())
        actionType = draggingRightmostControlPoint;

    else if (selectedSegmentIndex > 0)
    {
        // Interior control point selected => Double Click Deletion
        if (evt.getNumberOfClicks() > 1 && allowAddingOrDeletingControlPoints)
        {
            // Store the segment to be deleted 
            auto segment = envelopeDescriptor[selectedSegmentIndex];

            // Delete the segment
            auto it = envelopeDescriptor.begin() + selectedSegmentIndex;
            envelopeDescriptor.erase(it);
            selectedSegmentIndex--;

            // Add deleted segment length to previous segment, and modify previous segment final value
            envelopeDescriptor[selectedSegmentIndex].finalValue     = segment.finalValue;
            envelopeDescriptor[selectedSegmentIndex].finalX         = segment.finalX;

            repaint();
        }
        else
        {
            LOG ("draggingInteriorControlPoint");
            actionType = draggingInteriorControlPoint;
        }
    }

    // Segment clicked, not control point => getControlPointIndex returned -1.
    else
    {
        selectedSegmentIndex = getSegmentIndexFor(xValue - 2 * dotRadius);

        if (evt.mods.isPopupMenu())
        {
            LOG("Right Click!");

            curveMenu.addItem(CurveType::Linear      + 1, "Linear",     true, envelopeDescriptor[selectedSegmentIndex].curveType == CurveType::Linear) ;
            curveMenu.addItem(CurveType::Exponential + 1, "Exponential",true, envelopeDescriptor[selectedSegmentIndex].curveType == CurveType::Exponential) ;
            curveMenu.addItem(CurveType::Power       + 1, "Power",      true, envelopeDescriptor[selectedSegmentIndex].curveType == CurveType::Power) ;
            curveMenu.addItem(CurveType::QSine       + 1, "Sine",       true, envelopeDescriptor[selectedSegmentIndex].curveType == CurveType::QSine) ;
            curveMenu.addItem(CurveType::QCos        + 1, "Cosine",     true, envelopeDescriptor[selectedSegmentIndex].curveType == CurveType::QCos) ;

            auto setCurveType = [&, this](int selection)
                                {
                                    if (selection == 0)
                                        return;
                                    this->envelopeDescriptor[this->selectedSegmentIndex].curveType = static_cast<CurveType>(selection - 1);
                                    this->repaint();
                                };
            showPopupMenuWithQuickSearch(curveMenu, PopupMenuQuickSearchOptions(), this, setCurveType);
            // curveMenu.showMenuAsync(PopupMenu::Options(), setCurveType); 
            curveMenu.clear();   
        }

        // Double Click on Segment => Add new control point
        if (evt.getNumberOfClicks() > 1 && allowAddingOrDeletingControlPoints)
        {
            auto it = envelopeDescriptor.begin() + selectedSegmentIndex;

            auto    seg = *it;

            // Find the segment bounds of the current segment to split
            int     segStart        = envelopeDescriptor[selectedSegmentIndex].initialX ;
            int     segEnd          = envelopeDescriptor[selectedSegmentIndex].finalX;

            // Insert segment at the current position
            float lengthDelta = (float)(xValue - 2 * dotRadius - segStart) / (float)totalLength;
            envelopeDescriptor.insert(it, { seg.initialValue, fractionalY, seg.curvature, seg.initialX, fractionalX,seg.curveType});

            // Modify the segment to reduce length and set initial value to be same as the final value of inserted segment
            selectedSegmentIndex++;
            envelopeDescriptor[selectedSegmentIndex].initialValue   = fractionalY;
            envelopeDescriptor[selectedSegmentIndex].initialX       = fractionalX;

            repaint();
        }

        // Drag => adjust segment curvature
        else
        {
            if (selectedSegmentIndex >= 0)
            {
                actionType      = draggingSegmentBody;
                selectedSegmentCurvature  = envelopeDescriptor[selectedSegmentIndex].curvature;
            }
        }
    }
}

bool  EnvelopeEditor::allowHorizontalDrag ()
{
    return true;
}

bool  EnvelopeEditor::allowVerticalDrag ()
{
    return true;
}

void  EnvelopeEditor::mouseDrag (const MouseEvent& evt)
{
    auto    bounds      = getLocalBounds ().reduced(2 * dotRadius);
    int     mouseX      = evt.getPosition().getX();

    float   fractionalX = 0.0f + (evt.getPosition().getX() - 2 * dotRadius) / float(bounds.getWidth ());
    float   fractionalY = 1.0f - (evt.getPosition().getY() - 2 * dotRadius) / float(bounds.getHeight());
    
    if (fractionalY < 0.0f) 
        fractionalY = 0.0f;

    else if (fractionalY > 1.0f) 
        fractionalY = 1.0f;

    float   dy;
    float   curvature;

    // Find the segment bounds of the segment
    int     segStart    = envelopeDescriptor[selectedSegmentIndex].initialX * totalLength;
    int     segEnd      = envelopeDescriptor[selectedSegmentIndex].finalX   * totalLength;

    int prevSegStart    = (selectedSegmentIndex > 0) 
                            ? envelopeDescriptor[selectedSegmentIndex - 1].initialX * totalLength
                            : 0;

    switch (actionType)
    {
        case draggingLeftmostControlPoint:
        {
            if (allowVerticalDrag())
                envelopeDescriptor[selectedSegmentIndex].initialValue = fractionalY;
            break;
        }

        case draggingRightmostControlPoint:
        {
            if (allowVerticalDrag())
                envelopeDescriptor[selectedSegmentIndex - 1].finalValue = fractionalY;
            break;
        }

        case draggingInteriorControlPoint:
        {
            // Current Control Point Movement is restricted between the X values of the previous and next control points
            if (allowHorizontalDrag() && (mouseX - 2 * dotRadius) >= prevSegStart && (mouseX - 2 * dotRadius) <= segEnd)
            {
                envelopeDescriptor[selectedSegmentIndex]    .initialX   = fractionalX;
                envelopeDescriptor[selectedSegmentIndex - 1].finalX     = fractionalX;
            }

            if (allowVerticalDrag())
            {
                envelopeDescriptor[selectedSegmentIndex]    .initialValue   = fractionalY;
                envelopeDescriptor[selectedSegmentIndex - 1].finalValue     = fractionalY;
            }
            break;
        }

        case draggingSegmentBody:
        {
            // Modify curvature proportional to cube of drag distance
            dy = 0.01f * evt.getDistanceFromDragStartY();

            if (envelopeDescriptor[selectedSegmentIndex].finalValue < envelopeDescriptor[selectedSegmentIndex].initialValue)
                dy = -dy;

            curvature = selectedSegmentCurvature - (dy * dy * dy);

            if (curvature > MAX_CURVATURE) 
                curvature = MAX_CURVATURE;

            if (curvature < MIN_CURVATURE) 
                curvature = MIN_CURVATURE;

            envelopeDescriptor[selectedSegmentIndex].curvature = curvature;
            break;
        }

        case none:
            return;

        default:
            return;
    }

    repaint();
}

float EnvelopeEditor::getEnvelopeMultiplier (float normX) const
{
    return env.getValue(normX);
}

float EnvelopeEditor::getEnvelopeMultiplierForSegmentIndex (size_t segmentIndex, float normX) const
{
    return env.getValueForSegment(segmentIndex, normX);
}
