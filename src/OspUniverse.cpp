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
#include "MachineRocket.h"
#include "NodeSat.h"
#include "PlanetTerrain.h"
#include "OspUniverse.h"

using namespace osp;

OspUniverse::OspUniverse(Context* context) : Object(context)
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

    // Make 4 different sized cubes
    for (int i = 2; i < 6; i ++)
    {
        // Make a new child in the dbg category
        Node* aPart = category->CreateChild("dbg_" + String(i));

        // Set some information about it
        aPart->SetVar("country", "Canarda");
        aPart->SetVar("description", "A simple oddly shaped cube");
        aPart->SetVar("manufacturer", "Gotzietec Industries");
        aPart->SetVar("name", "Cube "  + String(i));
        aPart->SetVar("massdry", Pow(float(i), 3.0f));
        aPart->SetVar("prototype", aPart); // stored as WeakPtr

        // Tweakscale it based on i
        aPart->SetScale(0.25f * i);

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
        //MachineRocket* rocket = aPart->CreateComponent<MachineRocket>();

        Vector3 dir(0.0f, 0.0f, 0.5f);

        for (int i = 0; i < 6; i ++)
        {
            // make the node
            Node* attachment = aPart->CreateChild("attach_" + String(i));
            attachment->LookAt(dir, Vector3::UP);
            attachment->SetPosition(dir);
            attachment->SetScale(Vector3(0.1, 0.1, 1));
            attachment->AddTag("Attachment");

            // Use shifting and negating to get normals for all 6 faces

            dir *= -1; // Negate direction
            // Shift components if positive
            if (dir.x_ + dir.y_ + dir.z_ > 0)
            {
                dir = Vector3(dir.z_, dir.x_, dir.y_);
            }
        }
    }
}

void OspUniverse::debug_function(const StringHash which)
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
        URHO3D_LOGINFOF("Creating universe...");

        // Destroy the universe first
        m_bigUniverse = nullptr;

        // AstronomicalBody is currently hard-coded to default to a very dense
        // astroid-sized ridiculously spherical 4km radius bumpy ball

        // Set the root of the universe to a default astronomical body
        m_bigUniverse = new AstronomicalBody();

        // Add some more AstronomicalBody

        AstronomicalBody* moonA = new AstronomicalBody();
        moonA->set_position(LongVector3(1024 * 16000, 0, 0));
        m_bigUniverse->add_child(moonA);

        AstronomicalBody* moonB = new AstronomicalBody();
        moonB->set_position(LongVector3(-1024 * 16000, 0, 0));
        m_bigUniverse->add_child(moonB);
        
        AstronomicalBody* moonBA = new AstronomicalBody();
        moonBA->set_position(LongVector3(0, 0, 1024 * 16000));
        moonB->add_child(moonBA);

        AstronomicalBody* moonBAA = new AstronomicalBody();
        moonBAA->set_position(LongVector3(0, 0, 1024 * 16000));
        moonBA->add_child(moonBAA);

        // Creata a NodeSat to represent the player's craft
        NodeSat* subjectSat = new NodeSat();
        subjectSat->set_node(scene->GetChild("Subject"));
        m_bigUniverse->add_child(subjectSat);

        // Link the Urho scene to the universe just created
        ActiveArea* area = scene->CreateComponent<ActiveArea>();
        m_bigUniverse->add_child(area);

        // Make the ActiveArea follow the player's craft arouynd
        area->set_focus(subjectSat);

        // Position both 150m above the surface
        area->set_position(LongVector3(0, 4150 * 1024, 0));;
        subjectSat->set_position(LongVector3(0, 4150 * 1024, 0));

        // Now we're all set!
        // ActiveArea will start loading everything next frame

        // Universe tree should look like this:
        // * m_bigUniverse
        //   * moonA
        //   * moonB
        //     * moonBA
        //       * moonBAA
        //   * subjectSat
        //   * area

        // test calculation for relative position
        //LongVector3 d;
        //d = Satellite::calculate_relative_position(moonA, moonB, 1024);
        //URHO3D_LOGINFOF("moonA->moonB: %i %i %i", d.x_, d.y_, d.z_);

        // Load them into the area
        // Usually these functions should be called by the ActiveArea itself
        // but any sort of distance checking auto loading isn't implemented yet
        //m_bigUniverse->load(area);
        //moonA->load(area);
        //moonB->load(area);
        //moonBA->load(area);
        //moonBAA->load(area);

    }

}

void OspUniverse::process_directory(const String& path)
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
        fileSystem->ScanDir(scripts, pathScripts, "*.as", SCAN_FILES, false);
        for (String s : scripts)
        {
            URHO3D_LOGINFOF("Script %s", (dirName + "/Scripts/" + s).CString());
            cache->BackgroundLoadResource<ScriptFile>(dirName + "/Scripts/" + s);
        }
    }
}

void OspUniverse::register_parts(const GLTFFile* gltf)
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

            // Start considering machines

            if (JSONValue* machines = (*extras)["machines"])
            {
                if (machines->IsArray())
                {
                    for (const JSONValue& machineValue : machines->GetArray())
                    {
                        if (!machineValue.IsObject())
                        {
                            continue;
                        }

                        const JSONObject& machineObject = machineValue.GetObject();

                        String type = GLTFFile::StringValue(machineObject["type"]);

                        SharedPtr<Machine> machineComponent = DynamicCast<Machine>(context_->CreateObject("Machine" + type));

                        if (machineComponent.NotNull())
                        {
                            // Machine exists and is a machine
                            part->AddComponent(machineComponent, 0, REPLICATED);
                            machineComponent->load_json(machineObject);
                            //URHO3D_LOGINFOF("Machine Type: %s %p %i", type.CString(), machineComponent, machineComponent.NotNull());
                        }
                        else
                        {
                            URHO3D_LOGERRORF("Unknown Machine type: %s", type.CString());
                        }
                    }
                }
            }

            part->SetName(partName.Substring(5));
            part->SetPosition(Vector3::ZERO);
            part->SetRotation(Quaternion::IDENTITY);

            const Vector<SharedPtr<Node>>& partChildren = part->GetChildren();

            for (SharedPtr<Node> node : partChildren)
            {
                part_node_recurse(part, node);
            }

            m_parts->GetChild("dbg")->AddChild(part.Get());
 
        }
    }
}

void OspUniverse::part_node_recurse(Node* partRoot, Node* node)
{
    const VariantMap& vars = node->GetVars();

    const JSONObject* extras = (vars["extras"]) ? reinterpret_cast<JSONObject*>(vars["extras"]->GetVoidPtr()) : nullptr;

    if (node->GetName().StartsWith("col_"))
    {

        // Parse a collider

        const Vector3& pos = node->GetPosition();
        const Vector3& scale = node->GetScale();
        const Quaternion& rot = node->GetRotation();

        const String& shapeType = extras ? GLTFFile::StringValue((*extras)["shape"]) : "";

        CollisionShape* shape = node->GetParent()->CreateComponent<CollisionShape>();

        if (shapeType == "cylinder")
        {
            shape->SetCylinder(Min(scale.x_, scale.z_) * 2.0f, scale.y_ * 2.0f, pos, rot);
        }
        else
        {
            // Cube is default shape
            shape->SetBox(scale * 2.0f, pos, rot);
        }

        //URHO3D_LOGINFOF("Shape type: %s", shapeType.CString());
        //URHO3D_LOGINFOF("Shape made on: %s", node->GetParent()->GetName().CString());

        // don't loop through children
        return;
    }
    else if (node->GetName().StartsWith("attach_"))
    {
        // Parse an attachment

        // this is seriously all that's needed

        node->AddTag("Attachment");
    }

    const Vector<SharedPtr<Node>>& children = node->GetChildren();

    for (SharedPtr<Node> node : children)
    {
        part_node_recurse(partRoot, node);
    }
}

void OspUniverse::make_craft(Node* node)
{
    //node->CreateComponent<Entity>();
    PODVector<Node*> printthese = node->GetChildrenWithComponent("StaticModel", true);

    //for (Node* n : printthese)
    //{
    //    Material* m = n->GetComponent<StaticModel>()->GetMaterial();
    //    URHO3D_LOGINFOF("Material Scene: %p", m);
    //    m->SetScene(GetSubsystem<Renderer>()->GetViewport(0)->GetScene());
    //}
}
