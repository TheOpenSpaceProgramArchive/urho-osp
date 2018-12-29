#include <Urho3D/AngelScript/Script.h>
#include <Urho3D/AngelScript/ScriptFile.h>
#include <Urho3D/Graphics/Material.h>
#include <Urho3D/Graphics/StaticModel.h>
#include <Urho3D/Graphics/Technique.h>
#include <Urho3D/IO/FileSystem.h>
#include <Urho3D/Physics/CollisionShape.h>
#include <Urho3D/Physics/PhysicsWorld.h>
#include <Urho3D/Physics/RigidBody.h>
#include <Urho3D/Physics/CollisionShape.h>
#include <Urho3D/Resource/ResourceCache.h>

#include "ActiveArea.h"
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
    m_planet.initialize(context_, GetSubsystem<ResourceCache>()->GetResource<Image>("Textures/EquirectangularHeight.png"), body->get_radius());
    Material* m = GetSubsystem<ResourceCache>()->GetResource<Material>("Materials/Planet.xml");
    SetModel(m_planet.get_model());
    //m->SetCullMode(CULL_NONE);
    //m->SetFillMode(FILL_WIREFRAME);
    SetMaterial(m);
    SetCastShadows(false);
}

void PlanetTerrain::RegisterObject(Context* context)
{
    context->RegisterFactory<PlanetTerrain>("PlanetTerrain");
}

/**
 * @brief PlanetTerrain::UpdatePosition
 * @param activePosition [in] Center of the ActiveArea relative to this planet
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
    //GLTFFile* f = GetSubsystem<ResourceCache>()->GetResource<GLTFFile>("Gotzietek/TestFan/testfan.sturdy.gltf");
    //f->GetScene(0, category);

    // Make 8 rocket cubes
    for (int i = 12; i < 20; i ++)
    {
        // Make a new child in the dbg category
        Node* aPart = category->CreateChild("dbg_" + String(i));

        // Set some information about it
        aPart->SetVar("country", "Canarda");
        aPart->SetVar("description", "A simple oddly shaped cube");
        aPart->SetVar("manufacturer", "Gotzietec Industries");
        aPart->SetVar("name", "Cube "  + String(i));
        aPart->SetVar("massdry", Pow(0.05f * (i + 1), 3.0f));
        aPart->SetVar("prototype", aPart); // stored as WeakPtr

        // Tweakscale it based on i
        aPart->SetScale(0.05f * (i + 1));

        // Make sure it doesn't move when placed
        aPart->SetEnabled(false);

        // Give it a generic box model
        StaticModel* model = aPart->CreateComponent<StaticModel>();
        model->SetCastShadows(true);
        model->SetModel(GetSubsystem<ResourceCache>()->GetResource<Model>("Models/Box.mdl"));
        //model->SetModel(f->GetMeshs()[0]);

        Material* m = GetSubsystem<ResourceCache>()->GetResource<Material>("Materials/Floor0.json");
        //m->SetTexture(TU_DIFFUSE, GetSubsystem<ResourceCache>()->GetResource<Material>("Materials/Floor0.json")->GetTexture(TU_DIFFUSE));
        //m->SetTechnique(0, GetSubsystem<ResourceCache>()->GetResource<Technique>("Techniques/DiffUnlit.xml"));

        model->SetMaterial(m);

        // Give it physics, and set it's mass to size^3
        //RigidBody* rb = aPart->CreateComponent<RigidBody>();
        //rb->SetMass(Pow(0.05f * (i + 1), 3.0f));
        //rb->SetEnabled(false);

        // Give it collisions, default shape is already a unit cube
        // and is affected by scale
        CollisionShape* shape = aPart->CreateComponent<CollisionShape>();

        // Make the part into a functioning rocket
        MachineRocket* rocket = aPart->CreateComponent<MachineRocket>();
    }
}

/**
 * @brief Performs many other functions
 * @param which [in] Hash for which function to call
 */
void SystemOsp::debug_function(const StringHash which)
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
        area->relocate(ab, LongVector3(0, 4100 * 1000, 0));
        area->set_terrain(terrain);

    }

}

/**
 * @brief Adds an OSP resource directory
 * @param path [in] Absolute File Path to directory
 *
 * The directory contents will be scanned for parts and scripts.
 * Contents of a subdirectory named "Parts" will be scanned recursively for .gltf.sturdy files
 *
 */
void SystemOsp::process_directory(const String& path)
{
    ResourceCache* cache = GetSubsystem<ResourceCache>();
    FileSystem* fileSystem = GetSubsystem<FileSystem>();

    String dirName = path.Substring(path.FindLast('/') + 1, path.Length());
    URHO3D_LOGINFOF("Processing Directory: [%s] %s", dirName.CString(), path.CString());

    String pathParts = path + "/Parts";
    if (fileSystem->DirExists(pathParts))
    {
        // Start loading sturdy.gltf files
        URHO3D_LOGINFOF("Parts Directory Found: %s", pathParts.CString());
        Vector<String> gltfs;
        fileSystem->ScanDir(gltfs, pathParts, "*.sturdy.gltf", SCAN_FILES, true);
        for (String s : gltfs)
        {
            URHO3D_LOGINFOF("Part %s", (dirName + "/Parts/" + s).CString());
            cache->BackgroundLoadResource<GLTFFile>(dirName + "/Parts/" + s);
        }
    }

    String pathScripts = path + "/Scripts";
    if (fileSystem->DirExists(pathScripts))
    {
        // Start loading scripts
        URHO3D_LOGINFOF("Scripts Directory Found: %s", pathScripts.CString());
        Vector<String> scripts;
        fileSystem->ScanDir(scripts, pathScripts, "*.as", SCAN_FILES, true);
        for (String s : scripts)
        {
            URHO3D_LOGINFOF("Script %s", (dirName + "/Scripts/" + s).CString());
            cache->BackgroundLoadResource<ScriptFile>(dirName + "/Scripts/" + s);
        }
    }
}

/**
 * @brief Include a .gltf.sturdy's contents into the categories of parts
 * @param gltf [in] GLTF file to process
 */
void SystemOsp::register_parts(const GLTFFile* gltf)
{
    // Doesn't need to be an actual scene, just make it a node
    Node* gltfScene = m_hiddenScene->CreateChild("gltf_" + gltf->GetNameHash().ToString());

    URHO3D_LOGINFOF("Registering part: %s", gltf->GetName().CString());

    // Contents of Scene 0 of the GLTF will be parsed into gltfScene
    gltf->GetScene(0, gltfScene);

    // don't make this a reference, original will be modified
    const Vector<SharedPtr<Node>> children = gltfScene->GetChildren();

    for (SharedPtr<Node> part : children)
    {
        String partName = part->GetName();

        // Any node that is prefixed with part_ will be a part usable in game
        if (partName.StartsWith("part_"))
        {

            // This is a part, parse it
            const VariantMap& vars = part->GetVars();
            const JSONObject* extras = reinterpret_cast<JSONObject*>(vars["extras"]->GetVoidPtr());

            if (!extras)
                continue;

            part->SetVar("country", GLTFFile::StringValue((*extras)["country"]));
            part->SetVar("description", GLTFFile::StringValue((*extras)["description"]));
            part->SetVar("manufacturer", GLTFFile::StringValue((*extras)["manufacturer"]));
            part->SetVar("name", GLTFFile::StringValue((*extras)["name"]));
            part->SetVar("massdry", (*extras)["massdry"]->GetFloat());
            part->SetVar("prototype", part.Get()); // stored as WeakPtr

            URHO3D_LOGINFOF("Name: %s", GLTFFile::StringValue((*extras)["name"]).CString());
            //URHO3D_LOGINFOF("Description: %s", GLTFFile::StringValue((*extras)["description"]).CString());
            //URHO3D_LOGINFOF("Cost: %s", GLTFFile::StringValue((*extras)["cost"]).CString());

            part->SetName(partName.Substring(5));
            part->SetPosition(Vector3::ZERO);

            const Vector<SharedPtr<Node>>& partChildren = part->GetChildren();

            for (SharedPtr<Node> node : partChildren)
            {
                part_node_recurse(part, node);
            }

            m_parts->GetChild("dbg")->AddChild(part.Get());

        }
    }
}

void SystemOsp::part_node_recurse(Node* partRoot, Node* node)
{
    const VariantMap& vars = node->GetVars();
    //const JSONObject* extras = reinterpret_cast<JSONObject*>(vars["extras"]->GetVoidPtr());

    if (node->GetName().StartsWith("col_"))
    {
        const Vector3& pos = node->GetPosition();
        const Vector3& scale = node->GetScale();
        const Quaternion& rot = node->GetRotation();

        const String& shapeType = "";//GLTFFile::StringValue((*extras)["shape"]);

        CollisionShape* shape = node->GetParent()->CreateComponent<CollisionShape>();

        if (shapeType == "cylinder")
        {
            shape->SetCylinder(Min(scale.x_, scale.z_), scale.y_, pos, rot);
        }
        else
        {
            // Cube is default shape
            shape->SetBox(scale * 2.0f, pos, rot);
        }

        URHO3D_LOGINFOF("Shape made on: %s", node->GetParent()->GetName().CString());

        // don't loop through children
        return;
    }

    const Vector<SharedPtr<Node>>& children = node->GetChildren();

    for (SharedPtr<Node> node : children)
    {
        part_node_recurse(partRoot, node);
    }
}

/**
 * @brief SystemOsp::make_craft A function that adds Entity and some functionality to a node made of disabled parts. currently just adds Entity
 * @param node [in, out] Node to turn into a craft
 */
void SystemOsp::make_craft(Node* node)
{
    node->CreateComponent<Entity>();
    PODVector<Node*> printthese = node->GetChildrenWithComponent("StaticModel", true);

    //for (Node* n : printthese)
    //{
    //    Material* m = n->GetComponent<StaticModel>()->GetMaterial();
    //    URHO3D_LOGINFOF("Material Scene: %p", m);
    //    m->SetScene(GetSubsystem<Renderer>()->GetViewport(0)->GetScene());
    //}
}
