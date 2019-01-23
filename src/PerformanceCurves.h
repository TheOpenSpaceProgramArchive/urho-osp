#pragma once

#include <cstdint>

#include <Urho3D/Container/HashMap.h>
#include <Urho3D/Container/Ptr.h>
#include <Urho3D/Container/Vector.h>
#include <Urho3D/Math/StringHash.h>
#include <Urho3D/Math/MathDefs.h>


using namespace Urho3D;

namespace osp
{

struct Curve
{
    float m_inputRange, m_inputMinimum;
    PODVector<uint16_t> m_points;
    StringHash m_factor;
};

/**
 * @brief Inputs factors, like heat, pressure, or throttle, to scale a number
 *
 */
class PerformanceCurves
{

    float m_range, m_minimum;
    //SharedPtr<HashMap<StringHash, float>> m_curveInputs;
    HashMap<StringHash, float>* m_curveInputs;
    Vector<Curve> m_curves;

public:

    PerformanceCurves(HashMap<StringHash, float>* inputs, float range, float minimum);

    void add_factor(StringHash factor, float range = 1.0f, float minimum = 0.0f);
    void set_linear(StringHash factor, uint16_t low, uint16_t high);
    float get_float(float f);

};

}
