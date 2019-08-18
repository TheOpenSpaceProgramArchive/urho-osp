#include <Urho3D/Core/Context.h>
#include <Urho3D/Physics/PhysicsWorld.h>
#include <Urho3D/Physics/CollisionShape.h>

#include "ActiveArea.h"
#include "PlanetTerrain.h"

using namespace osp;
using namespace Urho3D;

ActiveArea::ActiveArea(Context* context) : LogicComponent(context)
{
    m_name = "Untitled ActiveArea";

    // load everything within a kilometer
    m_loadRadius = 1000 * 1024;
}

ActiveArea::~ActiveArea()
{
    //Satellite::~Satellite();
}

void ActiveArea::RegisterObject(Context* context)
{
    context->RegisterFactory<ActiveArea>("ActiveArea");
}

void ActiveArea::FixedUpdate(float timeStep)
{
    m_activeNode = node_;
    
    PhysicsWorld* pw = node_->GetScene()->GetComponent<PhysicsWorld>();

    LongVector3 focusPosLong;

    // Loop through all objects in the tree, while continuously calculating
    // relative positions

    // roughly based on calculate_relative_position in Satellite.cpp

    // TODO: account for differing m_unitsPerMeter and integer over/underflows
    // note: if the number of overflows is equal to the number of underflows,
    //       then the result is correct? I feel this has some similarities to
    //       the idea of "Parallel Universes" in Super Mario 64

    Satellite* satA = this;
    LongVector3 d(0, 0, 0); // Relative position
    IntVector3 overflows(0, 0, 0); // see note above

    // Go up into the root node, adding up positions
    // root has a depth of zero, so this loop will stop there
    while (satA->get_depth())
    {
        d -= satA->get_position();
        satA = satA->get_parent();
    }

    // satA should now be the root node, d being the position relative to this
    // Now distance check and load root node
    distance_check_then_load(satA, d, overflows);

    // Recursive Loop through everything else in the tree
    // while (Not on the last child of root)
    while ((satA->get_depth() != 1)
           || (satA->get_parent()->get_children().Size()
                - satA->get_index() != 1))
    {
        // Check if satA has children
        if (satA->get_children().Size())
        {
            // This node has children, set satA to the first child
            satA = satA->get_children()[0].Get();
            //URHO3D_LOGINFOF("DOWN");
        }
        else
        {
            // satA has no children, move up to ancestor that has a next child
            while (satA->get_parent()->get_children().Size()
                   - satA->get_index() == 1)
            {
                d -= satA->get_position();
                satA = satA->get_parent();
                //URHO3D_LOGINFOF("UP");
            }

            d -= satA->get_position();

            // Move to next child
            satA = satA->get_parent()
                    ->get_children()[satA->get_index() + 1].Get();

        }

        d += satA->get_position();

        // Do distance check and load
        distance_check_then_load(satA, d, overflows);

        //URHO3D_LOGINFOF("%s: %i %i %i",
        //                satA->get_name().CString(), d.x_, d.y_, d.z_);
    }

    return;


    Vector3 focusPos;

    if (m_focus.NotNull())
    {
        //focusPosLong = m_focus->get_long_position();
        //focusPos = m_focus->get_active_node()->GetPosition();
    }
    else
    {
        //focusPos = GetSubsystem<Renderer>()->GetViewport(0)
        //        ->GetCamera()->GetNode()->GetWorldPosition();
    }

    const float originDistance = focusPos.Length();
    const float threshold = 30.0f;

    if (originDistance > threshold) {
        Vector3 offsetDist = focusPos * -1000.0f;
        offsetDist.x_ = Floor(offsetDist.x_);
        offsetDist.y_ = Floor(offsetDist.y_);
        offsetDist.z_ = Floor(offsetDist.z_);
        //relocate(m_localBody, m_localBodyPos - LongVector3(offsetDist.x_, offsetDist.y_, offsetDist.z_));

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

    //pw->SetGravity(Vector3::ZERO);

    // some gravity for temporary fun
    //float moon = 88200000;
    //Vector3 gravity;// = (m_terrain->GetNode()->GetPosition() - node_->GetChild("CameraCenter")->GetPosition());
    //float r = gravity.Length();
    //gravity = gravity / r;
    //gravity *= moon / (r * r);
    //printf("gravity: (%f, %f, %f)\n", gravity.x_, gravity.y_, gravity.z_);
    //pw->SetGravity(gravity);
}

void ActiveArea::distance_check_then_load(Satellite* sat,
                                          LongVector3& relative,
                                          IntVector3& overflowCount)
{
    // Check if already loaded
    // make sure overflow count is all zero
    // dont load self
    if (sat->get_active_node()
            || overflowCount.x_ || overflowCount.y_ || overflowCount.z_
            || sat == this)
    {
        return;
    }

    // TODO: Load preview should be here too somewhere

    // Get magnitude of relative. There's probably a better way to do this
    // the highest possible value for any of these is exactly the float
    // maximum of ~3.4*10^38
    double xsq = Pow(float(relative.x_), 2.0f);
    double ysq = Pow(float(relative.y_), 2.0f);
    double zsq = Pow(float(relative.z_), 2.0f);

    // No need to sqrt anything, just compare the squares instead
    // These are really damn big numbers

    double magSqrRel = xsq + ysq + zsq; // = Magnitude ^2
    double magSqrRad = Pow(float(sat->get_load_radius()) + float(m_loadRadius),
                           2.0f);

    //URHO3D_LOGINFOF("(%i, %i, %i) mag: %f",
    //                relative.x_, relative.y_, relative.z_, magSqrRel);

    // Load if within radius
    if (magSqrRad > magSqrRel)
    {
        URHO3D_LOGINFOF("ActiveArea Loading: %s", sat->get_name().CString());

        Vector3 floatPos(Vector3(relative.x_, relative.y_, relative.z_)
                       / m_unitsPerMeter);
        sat->load(this, floatPos);
    }
}

Node* ActiveArea::load(ActiveArea* area, const Vector3& pos)
{
    URHO3D_LOGINFOF("hey");
}

Node* ActiveArea::load_preview(ActiveArea *area)
{
    return nullptr;
}

void ActiveArea::unload()
{
    URHO3D_LOGINFOF("hey");
}
