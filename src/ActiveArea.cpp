#include "ActiveArea.h"

using namespace osp;
using namespace Urho3D;

ActiveArea::ActiveArea(Context* context) : LogicComponent(context)
{

}

void ActiveArea::FixedUpdate(float timeStep)
{
    if (m_orbiting.NotNull())
    {
        // We in an SOI

        // Calculate where the planet(s) should be



    }
}
