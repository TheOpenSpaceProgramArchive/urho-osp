#include <iostream>

#include "PerformanceCurves.h"

using namespace Urho3D;
using namespace osp;

PerformanceCurves::PerformanceCurves(HashMap<StringHash, float> *inputs, float range, float minimum) : m_curveInputs(inputs), m_curves()
{
}

float PerformanceCurves::get_float(float f)
{
    float value = f;
    for (int i = 0; i < m_curves.Size(); i ++)
    {
        Curve* curve = &(m_curves[i]);
        // get the number, then scale to array index
        float factor = (*m_curveInputs)[curve->m_factor];
        factor = Clamp((factor - curve->m_inputMinimum) * (curve->m_points.Size() - 1) / curve->m_inputRange, 0.0f, float(curve->m_points.Size() - 1));
        // see which 2 numbers in m_points correspond
        float a = curve->m_points[FloorToInt(factor)] / 65535.0f;
        float b = curve->m_points[CeilToInt(factor)] / 65535.0f;
        //printf("F: %i %i, A: %f B: %f\n", FloorToInt(factor), CeilToInt(factor), a, b);
        // Interpolate between the two numbers
        value *= Lerp(a, b, Fract(factor));

    }
    //printf("Out: %f\n", value);
    return value;
}

void PerformanceCurves::add_factor(StringHash factor, float range, float minimum)
{

    Curve curve;
    curve.m_inputMinimum = minimum;
    curve.m_inputRange = range;
    curve.m_points.Push(65535);
    curve.m_points.Push(65535);
    curve.m_factor = factor;

    // Put curve into the array in ascending order
    for (int i = 0; i < m_curves.Size(); i ++)
    {
        if (m_curves[i].m_factor > factor)
        {
            m_curves.Insert(i, curve);
            return;
        }
    }
    // Push to end if array is empty or there's no value larger than hash
    m_curves.Push(curve);
}

void PerformanceCurves::set_linear(StringHash factor, uint16_t low, uint16_t high)
{
    for (int i = 0; i < m_curves.Size(); i ++)
    {
        if (m_curves[i].m_factor == factor)
        {
            m_curves[i].m_points.Resize(2);
            m_curves[i].m_points[0] = low;
            m_curves[i].m_points[1] = high;
            return;
        }
    }
    // talk about how the factor wasn't found
}
