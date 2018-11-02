#include <Urho3D/Graphics/Renderer.h>
#include <Urho3D/Graphics/StaticModel.h>
#include <Urho3D/Physics/CollisionShape.h>
#include <Urho3D/Physics/PhysicsWorld.h>
#include <Urho3D/Physics/RigidBody.h>
#include <Urho3D/Physics/CollisionShape.h>
#include <Urho3D/Resource/ResourceCache.h>

#include "ActiveArea.h"
#include "GLTFFile.h"
#include "OSP.h"
#include "Machines.h"

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

void PlanetTerrain::initialize(AstronomicalBody* body)
{
    m_planet.initialize(context_, body->get_radius());
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

/**
 * @brief PlanetTerrain::UpdatePosition
 * @param activePosition Center of the ActiveArea relative to this planet
 */
void PlanetTerrain::UpdatePosition(const LongVector3& activePosition)
{
    // negative activePosition is where the planet should be relative to the urho origin

    Vector3 newPos(activePosition.x_, activePosition.y_, activePosition.z_);

    // negate and divide by 1000, since localBodyPos is in millimeters
    node_->SetPosition(newPos * -0.001f);

    //URHO3D_LOGINFOF("Updated position %s", node_->GetPosition().ToString().CString());

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

    // A test
    GLTFFile* f = GetSubsystem<ResourceCache>()->GetResource<GLTFFile>("Gotzietek/TestFan/testfan.sturdy.gltf");
    f->GetScene(0, category);

    // Make 20 rocket cubes
    for (int i = 0; i < 20; i ++)
    {
        // Make a new child in the dbg category
        Node* aPart = category->CreateChild("dbg_" + String(i));

        // Set some information about it
        aPart->SetVar("country", "Canarda");
        aPart->SetVar("description", "A simple oddly shaped cube");
        aPart->SetVar("manufacturer", "Gotzietec Industries");
        aPart->SetVar("name", "Cube "  + String(i));

        // Tweakscale it based on i
        aPart->SetScale(0.05f * (i + 1));

        // Make sure it doesn't move when placed
        aPart->SetEnabled(false);

        // Give it a generic box model
        StaticModel* model = aPart->CreateComponent<StaticModel>();
        model->SetCastShadows(true);
        model->SetModel(GetSubsystem<ResourceCache>()->GetResource<Model>("Models/Box.mdl"));
        //model->SetModel(f->GetMeshs()[0]);

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
        //component->Initialize();
    }
    else if (which == StringHash("create_universe"))
    {
        //PhysicsWorld* pw = scene->GetComponent<PhysicsWorld>();//scene->CreateComponent<PhysicsWorld>();
        //pw->SetGravity(Vector3::ZERO);


        // Create "solar system" of just 1 planet
        m_solarSystem = m_hiddenScene->CreateChild("SolarSystem");
        AstronomicalBody* ab = m_solarSystem->CreateComponent<AstronomicalBody>();

        // Create a planet with terrain
        Node* planet = scene->CreateChild("PlanetTerrain");
        PlanetTerrain* terrain = planet->CreateComponent<PlanetTerrain>();
        terrain->initialize(ab);

        RigidBody* rb = planet->CreateComponent<RigidBody>();
        CollisionShape* shape = planet->CreateComponent<CollisionShape>();
        shape->SetSphere(4000.0f * 2.0f);
        //terrain->GetPlanet()->update(Vector3(0, 4002, 0));
        //shape->SetTriangleMesh(terrain->GetPlanet()->get_model(), 0, Vector3::ONE);

        ActiveArea* area = scene->CreateComponent<ActiveArea>();
        area->relocate(ab, LongVector3(0, 4002 * 1000, 0));
        area->set_terrain(terrain);

    }

}
