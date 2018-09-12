#pragma once

#include <Urho3D/Container/HashMap.h>

#include "PerformanceCurves.h"

#include "OSP.h"

namespace osp {


class MachineRocket : public Machine
{
    URHO3D_OBJECT(MachineRocket, Machine)

public:
    MachineRocket(Context* context);

    static void RegisterObject(Context* context);

    virtual void FixedUpdate(float timeStep);

    VariantMap m_curveInputs;

    PerformanceCurves m_thrust;
    PerformanceCurves m_efficiency;
    // efficiency
    // thrust

    // damage
    // heat
    // X orientation & gravity
    // pressure
    // proximity
    // X submerged
    // throttle
    // time


};



}
