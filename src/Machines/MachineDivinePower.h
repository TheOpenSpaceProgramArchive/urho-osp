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
 * A Machine designed to the laws of physics for the purpose of debugging,
 * testing, and fun
 */
class MachineDivinePower : public Machine
{
    URHO3D_OBJECT(MachineDivinePower, Machine)

    //float m_throttle;

public:
    MachineDivinePower(Context* context);
    ~MachineDivinePower();

    static void RegisterObject(Context* context)
    {
        context->RegisterFactory<MachineDivinePower>();

    }

    void loaded_active() override {}
    void loaded_editor() override {}

    void load_json(const JSONObject& machine) override;

private:

};



}