#pragma once

#include <cstdint>

#include <Urho3D/Container/HashMap.h>
#include <Urho3D/Container/Ptr.h>
#include <Urho3D/Container/Vector.h>
#include <Urho3D/Math/StringHash.h>
#include <Urho3D/Math/MathDefs.h>

#include <Urho3D/Core/Object.h>


using namespace Urho3D;

namespace osp
{

class FactorCurve
{

    friend class PerformanceCurves;

public:

    FactorCurve() : m_points(1) {}

    PODVector<uint16_t>& get_points() { return m_points; }
    StringHash get_name() { return m_name; }

protected:
    float m_inputRange, m_inputMinimum;
    StringHash m_name;
    PODVector<uint16_t> m_points;


};

/**
 * @brief Inputs factors, like heat, pressure, or throttle, to scale a number
 *
 */
class PerformanceCurves : public Object
{

    URHO3D_OBJECT(PerformanceCurves, Object)

    //float m_range, m_minimum;
    //SharedPtr<HashMap<StringHash, float>> m_curveInputs;
    //HashMap<StringHash, float>* m_curveInputs;
    Vector<FactorCurve> m_factors;

public:

    PerformanceCurves(Context* context);

    FactorCurve* add_factor(StringHash factor, float range = 1.0f, float minimum = 0.0f);
    FactorCurve* get_factor(StringHash factor);

    void set_linear(StringHash factor, uint16_t low, uint16_t high);
    float calculate_float(const HashMap<StringHash, float>& curveInputs, float f) const;

    PODVector<unsigned char> to_buffer() const;
    void from_buffer(const PODVector<unsigned char>& buffer);

};

}
