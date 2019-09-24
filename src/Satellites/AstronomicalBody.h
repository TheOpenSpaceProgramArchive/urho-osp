#pragma once

#include "Satellite.h"

namespace osp {

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
    ~AstronomicalBody() override;

    constexpr float get_radius()
    {
        return m_radius;
    }

    Node* load(ActiveArea* area, const Vector3& pos) override;

    Node* load_preview(ActiveArea* area) override;

    void unload() override;

private:

    // Maybe allow being loaded multiple times

    // Minimum height, for now
    float m_radius;

};

}
