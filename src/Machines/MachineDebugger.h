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
#include <Urho3D/Core/Variant.h>

#include "../Resource/PerformanceCurves.h"

#include "../OspUniverse.h"
#include "Machine.h"


namespace osp {

/**
 * @brief Makes a part apply thrust according to a PerformanceCurve
 */
class MachineDebugger : public Machine
{
    URHO3D_OBJECT(MachineDebugger, Machine)

    //float m_throttle;

public:
    MachineDebugger(Context* context);
    ~MachineDebugger();

    static void RegisterObject(Context* context)
    {
        context->RegisterFactory<MachineDebugger>();

    }

    void DelayedStart() override;
    void FixedUpdate(float timeStep) override;

    void load_json(const JSONObject& machine) override;

private:

};



}
