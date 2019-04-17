#include "PlanetTerrain.h"

#include <Urho3D/Core/Context.h>

using namespace osp;

PlanetTerrain::PlanetTerrain(Context* context) : StaticModel(context), m_first(false)
{
    //SetUpdateEventMask(USE_UPDATE);
    //SubscribeToEvent(E_UPDATE, URHO3D_HANDLER(PlanetTerrain, UpdatePlanet));
}

void PlanetTerrain::initialize(AstronomicalBody* body)
{
    m_planet.initialize(context_, GetSubsystem<ResourceCache>()->GetResource<Image>("Textures/EquirectangularHeight.png"), body->get_radius());
    Material* m = GetSubsystem<ResourceCache>()->GetResource<Material>("Materials/Planet.xml");
    SetModel(m_planet.get_model());
    //m->SetCullMode(CULL_NONE);
    //m->SetFillMode(FILL_WIREFRAME);
    SetMaterial(m);
    SetCastShadows(false);
}

void PlanetTerrain::RegisterObject(Context* context)
{
    context->RegisterFactory<PlanetTerrain>("PlanetTerrain");
}

/**
 * @brief PlanetTerrain::UpdatePosition
 * @param activePosition [in] Center of the ActiveArea relative to this planet
 */
void PlanetTerrain::UpdatePosition(const LongVector3& activePosition)
{
    // negative activePosition is where the planet should be relative to the urho origin

    Vector3 newPos(activePosition.x_, activePosition.y_, activePosition.z_);

    // negate and divide by 1000, since localBodyPos is in millimeters
    node_->SetPosition(newPos * -0.001f);

    //URHO3D_LOGINFOF("Updated position %s", node_->GetPosition().ToString().CString());

}
