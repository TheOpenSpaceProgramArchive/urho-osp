#include <Urho3D/Graphics/ParticleEmitter.h>

#include "Machines.h"

using namespace osp;

MachineRocket::MachineRocket(Context* context) : Machine(context),
    m_curveInputs(),
    m_thrust(&m_curveInputs, 0.0f, 10.0f),
    m_efficiency(&m_curveInputs, 0.0f, 10.0f)
{
    //m_curveInputs = new HashMap<StringHash, float>();
    //m_thrust(m_curveInputs, 0.0f, 10.0f);
    //m_efficiency(m_curveInputs, 0.0f, 10.0f);

    // Add throttle as a curve input from 0.0-100.0 linear
    m_curveInputs["Throttle"] = 0.0f;
    m_thrust.add_factor("Throttle", 100.0f, 0.0f);
    m_thrust.set_linear("Throttle", 0, 65535);

    // TODO: on activate event
}

MachineRocket::~MachineRocket()
{
    //delete m_curveInputs;
}

void MachineRocket::FixedUpdate(float timeStep)
{
    // Most of the code in this function is temporary.
    // Only here to fake a rocket game,

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

    // hard coded just to look cool
    if (m_plume.Null())
    {
        Node* plume = GetScene()->InstantiateJSON(GetSubsystem<ResourceCache>()->GetResource<JSONFile>("Data/Objects/Plume1.json")->GetRoot(), Vector3(0.0f, -0.5f, 0.0f), Quaternion(0, 0, 0));
        plume->Scale(2.3f);

        PODVector<ParticleEmitter*> plumes;
        plume->GetComponents<ParticleEmitter>(plumes);
        plumes[0]->SetEmitting(true);
        plumes[1]->SetEmitting(true);

        node_->AddChild(plume);
        m_plume = plume;
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
    m_curveInputs["Throttle"] = Clamp(m_curveInputs["Throttle"] + (int(i->GetKeyDown(KEY_F)) - i->GetKeyDown(KEY_G)) * 1.0f, 0.0f, 100.0f);

    // This is mostly temporary
    m_rocketSound->SetFrequency(44100.0f * (m_curveInputs["Throttle"] / 70.0f + 0.3f));
    m_rocketSound->SetGain(m_curveInputs["Throttle"] / 100.0f);

    // Set rocket plume
    //m_plume->SetScale(m_curveInputs["Throttle"] > 2 ? 2.6f : 0.0f);
    PODVector<ParticleEmitter*> plumes;
    m_plume->GetComponents<ParticleEmitter>(plumes);
    plumes[0]->SetEmitting(m_curveInputs["Throttle"] > 2);
    plumes[1]->SetEmitting(m_curveInputs["Throttle"] > 2);

    // Thrust related things
    Vector3 what(collider->GetPosition());
    rb->ApplyForce(rb->GetRotation() * Vector3(0, m_thrust.get_float(10.0f), 0), rb->GetRotation() * collider->GetPosition() + rb->GetCenterOfMass());
    //rb->GetBody()->applyForce(ToBtVector3(rb->GetRotation() * Vector3(0, m_thrust.get_float(10.0f), 0)), rb->GetRotation() * collider->GetPosition());

    // Rotation control (temporary)
    rb->ApplyTorque(Vector3((int(i->GetKeyDown(KEY_W)) - i->GetKeyDown(KEY_S)) * 0.5f, 0, (int(i->GetKeyDown(KEY_A)) - i->GetKeyDown(KEY_D)) * 0.5f));
}
