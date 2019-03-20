#include <Urho3D/Core/Context.h>

#include "Entity.h"

using namespace osp;

Entity::Entity(Context* context) : Satellite(context) {
    SetUpdateEventMask(USE_FIXEDUPDATE);
}

void Entity::RegisterObject(Context* context) {

    context->RegisterFactory<Entity>();

}

void Entity::FixedUpdate(float timeStep) {
    //RigidBody* a = static_cast<RigidBody*>(node_->GetComponent("RigidBody"));
    //Vector3 planetPos = node_->GetScene()->GetChild("Planet")->GetPosition();
    // Gravity equation, probably not very precise and efficient here

    //float moon = 88200000;
    //Vector3 gravity = (planetPos - node_->GetPosition());
    //float r = gravity.Length();
    //gravity = gravity / r;
    //gravity *= moon / (r * r);
    //printf("gravity: (%f, %f, %f)\n", gravity.x_, gravity.y_, gravity.z_);
    //a->SetGravityOverride(gravity);
    //printf("AAAA\n");
}
