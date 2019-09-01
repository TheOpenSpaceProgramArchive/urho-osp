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

    /**
     * Parse JSON performance curve factors.
     *
     * JSONObjects passed into this function should look like this:
     * {
     *   "pressure": [ 100, 0 ],
     *   "throttle" : [ 0, 4, 80, 100 ]
     * }
     *
     * "pressure": [ 100, 0 ] means that whatever valus is being scaled, like
     * thrust, will be multiplied by 1.0 at vacuum, and multiplied by zero at
     * max pressure.
     *
     * "throttle" : [ 0, 4, 80, 100 ] means that whatever value is being
     * scaled, like thrust again, will be multiplied by 0.0 when throttle is
     * 0%, slowly rise from 0-33% throttle, rapidly increase at 33%-66%, and
     * rise slowly again at 66-100% throttle.
     *
     * @param factors [in] JSONObject to parse
     */
    void parse_curve_factors(const JSONObject& factors);

    FactorCurve* add_factor(StringHash factor, float range = 1.0f,
                            float minimum = 0.0f);
    FactorCurve* get_factor(StringHash factor);

    void set_linear(StringHash factor, uint16_t low, uint16_t high);
    float calculate_float(const HashMap<StringHash,
                          float>& curveInputs, float f) const;

    PODVector<unsigned char> to_buffer() const;
    void from_buffer(const PODVector<unsigned char>& buffer);

};

}
