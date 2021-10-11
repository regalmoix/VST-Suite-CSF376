#include "FunctionTable.h"
#ifndef _USE_MATH_DEFINES
  #define _USE_MATH_DEFINES
#endif
#include <cmath>

CurveType& operator++(CurveType& curve)
{
    int curveIndex = (static_cast<int>(curve) + 1) % CurveType::NUM_CURVE_TYPES;
    curve = static_cast<CurveType>(curveIndex); 
    return curve;
}

FunctionTable::FunctionTable()
{
    funcTable.resize(DEFAULT_TABLE_SIZE);
}

FunctionTable::~FunctionTable()
{

}

void FunctionTable::initialiseTable (CurveType curveType, float curvature)
{
    switch (curveType)
    {
        case CurveType::Linear:
        {
            linearCurve();
            break;
        }

        case CurveType::Exponential:
        {
            if (curvature == 0.0f)
            {
                linearCurve();
                return;
            }

            exponentialCurve(curvature);
            break;
        }    

        case CurveType::Power:
        {
            if (curvature == 0.0f)
            {
                linearCurve();
                return;
            }
            powerCurve(pow(2.0f, -curvature));
            break;
        }  
        
        case CurveType::QSine:
        {
            if (curvature == MIN_CURVATURE)
            {
                linearCurve();
                return;
            }
            
            qSineCurve((curvature - MIN_CURVATURE)/(MAX_CURVATURE - MIN_CURVATURE));
            break;
        }

        case CurveType::QCos:
        {
            if (curvature == MAX_CURVATURE)
            {
                linearCurve();
                return;
            }
            
            qCosineCurve((MAX_CURVATURE - curvature)/(MAX_CURVATURE - MIN_CURVATURE));
            break;   
        }

        case CurveType::NUM_CURVE_TYPES:
        {
            linearCurve();
            break;
        }
    }
}

void FunctionTable::exponentialCurve(float curvature)
{
    float left      = -curvature;
    float right     = +curvature;

    float bottom    = -exp(-left);
    float top       = -exp(-right);

    float vscale    = 1.0f / (top - bottom);
        
    float x     = left;
    float dx    = (right - left) / (funcTable.size() - 1);

    for (float& sample : funcTable)
    {
        sample  = vscale * (-exp(-x) - bottom);
        x      += dx;
    }
}

void FunctionTable::powerCurve(float exponent)
{
    float x  = 0.0f;
    float dx = 1.0f / (funcTable.size() - 1);

    for (float& sample : funcTable)
    {
        sample  = pow(x, exponent);
        x      += dx;
    }
}

void FunctionTable::qSineCurve(float a)
{
    float x  = 0.0f;
    float dx = M_PI_2 / (funcTable.size() - 1);

    for (float& sample : funcTable)
    {
        sample  = std::sin(a * x) / std::sin(a * M_PI_2);
        x      += dx;
    }
}

void FunctionTable::qCosineCurve(float a)
{
    float x  = M_PI_2;
    float dx = M_PI_2 / (funcTable.size() - 1);

    for (float& sample : funcTable)
    {
        sample  = 1 - std::sin(a * x) / std::sin(a * M_PI_2);
        x      -= dx;
    }
}

void FunctionTable::linearCurve()
{
    float x     = 0;
    float dx    = 1.0f / float(funcTable.size() - 1);

    for (float& sample : funcTable)
    {
        sample  = x;
        x      += dx;
    }
}

float FunctionTable::getNormalisedInterpolatedSample(float phase) const
{
    if (phase < 0) 
        return funcTable[0];

    else if (phase >= 1.0) 
        return funcTable.back();

    float   readIndexFloat  = phase * funcTable.size();
    size_t  readIndex0      = (size_t) readIndexFloat;
    size_t  readIndex1      = (size_t) readIndexFloat + 1;
    float   fraction        = readIndexFloat - readIndex0;
    
    // Wrap Around the table.
    if (readIndex1 == funcTable.size())
        readIndex1  = funcTable.size() - 1;

    float   sample0         = funcTable[readIndex0];
    float   sample1         = funcTable[readIndex1];
    float   interpolated    = sample0 * (1 - fraction) + sample1 * fraction;

    return  interpolated;
}