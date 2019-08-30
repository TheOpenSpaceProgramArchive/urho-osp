#include <iostream>

#include <Urho3D/IO/Log.h>

#include "PerformanceCurves.h"

using namespace Urho3D;
using namespace osp;

PerformanceCurves::PerformanceCurves(Context* context) : Object(context), m_factors()
{
}

float PerformanceCurves::calculate_float(const HashMap<StringHash, float>& curveInputs, float f) const
{
    float value = f;
    for (const FactorCurve& curve : m_factors)
    {
        // get the number, then scale to array index
        if (float* in = curveInputs[curve.m_name])
        {
            float factor = *in;
            factor = Clamp((factor - curve.m_inputMinimum) * (curve.m_points.Size() - 1) / curve.m_inputRange, 0.0f, float(curve.m_points.Size() - 1));
            // see which 2 numbers in m_points correspond
            float a = curve.m_points[FloorToInt(factor)] / 65535.0f;
            float b = curve.m_points[CeilToInt(factor)] / 65535.0f;
            //printf("F: %i %i, A: %f B: %f\n", FloorToInt(factor), CeilToInt(factor), a, b);
            // Interpolate between the two numbers
            value *= Lerp(a, b, Fract(factor));
        }
    }
    //printf("Out: %f\n", value);
    return value;
}

FactorCurve* PerformanceCurves::get_factor(StringHash name)
{
    // Loop through all factors and return the one with matching name
    // consider changing this to a binary search or something later on
    for (FactorCurve& factor : m_factors)
    {
        if (factor.m_name == name)
        {
            return &factor;
        }
    }

    return nullptr;
}

FactorCurve* PerformanceCurves::add_factor(StringHash factor, float range, float minimum)
{

    FactorCurve curve;
    curve.m_inputMinimum = minimum;
    curve.m_inputRange = range;
    //curve.m_points.Push(65535);
    curve.m_points.Push(65535);
    curve.m_name = factor;

    // Put curve into the array in ascending order
    //for (int i = 0; i < m_factors.Size(); i ++)
    //{
    //    if (m_factors[i].m_name > factor)
    //    {
    //         m_factors.Insert(i, curve);
    //        return nullptr;
    //    }
    //}

    // Push to end if array is empty or there's no value larger than hash
    m_factors.Push(curve);

    return (m_factors.End() - 1).ptr_;
}

void PerformanceCurves::set_linear(StringHash name, uint16_t low, uint16_t high)
{
    if (FactorCurve* curve = get_factor(name))
    {
        curve->m_points.Resize(2);
        curve->m_points[0] = low;
        curve->m_points[1] = high;
    }
    // else talk about how the factor wasn't found
}


PODVector<unsigned char> PerformanceCurves::to_buffer() const
{
    // This probably isn't a good method but it works

    PODVector<unsigned char> buffer;

    unsigned bufferSize;

    bufferSize += sizeof(StringHash);
    bufferSize += sizeof(float) * 2;

    buffer.Reserve((sizeof(float) * 2 + sizeof(StringHash) + sizeof(unsigned) ));

    return buffer;
}

void PerformanceCurves::from_buffer(const PODVector<unsigned char>& buffer)
{

}
