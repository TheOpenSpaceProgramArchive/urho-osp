#pragma once

#include "OSP.h"

using namespace Urho3D;

namespace osp {

class ActiveArea : public LogicComponent
{
    URHO3D_OBJECT(ActiveArea, LogicComponent)

public:
    ActiveArea(Context* context);
    virtual void FixedUpdate(float timeStep);

private:

    LongVector3 m_position;
    WeakPtr<AstronomicalBody> m_orbiting;

};

}
