#pragma once

#include "OspUniverse.h"

using namespace Urho3D;

namespace osp {

/**
 * Makes a scene a part of the OSP universe
 * It handles floating origin
 */
class ActiveArea : public LogicComponent
{
    URHO3D_OBJECT(ActiveArea, LogicComponent)

public:
    ActiveArea(Context* context);
    virtual void FixedUpdate(float timeStep);

    static void RegisterObject(Context* context);

    void set_terrain(PlanetTerrain* terrain);
    void relocate(AstronomicalBody* body, const LongVector3& localBodyPos);

private:

    LongVector3 m_localBodyPos;
    WeakPtr<AstronomicalBody> m_localBody;
    WeakPtr<PlanetTerrain> m_terrain;

};

}
