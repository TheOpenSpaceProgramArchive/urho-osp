#pragma once

#include "Satellite.h"

namespace osp {

/**
 * Class containing data describing an astronomical body (star, planet, asteroid), and their properties (size, water level, mass)
 */
class AstronomicalBody : public Satellite
{
    URHO3D_OBJECT(AstronomicalBody, Satellite)

public:
    AstronomicalBody(Context* context);

    static void RegisterObject(Context* context);
    virtual void FixedUpdate(float timeStep) override;

    float get_radius() { return m_radius; }

private:

    float m_radius; // sea level
    WeakPtr<AstronomicalBody> m_orbiting;

};

}
