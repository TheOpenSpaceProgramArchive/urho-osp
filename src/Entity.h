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

public:
    Entity(Context* context);

    Vector3 m_staticCoM;
    float m_staticMass;

//private:

};

}
