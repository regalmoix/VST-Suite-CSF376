#include "SegmentGenerator.h"

bool Envelope::getNextSample(double& sample)
{
    // Retrieved Sample is put into the reference param. If this was the last sample we enter the if block
    if (segments[currentSegmentIndex].getNextSample(sample))
    {
        ++currentSegmentIndex;

        if (currentSegmentIndex >= segments.size())
        {
            currentSegmentIndex = 0;
            return true;
        }
    }
    return false;
}

int Envelope::getCurrentSegmentIndex() const 
{ 
    return currentSegmentIndex;
}

void Envelope::reset(const vector<SegmentDescriptor>& segmentArray)
{
    segments.clear();

    for (auto segmentDescriptor : segmentArray)
        segments.emplace_back(segmentDescriptor, totalLength);

    currentSegmentIndex = 0;
}

void Envelope::changeTotalLength(int totalLength)
{
    this->totalLength = totalLength;
}

double Envelope::getValue(double normPhase) const
{
    // Stores the x coordinate of end of the current segment.
    float   normSegmentEnd  = 0;
    int     segmentIndex    = -1;

    for (int i = 0; i < segments.size(); i++)
    {
        const Segment& segment  = segments[i];
        normSegmentEnd          = segment.properties.finalX;

        if (normSegmentEnd >= normPhase)
        {
            segmentIndex = i;
            break;
        }   
    }

    const double normPhaseForSegment  = 1.0 - (normSegmentEnd - normPhase)/ segments[segmentIndex].properties.getNormLengthSamples();

    return segments[segmentIndex].getSample(normPhaseForSegment);
}

double Envelope::getValueForSegment   (int segmentIndex, double normPhase) const
{
    if (segmentIndex >= segments.size())
        return nan("");
    return segments[segmentIndex].getSample(normPhase);
}

/**
 *  Segment, SegmentDescriptor Class Methods
 */

double SegmentDescriptor::getNormLengthSamples() const
{
    return finalX - initialX;
}

Segment::Segment(SegmentDescriptor descriptor, size_t newLength)
{
    normX               = 0.0f;
    properties          = descriptor;
    totalLength         = newLength;
    currentSampleIndex  = 0;

    dx          = 1 / (properties.getNormLengthSamples() * totalLength); 
    isLinear    = (properties.curvature == 0.0f);

    table.initialiseTable(properties.curveType, properties.curvature);
}

double Segment::getSample(double normPhase) const
{
    if (isLinear)
        return properties.initialValue + normPhase * (properties.finalValue - properties.initialValue);

    else
        return properties.initialValue + table.getNormalisedInterpolatedSample(normPhase) * (properties.finalValue - properties.initialValue);
}

bool Segment::getNextSample(double& sample)
{
    sample  = getSample(normX);
    normX  += dx;

    currentSampleIndex++;
    bool isEndOfSegment = (currentSampleIndex >= totalLength * properties.getNormLengthSamples());

    return isEndOfSegment;
}