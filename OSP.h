#ifndef OSP_H
#define OSP_H

#include <Urho3D/Scene/LogicComponent.h>
#include <Urho3D/Physics/RigidBody.h>

#include "PlanWren.h"

using namespace Urho3D;


class AstronomicalBody : public LogicComponent {
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

#endif
