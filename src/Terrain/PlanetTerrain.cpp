#include "PlanetTerrain.h"

#include <Urho3D/Core/Context.h>

namespace osp
{

PlanetTerrain::PlanetTerrain(Context* context) : StaticModel(context),
                                                    m_first(false)
{
    //SetUpdateEventMask(USE_UPDATE);
    //SubscribeToEvent(E_UPDATE, URHO3D_HANDLER(PlanetTerrain, UpdatePlanet));
}

void PlanetTerrain::lod_update(StringHash eventType, VariantMap& eventData)
{

}

void PlanetTerrain::set_lod_update_enabled(bool enable)
{
    if (enable == HasSubscribedToEvent(E_UPDATE))
    {
        // Already enabled/disabled
        return;
    }

    if (enable)
    {
        SubscribeToEvent(E_UPDATE, URHO3D_HANDLER(PlanetTerrain, lod_update));
    }
    else
    {
        UnsubscribeFromEvent(E_UPDATE);
    }
}

void PlanetTerrain::initialize(AstronomicalBody* body)
{
    Image* heightMap = GetSubsystem<ResourceCache>()
                    ->GetResource<Image>("Textures/EquirectangularHeight.png");
    m_planet.initialize(context_, heightMap, body->get_radius());

    Material* planetMaterial = GetSubsystem<ResourceCache>()
                            ->GetResource<Material>("Materials/Planet.xml");
    SetModel(m_planet.get_model());
    //m->SetCullMode(CULL_NONE);
    //m->SetFillMode(FILL_WIREFRAME);
    SetMaterial(planetMaterial);
    SetCastShadows(false);
}

void PlanetTerrain::RegisterObject(Context* context)
{
    context->RegisterFactory<PlanetTerrain>("PlanetTerrain");
}

} // namespace osp
