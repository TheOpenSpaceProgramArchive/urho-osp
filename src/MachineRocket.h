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
#include "Machine.h"


namespace osp {

/**
 * @brief Makes a part apply thrust according to a PerformanceCurve
 */
class MachineRocket : public Machine
{
    URHO3D_OBJECT(MachineRocket, Machine)

    //float m_throttle;

public:
    MachineRocket(Context* context);
    ~MachineRocket();

    static void RegisterObject(Context* context)
    {
        context->RegisterFactory<MachineRocket>();

        URHO3D_ACCESSOR_ATTRIBUTE("Enable Thrust Curves", is_curve_thrust_enabled, enable_curve_thrust, bool, true, AM_DEFAULT);
        URHO3D_ACCESSOR_ATTRIBUTE("Enable Efficiency Curves", is_curve_efficiency_enabled, enable_curve_efficiency, bool, true, AM_DEFAULT);
    }

    bool is_curve_thrust_enabled() const { return m_curveThrustEnabled; }
    void enable_curve_thrust(bool enable) { m_curveThrustEnabled = enable; }

    bool is_curve_efficiency_enabled() const { return m_curveEfficiencyEnabled; }
    void enable_curve_efficiency(bool enable) { m_curveEfficiencyEnabled = enable; }

    void FixedUpdate(float timeStep) override;

    void load_json(const JSONObject& machine) override;

private:

    PerformanceCurves m_thrust;
    PerformanceCurves m_efficiency;
    WeakPtr<SoundSource> m_rocketSound;
    WeakPtr<Node> m_plume;

    bool m_curveThrustEnabled;
    bool m_curveEfficiencyEnabled;


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
