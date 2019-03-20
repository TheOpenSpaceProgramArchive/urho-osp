#pragma once

#include <Urho3D/Scene/LogicComponent.h>

#include "Satellite.h"

using namespace Urho3D;

namespace osp
{

/**
 * Anything in the universe that has physics, like a space craft
 */
class Entity : public Satellite
{

    URHO3D_OBJECT(Entity, Satellite)

public:
    Entity(Context* context);

    static void RegisterObject(Context* context);

    virtual void FixedUpdate(float timeStep) override;

    Vector3 m_staticCoM;
    float m_staticMass;

//private:

};

}
