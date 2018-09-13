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

    static void RegisterObject(Context* context)
    {
        context->RegisterFactory<MachineRocket>("MachineRocket");
    }

    //virtual void FixedUpdate(float timeStep);

    HashMap<StringHash, float>* m_curveInputs;

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
