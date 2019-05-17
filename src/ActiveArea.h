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
    virtual void FixedUpdate(float timeStep) override;

    static void RegisterObject(Context* context);

    void relocate(AstronomicalBody* body, const LongVector3& localBodyPos);

    Satellite* get_focus() const { return m_focus; };

    void set_terrain(PlanetTerrain* terrain);
    void set_focus(Satellite* sat) { m_focus = sat; };

private:

    LongVector3 m_localBodyPos;
    WeakPtr<AstronomicalBody> m_localBody;
    WeakPtr<PlanetTerrain> m_terrain;
    WeakPtr<Satellite> m_focus;

};

}
