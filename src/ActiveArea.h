#pragma once

#include <Urho3D/Scene/LogicComponent.h>

#include "Satellite.h"

using namespace Urho3D;

namespace osp {

class ActiveArea;

/**
 * Turns an ordinary Urho3D scene into a window to the larger OSP universe
 * Loads and unloads Satellites, and handles floating origin
 */
class ActiveArea : public LogicComponent, public Satellite
{
    URHO3D_OBJECT(ActiveArea, LogicComponent)

public:

    ActiveArea(Context* context);
    ~ActiveArea();

    virtual void FixedUpdate(float timeStep) override;

    static void RegisterObject(Context* context);

    //void relocate(AstronomicalBody* body, const LongVector3& localBodyPos);

    Satellite* get_focus() const { return m_focus; };
    void set_focus(Satellite* sat) { m_focus = sat; };

    void load(ActiveArea *area) override;
    void unload() override;

private:

    LongVector3 m_localBodyPos;
    WeakPtr<Satellite> m_focus;

};

}
