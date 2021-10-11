#pragma once
#include <JuceHeader.h>
#include "FunctionTable.h"

using std::vector;

struct SegmentDescriptor
{
public:
    double      initialValue;
    double      finalValue;
    double      curvature;
    double      initialX;
    double      finalX;
    CurveType   curveType   { CurveType::Exponential };

    double      getNormLengthSamples() const;
};

class Segment
{
public:
    SegmentDescriptor   properties;

    /**
     * @brief Construct a new Segment from the descriptor, and set dx based on newLength
     * @param descriptor 
     * @param newLength 
     */
    Segment (SegmentDescriptor descriptor, size_t newLength);

    /**
     * @brief Get the Sample for the normalised Phase.
     * @param normPhase 
     * @return Sample value. 
     */
    double  getSample       (double normPhase) const;

    /**
     * @brief   Get the next sample and increment currentSampleIndex and normX += dx;
     * @param   sample Reference param to store the sample
     * @return  true if end of Segment reached, false otherwise
     */
    bool    getNextSample   (double& sample);

private:
    FunctionTable   table;

    bool            isLinear;

    size_t          totalLength;
    size_t          currentSampleIndex;

    double          normX;
    double          dx;
};

class Envelope
{
public:
    void    changeTotalLength (int totalLength);

    /**
     * @brief Re Constructs segments from segmentArray.
     * @param segmentArray : The vector of Segment Descriptors describing an Envelope
     */
    void    reset (const vector<SegmentDescriptor>& segmentArray);

    /**
     * @brief   Get the next sample and increment internal segment counter on crossing a segment boundary
     * @param   sample Reference param to store the sample
     * @return  true if end of envelope reached, false otherwise
     */
    bool    getNextSample  (double& sample);

    int     getCurrentSegmentIndex () const;

    double  getValue     (double normPhase) const;

    double  getValueForSegment   (int segmentIndex, double normPhase) const;

protected:
    vector<Segment> segments;

    int currentSegmentIndex     { 0 };
    int totalLength             { 0 };
};
