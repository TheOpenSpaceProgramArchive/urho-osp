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
 * A Machine that models the thrust of a rocket by using PerformanceCurves
 * It has some fun particle effects and sounds thrown in as well
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

        // Set urho attributes so that they its easier to clone and serialize
        // this Machine

        URHO3D_ACCESSOR_ATTRIBUTE("Enable Thrust Curves",
                                  is_curve_thrust_enabled,
                                  set_enable_curve_thrust,
                                  bool, true, AM_DEFAULT);
        URHO3D_ACCESSOR_ATTRIBUTE("Enable Efficiency Curves",
                                  is_curve_efficiency_enabled,
                                  set_enable_curve_efficiency,
                                  bool, true, AM_DEFAULT);
        URHO3D_ACCESSOR_ATTRIBUTE("Enable Efficiency Curves",
                                  is_curve_efficiency_enabled,
                                  set_enable_curve_efficiency,
                                  bool, true, AM_DEFAULT);

    }

    bool is_curve_thrust_enabled() const
    { return m_curveThrustEnabled; }
    void set_enable_curve_thrust(bool enable)
    { m_curveThrustEnabled = enable; }

    bool is_curve_efficiency_enabled() const
    { return m_curveEfficiencyEnabled; }
    void set_enable_curve_efficiency(bool enable)
    { m_curveEfficiencyEnabled = enable; }

    PODVector<unsigned char> get_curve_thrust_buffer() const
    { return m_thrust->to_buffer(); }
    void set_curve_thrust_buffer(const PODVector<unsigned char>& curves)
    { m_thrust->from_buffer(curves); }

    PerformanceCurves* get_curve_thrust() const { return m_thrust; }
    void set_curve_thrust(PerformanceCurves* curves) { m_thrust = curves; }

    PerformanceCurves* get_curve_efficiency() const { return m_efficiency; }
    void set_curve_efficiency(PerformanceCurves* curves)
    { m_efficiency = curves; }
    
    float get_base_thrust() const { return m_baseThrust; }
    void set_base_thrust(float baseThrust) { m_baseThrust = baseThrust; }

    //void DelayedStart() override;
    //void FixedUpdate(float timeStep) override;

    void update_active(StringHash eventType, VariantMap& eventData);

    void loaded_active() override;
    void loaded_editor() override {}

    void load_json(const JSONObject& machine) override;

private:

    // PerformanceCurves that takes inputs such as:
    // * heat
    // * pressure
    // * throttle  (only one implemented so far)
    // * time
    // and outputs a percentage that can be used to scale a value like:
    // * thrust
    // * efficiency
    //
    // Both of these share the same m_curveInput
    SharedPtr<PerformanceCurves> m_thrust;
    SharedPtr<PerformanceCurves> m_efficiency;

    // A cool hard-coded rocket sound
    WeakPtr<SoundSource> m_rocketSound;

    // A node that has ParticleEmitters to look cool
    WeakPtr<Node> m_plume;

    bool m_curveThrustEnabled;
    bool m_curveEfficiencyEnabled;

    float m_baseThrust;


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

} // namespace osp
