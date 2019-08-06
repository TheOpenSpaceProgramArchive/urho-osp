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

    AstronomicalBody()
    {

    };

    float get_radius()
    {
        return m_radius;
    }

    void load(ActiveArea *area) const override;

    void unload() const override;

private:

    float m_radius; // sea level

};

}
