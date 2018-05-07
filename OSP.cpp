#include "OSP.h"


AstronomicalBody::AstronomicalBody(Context* context) : LogicComponent(context) {
    SetUpdateEventMask(USE_FIXEDUPDATE);

}

void AstronomicalBody::RegisterObject(Context* context) {

    context->RegisterFactory<AstronomicalBody>();

}

void AstronomicalBody::FixedUpdate(float timeStep) {
    if (planet_.IsReady()) {
        Vector3 dir(GetScene()->GetChild("Camera")->GetPosition() - GetScene()->GetChild("planet")->GetPosition());
        float dist = dir.Length();
        dir /= dist;
        planet_.Update(dist, dir);
    }
}

void AstronomicalBody::Initialize(Context* context, double size) {
    ResourceCache* cache = GetSubsystem<ResourceCache>();
    planet_.Initialize(context, size, GetScene(), cache);
}
