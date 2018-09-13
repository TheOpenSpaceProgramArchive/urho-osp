#include "Machines.h"

using namespace osp;

MachineRocket::MachineRocket(Context* context) : Machine(context),
    m_curveInputs(new HashMap<StringHash, float>()),
    m_thrust(m_curveInputs, 0.0f, 10.0f),
    m_efficiency(m_curveInputs, 0.0f, 10.0f)
{
    //m_curveInputs = new HashMap<StringHash, float>();
    //m_thrust(m_curveInputs, 0.0f, 10.0f);
    //m_efficiency(m_curveInputs, 0.0f, 10.0f);
}
