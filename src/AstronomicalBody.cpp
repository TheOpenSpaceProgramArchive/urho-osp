#include <Urho3D/Core/Context.h>

#include "AstronomicalBody.h"

using namespace osp;

AstronomicalBody::AstronomicalBody(Context* context) : Satellite(context)
{
    SetUpdateEventMask(USE_FIXEDUPDATE);
    m_radius = 4000.0f;
}

void AstronomicalBody::RegisterObject(Context* context)
{

    context->RegisterFactory<AstronomicalBody>("AstronomicalBody");

}

void AstronomicalBody::FixedUpdate(float timeStep)
{
    //if (planet_.is_ready()) {
    //    Vector3 dir(GetScene()->GetChild("Camera")->GetPosition() - GetScene()->GetChild("planet")->GetPosition());
    //    float dist = dir.Length();
    //    dir /= dist;
        //planet_.Update(dist, dir);
    //}
}
