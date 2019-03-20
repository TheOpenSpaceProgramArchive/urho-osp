#pragma once

#include <Urho3D/Scene/LogicComponent.h>

#include "LongVector3.h"

using namespace Urho3D;

namespace osp {

class AstronomicalBody;
class PlanetTerrain;

/**
 * @brief Base class for anything orbiting or landed on an AstronomicalBody.
 * This should contain orbital data.
 */
class Satellite : public LogicComponent
{
    URHO3D_OBJECT(Satellite, LogicComponent)

public:
    Satellite(Context* context) : LogicComponent(context) {}

    LongVector3 m_position;
    WeakPtr<AstronomicalBody> orbiting;

};

}
