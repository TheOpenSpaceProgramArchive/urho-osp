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

public:

    AstronomicalBody();
    ~AstronomicalBody() override;

    float get_radius()
    {
        return m_radius;
    }

    void load(ActiveArea *area) override;

    void unload() override;

private:

    // Minimum height, for now
    float m_radius;

};

}
