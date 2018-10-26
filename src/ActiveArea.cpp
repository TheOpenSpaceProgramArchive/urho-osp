#include "ActiveArea.h"

using namespace osp;
using namespace Urho3D;

ActiveArea::ActiveArea(Context* context) : LogicComponent(context)
{

}

void ActiveArea::RegisterObject(Context* context)
{
    context->RegisterFactory<ActiveArea>("ActiveArea");
}

void ActiveArea::FixedUpdate(float timeStep)
{
    if (m_terrain.NotNull())
    {
        Vector3 cameraPos = GetSubsystem<Renderer>()->GetViewport(0)->GetCamera()->GetNode()->GetWorldPosition();
        Vector3 planetPos = m_terrain->GetNode()->GetPosition();
        m_terrain->GetPlanet()->update(cameraPos - planetPos);
        m_terrain->UpdatePosition(m_localBodyPos);
    }
}

/**
 * Move the active area to somewhere in the universe
 * @param body The Astronomical Body to move to
 * @param localBodyPos XYZ coordinates relative to that body
 */
void ActiveArea::relocate(AstronomicalBody* body, const LongVector3& localBodyPos)
{
    m_localBodyPos = localBodyPos;
    m_localBody = body;
}

/**
 * Set the PlanetTerrain that should be associated with the current body
 * @param terrain Pointer to an existing terrain component
 */
void ActiveArea::set_terrain(PlanetTerrain* terrain)
{
    m_terrain = WeakPtr<PlanetTerrain>(terrain);
}
