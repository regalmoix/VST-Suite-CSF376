#pragma once
#include <vector>
#include <cmath>


#define DEFAULT_TABLE_SIZE 256
#define MIN_CURVATURE -6.0f
#define MAX_CURVATURE +6.0f


using std::vector;

enum CurveType
{
    Linear,
    Exponential,
    Power,
    QSine,          // Quarter of a sine wave
    QCos,           // Quarter of a cos  wave
    NUM_CURVE_TYPES
};

CurveType& operator++(CurveType&);

struct FunctionTable
{
public:
    vector<float>   funcTable;

    FunctionTable   ();
    ~FunctionTable  ();

    /**
     * @brief Get the Normalised Interpolated Sample for the normalised phase
     * @param normPhase 
     * @return normalised sample conforming to the exp or pow curve
     */
    float getNormalisedInterpolatedSample   (float normPhase) const;
    
    /**
     * @brief Initialize the FunctionTable according to a curve
     * @param curveType The type of curve to initialise the table with
     * @param curvature The parameter used to define extent of curvature
     */
    void initialiseTable    (CurveType curveType, float curvature);

protected:
    /**
     * @brief Initialize the FunctionTable to an exponential shape, scaled to fit in the unit square.
     * @param curvature  Decides left and right bounds for the function y = -exp(-x)
     * @details 
     *      The function itself is y = -exp(-x), where x ranges from 'left' to 'right'.
     *      The more negative 'left' is, the more vertical the start of the rise; -5.0 yields near-vertical.
     *      The more positive 'right' is, the more horizontal then end of the rise; +5.0 yields near-horizontal.
     */
    void exponentialCurve   (float curvature);

    /**
     * @brief Initialize the FunctionTable to a power-curve shape, defined in the unit square.
     * @param exponent positive for concave-up shape, negative for concave-down. Typical range [-4, 5]
     */
    void powerCurve         (float exponent);

    /** @brief Initialize the FunctionTable to a linear shape y = x, defined in the unit square. */
    void linearCurve        ();

    /** @brief Initialize the FunctionTable to a quarter cycle of sine shape y = sin(ax), defined in the unit square. */
    void qSineCurve         (float a);

    /** @brief Initialize the FunctionTable to a quarter cycle of cosine shape resembling y = cos(ax), defined in the unit square. */
    void qCosineCurve      (float a);
};
