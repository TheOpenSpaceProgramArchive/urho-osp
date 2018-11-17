#pragma once

#include <Urho3D/Input/Input.h>
#include <Urho3D/Container/Vector.h>
#include <Urho3D/Physics/CollisionShape.h>
#include <Urho3D/Physics/PhysicsUtils.h>
#include <Urho3D/Physics/RigidBody.h>
#include <Urho3D/Audio/Sound.h>
#include <Urho3D/Audio/SoundSource.h>
#include <Urho3D/Audio/SoundSource3D.h>
#include <Urho3D/Container/HashMap.h>

#include "PerformanceCurves.h"

#include "OSP.h"

namespace osp {

/**
 * @brief Makes a part apply thrust according to a PerformanceCurve
 */
class MachineRocket : public Machine
{
    URHO3D_OBJECT(MachineRocket, Machine)

    HashMap<StringHash, float> m_curveInputs;

    PerformanceCurves m_thrust;
    PerformanceCurves m_efficiency;
    WeakPtr<SoundSource> m_rocketSound;
    WeakPtr<Node> m_plume;
    //float m_throttle;

public:
    MachineRocket(Context* context);
    ~MachineRocket();

    static void RegisterObject(Context* context)
    {
        context->RegisterFactory<MachineRocket>("MachineRocket");
    }

    virtual void FixedUpdate(float timeStep);

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
