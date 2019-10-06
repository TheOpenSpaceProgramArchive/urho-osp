#pragma once

#include "Satellite.h"

namespace osp
{

/**
 * Class containing data describing an astronomical body
 * [star, planet, moon, asteroid, etc...],
 * and their properties (size, sea level, mass)
 */
class AstronomicalBody : public Satellite
{
    URHO3D_OBJECT(AstronomicalBody, Satellite)

public:
    AstronomicalBody(Context* context);
    ~AstronomicalBody() = default;

    constexpr float get_radius();

    Node* load(ActiveArea* area, const Vector3& pos) override;

    Node* load_preview(ActiveArea* area) override;

    void unload() override;

private:
    // Maybe allow being loaded multiple times

    // Minimum height, for now
    float m_radius;
};

inline AstronomicalBody::AstronomicalBody(Context* context)
 : Satellite(context)
{
    m_loadRadius = 5000 * 1024;
    m_radius = 4000.0f;
    m_name = "Untitled Moon?";
}

inline Node* AstronomicalBody::load_preview(ActiveArea *area)
{
    // TODO
    return nullptr;
}

inline void AstronomicalBody::unload()
{
    m_activeNode->Remove();
}

constexpr float AstronomicalBody::get_radius()
{
    return m_radius;
}

} // namespace osp
