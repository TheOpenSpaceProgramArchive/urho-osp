#include <Urho3D/Core/Context.h>
#include <Urho3D/Container/Ptr.h>

#include "ActiveArea.h"
#include "AstronomicalBody.h"
#include "PlanetTerrain.h"

using namespace osp;

AstronomicalBody::AstronomicalBody()
{
    m_radius = 4000.0f;
    m_name = "Untitled";
}

AstronomicalBody::~AstronomicalBody()
{
    //Satellite::~Satellite();
}

void AstronomicalBody::load(ActiveArea* area)
{
    Node* scene = area->GetNode();

    m_activeNode = scene->CreateChild(m_name);
    PlanetTerrain* terrain = m_activeNode->CreateComponent<PlanetTerrain>();
    terrain->initialize(this);

    LongVector3 relativePos = Satellite::calculate_relative_position(
                                        area, this, m_unitsPerMeter);

    Vector3 floatPos;
    floatPos.x_ = float(relativePos.x_) / m_unitsPerMeter;
    floatPos.y_ = float(relativePos.y_) / m_unitsPerMeter;
    floatPos.z_ = float(relativePos.z_) / m_unitsPerMeter;

    URHO3D_LOGINFOF("Center at: %f %f %f", floatPos.x_,
                    floatPos.y_, floatPos.z_);

    m_activeNode->SetPosition(floatPos);
}

void AstronomicalBody::unload()
{
    m_activeNode->Remove();
}
