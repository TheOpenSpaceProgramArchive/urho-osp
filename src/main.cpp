#include <string>
#include <sstream>
#include <iostream>

#include <Urho3D/AngelScript/Script.h>
#include <Urho3D/AngelScript/ScriptFile.h>
#include <Urho3D/Container/Vector.h>
#include <Urho3D/Core/CoreEvents.h>
#include <Urho3D/Core/WorkQueue.h>
#include <Urho3D/Engine/Application.h>
#include <Urho3D/Engine/Engine.h>
#include <Urho3D/Graphics/Camera.h>
#include <Urho3D/Graphics/DebugRenderer.h>
#include <Urho3D/Graphics/Geometry.h>
#include <Urho3D/Graphics/Graphics.h>
#include <Urho3D/Graphics/IndexBuffer.h>
#include <Urho3D/Graphics/Light.h>
#include <Urho3D/Graphics/Material.h>
#include <Urho3D/Graphics/Model.h>
#include <Urho3D/Graphics/Octree.h>
#include <Urho3D/Graphics/Renderer.h>
#include <Urho3D/Graphics/Skybox.h>
#include <Urho3D/Graphics/StaticModel.h>
#include <Urho3D/Graphics/VertexBuffer.h>
#include <Urho3D/Graphics/Texture2D.h>
#include <Urho3D/Input/InputEvents.h>
#include <Urho3D/Input/Input.h>
#include <Urho3D/IO/Log.h>
#include <Urho3D/Math/MathDefs.h>
#include <Urho3D/Physics/CollisionShape.h>
#include <Urho3D/Physics/PhysicsWorld.h>
#include <Urho3D/Physics/RigidBody.h>
#include "Urho3D/Resource/BackgroundLoader.h"
#include <Urho3D/Resource/ResourceCache.h>
#include <Urho3D/Resource/ResourceEvents.h>
#include <Urho3D/Resource/XMLFile.h>
#include <Urho3D/Scene/SceneEvents.h>
#include <Urho3D/Scene/Scene.h>
#include <Urho3D/UI/Button.h>
#include <Urho3D/UI/BorderImage.h>
#include <Urho3D/UI/Font.h>
#include <Urho3D/UI/Text.h>
#include <Urho3D/UI/Window.h>
#include <Urho3D/UI/UIElement.h>
#include <Urho3D/UI/UIEvents.h>
#include <Urho3D/UI/UI.h>
#include <Urho3D/AngelScript/APITemplates.h>


#include "OSP.h"
#include "Machines.h"
#include "config.h"

using namespace Urho3D;
using namespace osp;

class OSPApplication : public Application {
public:
    int m_framecount;
    float m_time;

    SharedPtr<Scene> m_scene;
    SharedPtr<SystemOsp> m_osp;
    //SharedPtr<Node> m_cameraNode;
    //Vector<ScriptFile*> m_runImmediately;
    Vector<String> m_runImmediately;

    /**
    * This happens before the engine has been initialized
    * so it's usually minimal code setting defaults for
    * whatever instance variables you have.
    * You can also do this in the Setup method.
    */
    OSPApplication(Context * context) : Application(context), m_framecount(0), m_time(0)
    {
        AstronomicalBody::RegisterObject(context);
        Entity::RegisterObject(context);
        PlanetTerrain::RegisterObject(context);
        MachineRocket::RegisterObject(context);
    }

    SystemOsp* GetOsp()
    {
        return m_osp.Get();
    }

    /**
    * This method is called before the engine has been initialized.
    * Thusly, we can setup the engine parameters before anything else
    * of engine importance happens (such as windows, search paths,
    * resolution and other things that might be user configurable).
    */
    virtual void Setup()
    {
        engineParameters_["FullScreen"] = false;
        engineParameters_["WindowWidth"] = 1280;
        engineParameters_["WindowHeight"] = 720;
        engineParameters_["WindowResizable"] = true;
        engineParameters_["WindowTitle"] = "OpenSpaceProgram Urho3D";
        engineParameters_["ResourcePaths"] = "Data;CoreData;OSPData";
    }

    virtual void Start() {
        // Get the subsystem that is used to load resources
        ResourceCache* cache = GetSubsystem<ResourceCache>();

        // Use Default Urho3D style
        UIElement* root = GetSubsystem<UI>()->GetRoot();
        root->SetDefaultStyle(cache->GetResource<XMLFile>("UI/DefaultStyle.xml"));

        // Create the loading screen image and add it to the UI root
        BorderImage* loading = new BorderImage(context_);
        loading->SetTexture(cache->GetResource<Texture2D>("Textures/TempLoad.png"));
        loading->SetSize(1280, 720);
        root->AddChild(loading);

        // Initialize OSP system
        m_osp = new SystemOsp(context_);

        // Get Angelscript working
        context_->RegisterSubsystem(new Script(context_));

        // Register "osp" as a global in AngelScript
        asIScriptEngine* scriptEngine = GetSubsystem<Script>()->GetScriptEngine();
        // Copied from Urho3D/AngelScript/*API.cpp
        RegisterObject<SystemOsp>(scriptEngine, "SystemOsp");
        scriptEngine->RegisterObjectMethod("SystemOsp", "Scene@+ get_hiddenScene() const", asMETHOD(SystemOsp, get_hidden_scene), asCALL_THISCALL);
        scriptEngine->RegisterObjectMethod("SystemOsp", "void make_craft(Node@+) const", asMETHOD(SystemOsp, make_craft), asCALL_THISCALL);
        scriptEngine->RegisterObjectMethod("SystemOsp", "void debug_function(StringHash) const", asMETHOD(SystemOsp, debug_function), asCALL_THISCALL);
        // call GetOsp when osp is accessed from angelscript, see thing about singleton https://www.angelcode.com/angelscript/sdk/docs/manual/doc_register_func.html
        scriptEngine->RegisterGlobalFunction("SystemOsp@+ get_osp()", asMETHOD(OSPApplication, GetOsp), asCALL_THISCALL_ASGLOBAL, this);

        // Run main menu script once it's loaded
        m_runImmediately.Push("Scripts/MainMenu.as");
        cache->BackgroundLoadResource<ScriptFile>("Scripts/MainMenu.as", true);

        //m_scene->CreateComponent<Octree>();
        //m_scene->CreateComponent<DebugRenderer>();
        //PhysicsWorld* world = m_scene->CreateComponent<PhysicsWorld>();
        //world->SetGravity(Vector3::ZERO);

        // Let's put some sky in there.
        //Node* skyNode=m_scene->CreateChild("Sky");
        //skyNode->SetScale(500.0f); // The scale actually does not matter
        //Skybox* skybox = skyNode->CreateComponent<Skybox>();
        //skybox->SetModel(cache->GetResource<Model>("Models/Box.mdl"));
        //skybox->SetMaterial(cache->GetResource<Material>("Materials/Skybox.xml"));

        // Create empty scene
        m_scene = new Scene(context_);

        // Add camera node and component
        //m_cameraNode = m_scene->CreateChild("Camera");
        //m_cameraNode->CreateComponent<Camera>();
        //m_cameraNode->SetPosition(Vector3(0, 0, 0));

        // Add a single viewport
        Renderer* renderer = GetSubsystem<Renderer>();
        SharedPtr<Viewport> viewport(new Viewport(context_));
        viewport->SetScene(m_scene);
        renderer->SetViewport(0, viewport);

        // Don't grab the mouse
        GetSubsystem<Input>()->SetMouseGrabbed(false);
        GetSubsystem<Input>()->SetMouseVisible(true);

        // We subscribe to the events we'd like to handle.
        // Some of these are unused, and much more fresh from the example
        SubscribeToEvent(E_BEGINFRAME, URHO3D_HANDLER(OSPApplication, HandleBeginFrame));
        SubscribeToEvent(E_KEYDOWN, URHO3D_HANDLER(OSPApplication, HandleKeyDown));
        SubscribeToEvent(E_UPDATE, URHO3D_HANDLER(OSPApplication, HandleUpdate));
        SubscribeToEvent(E_POSTUPDATE, URHO3D_HANDLER(OSPApplication, HandlePostUpdate));
        SubscribeToEvent(E_RENDERUPDATE, URHO3D_HANDLER(OSPApplication, HandleRenderUpdate));
        SubscribeToEvent(E_POSTRENDERUPDATE, URHO3D_HANDLER(OSPApplication, HandlePostRenderUpdate));
        SubscribeToEvent(E_ENDFRAME, URHO3D_HANDLER(OSPApplication, HandleEndFrame));
        SubscribeToEvent(E_RESOURCEBACKGROUNDLOADED, URHO3D_HANDLER(OSPApplication, HandleResourceLoaded));
    }

    /**
    * Good place to get rid of any system resources that requires the
    * engine still initialized. You could do the rest in the destructor,
    * but there's no need, this method will get called when the engine stops,
    * for whatever reason (short of a segfault).
    */
    virtual void Stop() {
    }

    /**
    * Every frame's life must begin somewhere. Here it is.
    */
    void HandleBeginFrame(StringHash eventType,VariantMap& eventData) {
        // We really don't have anything useful to do here for this example.
        // Probably shouldn't be subscribing to events we don't care about.
    }

    /**
    * Input from keyboard is handled here. I'm assuming that Input, if
    * available, will be handled before E_UPDATE.
    */
    void HandleKeyDown(StringHash eventType, VariantMap& eventData) {
        using namespace KeyDown;
        int key=eventData[P_KEY].GetInt();
        if(key==KEY_ESCAPE)
            engine_->Exit();

        if(key==KEY_Q) {
            Material* m = GetSubsystem<ResourceCache>()->GetResource<Material>("Materials/DefaultGrey.xml");
            m->SetFillMode((m->GetFillMode() == FILL_WIREFRAME) ? FILL_SOLID : FILL_WIREFRAME );
        }

        if(key==KEY_TAB) {
            // toggle mouse cursor when pressing tab
            GetSubsystem<Input>()->SetMouseVisible(!GetSubsystem<Input>()->IsMouseVisible());
            GetSubsystem<Input>()->SetMouseGrabbed(!GetSubsystem<Input>()->IsMouseGrabbed());
        }
    }

    /**
    * You can get these events from when ever the user interacts with the UI.
    */
    void HandleClosePressed(StringHash eventType,VariantMap& eventData) {
        engine_->Exit();
    }
    /**
    * Your non-rendering logic should be handled here.
    * This could be moving objects, checking collisions and reaction, etc.
    */
    void HandleUpdate(StringHash eventType,VariantMap& eventData) {
        float timeStep = eventData[Update::P_TIMESTEP].GetFloat();
        m_framecount ++;
        m_time += timeStep;

        Scene* scene= m_scene;

        if (GetSubsystem<Renderer>()->GetNumViewports() != 0)
        {
            if (GetSubsystem<Renderer>()->GetViewport(0)->GetCamera() != NULL)
            {
                        scene = GetSubsystem<Renderer>()->GetViewport(0)->GetCamera()->GetScene();
            }
        }

        // Quick and easy floating point origin
        /*Vector3 translateEverything(m_cameraNode->GetPosition());
        translateEverything.x_ = Floor(translateEverything.x_ / 64) * 64;
        translateEverything.y_ = Floor(translateEverything.y_ / 64) * 64;
        translateEverything.z_ = Floor(translateEverything.z_ / 64) * 64;
        if (translateEverything != Vector3::ZERO) {
            printf("saaaaa %.2f %.2f %.2f\n", translateEverything.x_, translateEverything.y_, translateEverything.z_);
            const Vector<SharedPtr<Node>> e = m_scene->GetChildren();
            for (uint i = 0; i < e.Size(); i ++) {
                e[i]->Translate(-translateEverything, TS_WORLD);
            }
          
        }*/

        auto planets = scene->GetChildrenWithComponent("PlanetTerrain");
        if (planets.Size() != 0)
        {
            planets[0]->GetComponent<PlanetTerrain>()->GetPlanet()->update(GetSubsystem<Renderer>()->GetViewport(0)->GetCamera()->GetNode()->GetPosition());
        }

        if (m_time >= 0.2) {

            std::string str;
            str.append("OpenSpaceProgram " GIT_BRANCH "-" GIT_COMMIT_HASH ", too early\n"
                       "Tab: Toggle mouse, WASD: Move, Shift: Fast mode, Q: Toggle Wireframe\n"
                       "T: toggle planet update, R/F: LoD up/down, P: Show truth, Esc: quit\n"
                       "--------------------------------------------------------------------\n");
            {
                std::ostringstream ss;
                ss<<(float)m_framecount/m_time;
                std::string s(ss.str());
                str.append("Frame R8    : ");
                str.append(s.substr(0,6));
                str.append("\n");
            }
            String s(str.c_str(),str.size());
            //text_->SetText(s);
            ///URHO3D_LOGINFO(s);     // this show how to put stuff into the log
            m_framecount = 0;
            m_time = 0;
        }

        Input* input = GetSubsystem<Input>();

    }
    /**
    * Anything in the non-rendering logic that requires a second pass,
    * it might be well suited to be handled here.
    */
    void HandlePostUpdate(StringHash eventType,VariantMap& eventData) {
        // We really don't have anything useful to do here for this example.
        // Probably shouldn't be subscribing to events we don't care about.
    }
    /**
    * If you have any details you want to change before the viewport is
    * rendered, try putting it here.
    * See http://urho3d.github.io/documentation/1.32/_rendering.html
    * for details on how the rendering pipeline is setup.
    */
    void HandleRenderUpdate(StringHash eventType, VariantMap & eventData) {
        // We really don't have anything useful to do here for this example.
        // Probably shouldn't be subscribing to events we don't care about.
    }
    /**
    * After everything is rendered, there might still be things you wish
    * to add to the rendering. At this point you cannot modify the scene,
    * only post rendering is allowed. Good for adding things like debug
    * artifacts on screen or brush up lighting, etc.
    */
    void HandlePostRenderUpdate(StringHash eventType, VariantMap & eventData) {
        // We could draw some debuggy looking thing for the octree.
        // m_scene->GetComponent<Octree>()->DrawDebugGeometry(true);
    }
    /**
    * All good things must come to an end.
    */
    void HandleEndFrame(StringHash eventType,VariantMap& eventData) {
        // We really don't have anything useful to do here for this example.
        // Probably shouldn't be subscribing to events we don't care about.
    }

    void HandleResourceLoaded(StringHash eventType, VariantMap& eventData) {

        if (eventData[ResourceBackgroundLoaded::P_SUCCESS].GetBool())
        {
            printf("Succ: %s %s\n", eventData[ResourceBackgroundLoaded::P_RESOURCENAME].GetString().CString(), m_runImmediately[0].CString());

            // Check if the the resource is in the run run immediately list
            unsigned i = m_runImmediately.IndexOf(eventData[ResourceBackgroundLoaded::P_RESOURCENAME].GetString());
            if (i != -1)
            {
                // Run the thing, probably a script
                m_runImmediately.Erase(i);
                ScriptFile* script = reinterpret_cast<ScriptFile*>(eventData[ResourceBackgroundLoaded::P_RESOURCE].GetVoidPtr());
                VariantVector params;
                params.Push(Variant(m_scene));
                script->Execute("void main(Scene&)", params);
            }


        } else {
            // error!
            URHO3D_LOGERROR("Loading something went wrong");
        }
    }

};


URHO3D_DEFINE_APPLICATION_MAIN(OSPApplication)
