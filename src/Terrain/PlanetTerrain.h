#pragma once

#include <Urho3D/Graphics/StaticModel.h>
#include <Urho3D/Physics/RigidBody.h>

#include "../Satellites/AstronomicalBody.h"
#include "PlanetWrenderer.h"

namespace osp
{

/**
 * A component for drawing planets using PlanetWrenderer.
 * Usually created when loading an AstronomicalBody
 */
class PlanetTerrain : public StaticModel
{
    URHO3D_OBJECT(PlanetTerrain, StaticModel)

public:
    // For Urho3D
    static void RegisterObject(Context* context);

    PlanetTerrain(Urho3D::Context* context);

    /**
     * Subdivide/Unsubdivide, and chunk/unchunk depending on how far the
     * ActiveArea's m_focus is from the planet
     * @param eventType
     * @param eventData
     */
    void lod_update(Urho3D::StringHash eventType, VariantMap& eventData);

    /**
     * When true, lod_update will subscribe to E_UPDATE
     * @param [in] enable a self explanatory boolean value
     */
    void set_lod_update_enabled(bool enable);

    /**
     * Generate preview model
     * @param [in] AstronomicalBody to get parameters from
     */
    void initialize(AstronomicalBody* body);

    PlanetWrenderer* get_planet();

private:
    // Used to generate the planet model
    PlanetWrenderer m_planet;

    // Not yet used
    Urho3D::WeakPtr<RigidBody> m_collider;

    // Associated AstronomicalBody
    Urho3D::WeakPtr<AstronomicalBody> m_body;

    bool m_first;
};



inline PlanetWrenderer* PlanetTerrain::get_planet()
{
    return &m_planet;
}

} // namespace osp
