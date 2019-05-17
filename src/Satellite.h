#pragma once

#include <Urho3D/Scene/LogicComponent.h>

#include "LongVector3.h"

using namespace Urho3D;

namespace osp {

class AstronomicalBody;
class PlanetTerrain;

/**
 * Base class for any physical object in the universe
 */
class Satellite : public LogicComponent
{
    URHO3D_OBJECT(Satellite, LogicComponent)

public:
    Satellite(Context* context) : LogicComponent(context) {}
    virtual LongVector3 get_long_position() = 0;

protected:
    //LongVector3 m_position;
    WeakPtr<AstronomicalBody> orbiting;

};

}
