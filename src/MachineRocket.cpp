


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
    m_curveInputs->operator[]("Throttle") = 0.0f;
    m_thrust.add_factor("Throttle", 100.0f, 0.0f);
    m_thrust.set_linear("Throttle", 0, 65535);

    // TODO: on activate event
}

void MachineRocket::FixedUpdate(float timeStep)
{
    // Lots of trial and error happend in this function
    //printf("aaaaa %f\n", m_curveInputs->operator[]("Throttle"));
    RigidBody* rb = node_->GetParent()->GetComponent<RigidBody>();
    if (rb == NULL)
    {
        return;
    }
    if (m_rocketSound == NULL)
    {
        m_rocketSound = node_->CreateComponent<SoundSource>();
        m_rocketSound->SetSoundType(SOUND_EFFECT);
        Sound* sound = GetSubsystem<ResourceCache>()->GetResource<Sound>("Sfx/Rocket2.ogg");
        sound->SetLooped(true);
        m_rocketSound->SetFrequency(44100.0f);
        m_rocketSound->Play(sound);
    }
    //CollisionShape* collider = static_cast<CollisionShape*>(&(node_->GetParent()->GetComponents().At[node_->GetVar("colliderIndices").Get<Vector<int>>()[0]]));
    //Vector<int>* indices = static_cast<Vector<int>*>(node_->GetVar("Colliders").GetVoidPtr());
    //const VariantVector* colliders = node_->GetVar("Colliders").GetVariantVector();
    //printf("aaa %s\n", collider->GetNode()->GetName().CString());
    //for(uint j = 0; j < colliders.Size(); j ++)
    //{
    //    printf("aaa %p\n", colliders[j].GetVoidPtr());
    //}

    VariantVector colliders = node_->GetVar("Colliders").GetVariantVector();
    CollisionShape* collider = static_cast<CollisionShape*>(colliders[0].GetPtr());
    //printf("COM: %s\n", rb->GetCenterOfMass().ToString().CString());

    Input* i = GetSubsystem<Input>();

    // This is mostly temporary
    m_curveInputs->operator[]("Throttle") = Clamp(m_curveInputs->operator[]("Throttle") + (int(i->GetKeyDown(KEY_F)) - i->GetKeyDown(KEY_G)) * 1.0f, 0.0f, 100.0f);
    m_rocketSound->SetFrequency(44100.0f * (m_curveInputs->operator[]("Throttle") / 70.0f + 0.3f));
    m_rocketSound->SetGain(m_curveInputs->operator[]("Throttle") / 100.0f);

    // Thrust related things
    Vector3 what(collider->GetPosition());
    rb->ApplyForce(rb->GetRotation() * Vector3(0, m_thrust.get_float(10.0f), 0), rb->GetRotation() * collider->GetPosition() + rb->GetCenterOfMass());
    //rb->GetBody()->applyForce(ToBtVector3(rb->GetRotation() * Vector3(0, m_thrust.get_float(10.0f), 0)), rb->GetRotation() * collider->GetPosition());

    // Rotation control (temporary)
    rb->ApplyTorque(Vector3((int(i->GetKeyDown(KEY_W)) - i->GetKeyDown(KEY_S)) * 0.5f, 0, (int(i->GetKeyDown(KEY_A)) - i->GetKeyDown(KEY_D)) * 0.5f));
}
