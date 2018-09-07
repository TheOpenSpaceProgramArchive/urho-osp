#ifndef OSP_H
#define OSP_H

#include <Urho3D/Container/RefCounted.h>
#include <Urho3D/Scene/LogicComponent.h>
#include <Urho3D/Physics/RigidBody.h>

#include "PlanWren.h"

using namespace Urho3D;

class AstronomicalBody;

class LongVector
{
    int64_t m_x, m_y, m_z;
    Vector3 m_fraction;
};

class Sattelite : public LogicComponent
{
    URHO3D_OBJECT(Sattelite, LogicComponent)

public:
    Sattelite(Context* context) : LogicComponent(context) {}

    LongVector m_position;
    WeakPtr<AstronomicalBody> orbiting;

};

class AstronomicalBody : public Sattelite
{
    URHO3D_OBJECT(AstronomicalBody, Sattelite)

public:
    AstronomicalBody(Context* context);

    static void RegisterObject(Context* context);

    virtual void FixedUpdate(float timeStep);

    void Initialize(Context* context, double size);

private:
    PlanWren planet_;
    WeakPtr<RigidBody> collider_;
};

class Entity : public Sattelite
{

    URHO3D_OBJECT(Entity, Sattelite)

public:
    Entity(Context* context);

    static void RegisterObject(Context* context);

    virtual void FixedUpdate(float timeStep);

    Vector3 m_staticCoM;
    float m_staticMass;

//private:

};

//class OspPart
//{
//    String m_id;
//
//    String m_name;
//    String m_desc;
//};

class SystemOsp : public Object
{
    SharedPtr<Scene> m_hiddenScene;
    SharedPtr<Node> m_parts;
    //Vector<OspPart>
    // list of countried, manufacturers, and other stuff

    URHO3D_OBJECT(SystemOsp, Object)
public:
    SystemOsp(Context* context);

    Scene* get_hidden_scene() { return m_hiddenScene; }
    void make_craft(Node* node);
};

#endif
