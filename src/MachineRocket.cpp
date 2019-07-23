#include <Urho3D/Graphics/ParticleEmitter.h>

#include "MachineRocket.h"

using namespace osp;

MachineRocket::MachineRocket(Context* context) : Machine(context)
{
    //m_curveInputs = new HashMap<StringHash, float>();
    //m_thrust(m_curveInputs, 0.0f, 10.0f);
    //m_efficiency(m_curveInputs, 0.0f, 10.0f);

    // Add throttle as a curve input from 0.0-100.0 linear
    //m_curveInputs["throttle"] = 0.0f;
    //m_thrust.add_factor("throttle", 100.0f, 0.0f);
    //m_thrust.set_linear("throttle", 0, 65535);

    // TODO: on activate event
}

MachineRocket::~MachineRocket()
{
    //delete m_curveInputs;
}

void MachineRocket::DelayedStart()
{
    // Load some data if there's a prototype
    if (Node* prototype = reinterpret_cast<Node*>(node_->GetVar("prototype").GetVoidPtr()))
    {
        if (prototype->GetType() == "Node")
        {
            //URHO3D_LOGINFOF("Q: What colour is the crow? A: Yes.");

            if (MachineRocket* protoRocket = prototype->GetComponent<MachineRocket>())
            {
                // Get te performance curve pointers

                m_thrust = protoRocket->get_curve_thrust();
                m_efficiency = protoRocket->get_curve_efficiency();
                
                m_baseThrust = protoRocket->get_base_thrust();
            }

        }

    }

    m_curveInputs["throttle"] = 0;

    // Start hard-coded thing that look cool

    m_rocketSound = node_->CreateComponent<SoundSource>();
    m_rocketSound->SetSoundType(SOUND_EFFECT);
    Sound* sound = GetSubsystem<ResourceCache>()->GetResource<Sound>("Sfx/Rocket2.ogg");
    sound->SetLooped(true);
    m_rocketSound->SetFrequency(44100.0f);
    m_rocketSound->SetGain(0);
    m_rocketSound->Play(sound);

    Node* plume = GetScene()->InstantiateJSON(GetSubsystem<ResourceCache>()->GetResource<JSONFile>("Data/Objects/Plume1.json")->GetRoot(), Vector3(0.0f, 0.0f, 0.0f), Quaternion(0, 0, 0));
    plume->Scale(2.3f);

    PODVector<ParticleEmitter*> plumes;
    plume->GetComponents<ParticleEmitter>(plumes);
    plumes[0]->SetEmitting(false);
    plumes[1]->SetEmitting(false);

    node_->AddChild(plume);
    m_plume = plume;
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

    //CollisionShape* collider = static_cast<CollisionShape*>(&(node_->GetParent()->GetComponents().At[node_->GetVar("colliderIndices").Get<Vector<int>>()[0]]));
    //Vector<int>* indices = static_cast<Vector<int>*>(node_->GetVar("Colliders").GetVoidPtr());
    //const VariantVector* colliders = node_->GetVar("Colliders").GetVariantVector();
    //printf("aaa %s\n", collider->GetNode()->GetName().CString());
    //for(uint j = 0; j < colliders.Size(); j ++)
    //{
    //    printf("aaa %p\n", colliders[j].GetVoidPtr());
    //}

    VariantVector colliders = node_->GetVar("colliders").GetVariantVector();
    CollisionShape* collider = static_cast<CollisionShape*>(colliders[0].GetPtr());
    //printf("COM: %s\n", rb->GetCenterOfMass().ToString().CString());

    Input* i = GetSubsystem<Input>();
    m_curveInputs["throttle"] = Clamp(m_curveInputs["throttle"] + (int(i->GetKeyDown(KEY_F)) - i->GetKeyDown(KEY_G)) * 1.0f, 0.0f, 100.0f);

    // Set rocket plume
    //m_plume->SetScale(m_curveInputs["throttle"] > 2 ? 2.6f : 0.0f);
    PODVector<ParticleEmitter*> plumes;
    m_plume->GetComponents<ParticleEmitter>(plumes);
    plumes[0]->SetEmitting(m_curveInputs["throttle"] > 2);
    plumes[1]->SetEmitting(m_curveInputs["throttle"] > 2);
    
    // Thrust related things
    // Calculate thrust from curves
    float thrustCalculated = m_thrust->calculate_float(m_curveInputs, m_baseThrust);
    Vector3 what(collider->GetPosition());
    rb->ApplyForce(rb->GetRotation() * Vector3(0, 0, thrustCalculated), rb->GetRotation() * collider->GetPosition() + rb->GetCenterOfMass());
    //rb->GetBody()->applyForce(ToBtVector3(rb->GetRotation() * Vector3(0, m_thrust.get_float(10.0f), 0)), rb->GetRotation() * collider->GetPosition());

    // This is mostly temporary
    // Change rocket sound pitch based on thrust
    m_rocketSound->SetFrequency(44100.0f * (thrustCalculated / m_baseThrust * 2.0 + 0.3f));
    m_rocketSound->SetGain(m_curveInputs["throttle"] / 100.0f);

    // Rotation control (temporary)
    Scene* scene = GetScene();
    rb->SetAngularDamping(0.6f);
    Vector3 torque = Vector3((int(i->GetKeyDown(KEY_W)) - i->GetKeyDown(KEY_S)), 0, (int(i->GetKeyDown(KEY_A)) - i->GetKeyDown(KEY_D)));
    Quaternion cameraRot = scene->GetChild("CameraCenter")->GetChild("Camera")->GetWorldRotation();
    torque = cameraRot * torque * 0.14f;
    //torque *= rb->GetMass();
    rb->SetAngularVelocity(rb->GetAngularVelocity() + torque);
    //rb->ApplyTorque(torque);
}

void MachineRocket::load_json(const JSONObject& machine)
{

    m_thrust.Reset();
    m_efficiency.Reset();
    m_thrust = new PerformanceCurves(context_);
    m_efficiency = new PerformanceCurves(context_);

    m_baseThrust = 100;
    
    if (JSONValue* baseThrust = machine["baseThrust"])
    {
        if (baseThrust->IsNumber())
        {
            m_baseThrust = baseThrust->GetFloat();
        }
    }
    
    if (JSONValue* thrustFactorsValue = machine["thrust"])
    {
        if (thrustFactorsValue->IsObject())
        {
            parse_factors(*m_thrust, thrustFactorsValue->GetObject());
        }
    }

    if (JSONValue* efficiencyFactorsValue = machine["efficiency"])
    {
        if (efficiencyFactorsValue->IsObject())
        {
            parse_factors(*m_efficiency, efficiencyFactorsValue->GetObject());
        }
    }
}
