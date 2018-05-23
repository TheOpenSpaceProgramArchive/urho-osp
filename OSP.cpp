#include <Urho3D/Physics/CollisionShape.h>

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
    node_->SetPosition(Vector3(0,-size / 2, 0));
    node_->SetScale(Vector3(size / 2,size / 2,size / 2));
    //collider_ = node_->CreateComponent<RigidBody>();
    //CollisionShape* sphere = node_->CreateComponent<CollisionShape>();
    //sphere->SetSphere(1.0f);

}
