#pragma once

#include <Urho3D/Graphics/StaticModel.h>
#include <Urho3D/Physics/RigidBody.h>

#include "AstronomicalBody.h"
#include "PlanetWrenderer.h"

namespace osp {

/**
 * @brief LOD Planet terrain that could be rendered
 */
class PlanetTerrain : public StaticModel
{
    URHO3D_OBJECT(PlanetTerrain, StaticModel)

public:
    PlanetTerrain(Context* context);

    static void RegisterObject(Context* context);

    void initialize(AstronomicalBody* body);
    void UpdatePosition(const LongVector3& activePosition);
    PlanetWrenderer* GetPlanet() { return &m_planet; }

private:
    bool m_first;
    PlanetWrenderer m_planet;
    WeakPtr<RigidBody> m_collider;
    WeakPtr<AstronomicalBody> m_body;
};

}
