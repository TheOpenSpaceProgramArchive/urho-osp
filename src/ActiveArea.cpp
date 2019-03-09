#include <Urho3D/Physics/PhysicsWorld.h>
#include <Urho3D/Physics/CollisionShape.h>


#include "ActiveArea.h"

using namespace osp;
using namespace Urho3D;

ActiveArea::ActiveArea(Context* context) : LogicComponent(context)
{

}

void ActiveArea::RegisterObject(Context* context)
{
    context->RegisterFactory<ActiveArea>("ActiveArea");
}

void ActiveArea::FixedUpdate(float timeStep)
{
    if (m_terrain.NotNull())
    {
        PhysicsWorld* pw = node_->GetScene()->GetComponent<PhysicsWorld>();//scene->CreateComponent<PhysicsWorld>();

        Vector3 cameraPos = GetSubsystem<Renderer>()->GetViewport(0)->GetCamera()->GetNode()->GetWorldPosition();
        Vector3 planetPos = m_terrain->GetNode()->GetPosition();

         //CollisionShape* cs = m_terrain->GetNode()->GetComponent<CollisionShape>();
        //cs->SetTriangleMesh(m_terrain->GetPlanet()->get_model(), 0, Vector3::ONE);

        const float originDistance = cameraPos.Length();
        const float threshold = 30.0f;

        if (originDistance > threshold) {
            Vector3 offsetDist = cameraPos * -1000.0f;
            offsetDist.x_ = Floor(offsetDist.x_);
            offsetDist.y_ = Floor(offsetDist.y_);
            offsetDist.z_ = Floor(offsetDist.z_);
            relocate(m_localBody, m_localBodyPos - LongVector3(offsetDist.x_, offsetDist.y_, offsetDist.z_));

            offsetDist *= 0.001f;

            //URHO3D_LOGINFOF("move!");
            // Translate all nodes to move the camera back to the center
            PODVector<Node*> rigidBodies;
            node_->GetChildren(rigidBodies);
            for (Node* rbNode : rigidBodies)
            {
                rbNode->Translate(offsetDist, TS_WORLD);


                // Delete far away nodes
                const VariantMap& vars = rbNode->GetVars();
                Variant removeDistance;
                if (vars.TryGetValue("RemoveDistance", removeDistance))
                {
                    if (rbNode->GetPosition().Length() > removeDistance.GetFloat())
                    {
                        rbNode->Remove();
                        URHO3D_LOGINFO("Distant node removed");
                    }
                }
            }

            //GetSubsystem<Renderer>()->GetViewport(0)->GetCamera()->GetNode()->GetParent()->Translate(offsetDist, TS_WORLD);
        }

        m_terrain->GetPlanet()->update(cameraPos - planetPos);
        m_terrain->UpdatePosition(m_localBodyPos);


        //pw->SetGravity(Vector3::ZERO);

        // some gravity for temporary fun
        float moon = 88200000;
        Vector3 gravity = (m_terrain->GetNode()->GetPosition() - node_->GetChild("CameraCenter")->GetPosition());
        float r = gravity.Length();
        gravity = gravity / r;
        gravity *= moon / (r * r);
        //printf("gravity: (%f, %f, %f)\n", gravity.x_, gravity.y_, gravity.z_);
        pw->SetGravity(gravity);
    }
}

/**
 * Move the active area to somewhere in the universe
 * @param body The Astronomical Body to move to
 * @param localBodyPos XYZ coordinates relative to that body
 */
void ActiveArea::relocate(AstronomicalBody* body, const LongVector3& localBodyPos)
{
    m_localBodyPos = localBodyPos;
    m_localBody = body;
}

/**
 * Set the PlanetTerrain that should be associated with the current body
 * @param terrain Pointer to an existing terrain component
 */
void ActiveArea::set_terrain(PlanetTerrain* terrain)
{
    m_terrain = WeakPtr<PlanetTerrain>(terrain);
}
