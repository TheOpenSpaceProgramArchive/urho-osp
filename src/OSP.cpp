#include <Urho3D/Graphics/Renderer.h>
#include <Urho3D/Graphics/StaticModel.h>
#include <Urho3D/Physics/CollisionShape.h>
#include <Urho3D/Physics/PhysicsWorld.h>
#include <Urho3D/Physics/RigidBody.h>
#include <Urho3D/Resource/ResourceCache.h>


#include "GLTFFile.h"
#include "OSP.h"
#include "Machines.h"

using namespace osp;

AstronomicalBody::AstronomicalBody(Context* context) : Satellite(context)
{
    SetUpdateEventMask(USE_FIXEDUPDATE);

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

void AstronomicalBody::Initialize(Context* context) {
    ResourceCache* cache = GetSubsystem<ResourceCache>();
    //planet_.initialize(context, size, GetScene(), cache);
    //node_->SetPosition(Vector3(0, 4, 10));
    //node_->SetScale(Vector3(size / 2, size / 2, size / 2));
    //collider_ = node_->CreateComponent<RigidBody>();
    //CollisionShape* sphere = node_->CreateComponent<CollisionShape>();
    //sphere->SetSphere(size / 2);
    //sphere->SetSphere(size * 2);
    //collider_->SetMass(1.0f);
    //collider_->SetFriction(0.75f);
    //StaticModel* planetModel = node_->CreateComponent<StaticModel>();
    //m_terrain = node_->CreateComponent<PlanetTerrain>();
    //m_p->SetModel(planet_.get_model());
    //Material* m = cache->GetResource<Material>("Materials/DefaultGrey.xml");
    //m->SetCullMode(CULL_CW);
    //m->SetFillMode(FILL_WIREFRAME);
    //planetModel->SetMaterial(m);
}



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

PlanetTerrain::PlanetTerrain(Context* context) : StaticModel(context), m_first(false)
{
    //SetUpdateEventMask(USE_UPDATE);
    //SubscribeToEvent(E_UPDATE, URHO3D_HANDLER(PlanetTerrain, UpdatePlanet));
}

void PlanetTerrain::Initialize()
{
    m_planet.initialize(context_, 1.0f);
    Material* m = GetSubsystem<ResourceCache>()->GetResource<Material>("Materials/DefaultGrey.xml");
    SetModel(m_planet.get_model());
    m->SetCullMode(CULL_NONE);
    m->SetFillMode(FILL_WIREFRAME);
    SetMaterial(m);
}

void PlanetTerrain::RegisterObject(Context* context)
{
    context->RegisterFactory<PlanetTerrain>("PlanetTerrain");
}

SystemOsp::SystemOsp(Context* context) : Object(context)
{
    // Create the hidden scene, accesible through angelscript with osp.hiddenScene
    m_hiddenScene = new Scene(context);

    // Make sure no updates happen in this scene
    m_hiddenScene->SetUpdateEnabled(false);
    m_hiddenScene->SetEnabled(false);

    // Make a node that holds parts
    m_parts = m_hiddenScene->CreateChild("Parts");

    // Create a category of parts, dbg
    Node* category = m_parts->CreateChild("dbg");
    category->SetVar("DisplayName", "Debug Parts");

    // Make 20 rocket cubes
    for (int i = 0; i < 20; i ++)
    {
        // Make a new child in the dbg category
        Node* aPart = category->CreateChild("dbg_" + String(i));

        // Set some information about it
        aPart->SetVar("Country", "Canarda");
        aPart->SetVar("Description", "A simple oddly shaped cube");
        aPart->SetVar("Manufacturer", "Gotzietec Industries");
        aPart->SetVar("DisplayName", "Cube "  + String(i));

        // Tweakscale it based on i
        aPart->SetScale(0.05f * (i + 1));

        // Make sure it doesn't move when placed
        aPart->SetEnabled(false);

        // Give it a generic box model
        StaticModel* model = aPart->CreateComponent<StaticModel>();
        model->SetCastShadows(true);
        //model->SetModel(GetSubsystem<ResourceCache>()->GetResource<Model>("Models/Box.mdl"));
        // A test
        GLTFFile* f = GetSubsystem<ResourceCache>()->GetResource<GLTFFile>("Gotzietek/TestFan/testfan.sturdy.gltf");
        model->SetModel(f->GetMeshs()[0]);

        model->SetMaterial(GetSubsystem<ResourceCache>()->GetResource<Material>("Materials/Floor0.json"));

        // Give it physics, and set it's mass to size^3
        RigidBody* rb = aPart->CreateComponent<RigidBody>();
        rb->SetMass(Pow(0.05f * (i + 1), 3.0f));
        rb->SetEnabled(false);

        // Give it collisions, default shape is already a unit cube
        // and is affected by scale
        CollisionShape* shape = aPart->CreateComponent<CollisionShape>();

        // Make the part into a functioning rocket
        MachineRocket* rocket = aPart->CreateComponent<MachineRocket>();
    }
}

/**
 * @brief SystemOsp::make_craft A function that adds Entity and some functionality to a node made of disabled parts. currently just adds Entity
 * @param node Node to turn into a craft
 */
void SystemOsp::make_craft(Node* node)
{
    node->CreateComponent<Entity>();

}

/**
 * @brief SystemOsp::debug_function Intended to be called from angelscript to perform various functions in c++
 * @param which
 */
void SystemOsp::debug_function(StringHash which)
{
    // Get scene
    Scene* scene = GetSubsystem<Renderer>()->GetViewport(0)->GetScene();

    if (which == StringHash("make_planet"))
    {

        // Make planet
        //Node* planet = node->GetScene()->CreateChild("Planet");
        //planet->CreateComponent<AstronomicalBody>();

        // Make terrain
        Node* terrain = scene->CreateChild("PlanetTerrain");
        PlanetTerrain* component = terrain->CreateComponent<PlanetTerrain>();
        terrain->SetPosition(Vector3(0, 0, 0));
        component->Initialize();
    }
    else if (which == StringHash("create_universe"))
    {
        PhysicsWorld* pw = scene->CreateComponent<PhysicsWorld>();

    }

}
