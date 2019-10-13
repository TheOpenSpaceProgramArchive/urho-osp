#pragma once

#include <Urho3D/Scene/Node.h>
#include <Urho3D/Container/Ptr.h>
#include <Urho3D/Container/RefCounted.h>

#include "../LongVector3.h"

namespace osp {

/**
 * Base class for any strategy that describes a path a Satellite can take
 * eg. TrajectoryKeplerOrbit, TrajectoryLanded, etc...
 */
class Trajectory : public Urho3D::Object
{
    URHO3D_OBJECT(Trajectory, Urho3D::Object)

public:


    using Urho3D::Object::Object;
    ~Trajectory() = default;

protected:

    // put stuff here
};

}
