#ifndef OSP_H
#define OSP_H

#include <Urho3D/Container/RefCounted.h>
#include <Urho3D/Scene/LogicComponent.h>
#include <Urho3D/Physics/RigidBody.h>

#include "PlanWren.h"

using namespace Urho3D;


class AstronomicalBody : public LogicComponent
{
    URHO3D_OBJECT(AstronomicalBody, LogicComponent)

public:
    AstronomicalBody(Context* context);

    static void RegisterObject(Context* context);

    virtual void FixedUpdate(float timeStep);

    void Initialize(Context* context, double size);

private:
    PlanWren planet_;
    WeakPtr<RigidBody> collider_;
};

class OspInstance : public LogicComponent
{
    URHO3D_OBJECT(OspInstance, LogicComponent)

public:
    OspInstance(Context* context);

    static void RegisterObject(Context* context);

    virtual void FixedUpdate(float timeStep);

private:

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

    Scene* GetHiddenScene() { return m_hiddenScene; }
};

#endif
