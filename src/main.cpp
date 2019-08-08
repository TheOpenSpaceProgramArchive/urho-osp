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
#include <Urho3D/IO/FileSystem.h>
#include <Urho3D/IO/Log.h>
#include <Urho3D/Math/MathDefs.h>
#include <Urho3D/Physics/CollisionShape.h>
#include <Urho3D/Physics/PhysicsWorld.h>
#include <Urho3D/Physics/RigidBody.h>
#include <Urho3D/Resource/BackgroundLoader.h>
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
#include <Urho3D/ThirdParty/AngelScript/angelscript.h>

#include "ActiveArea.h"
#include "config.h"
#include "GLTFFile.h"
#include "Machine.h"
#include "MachineRocket.h"
#include "OspUniverse.h"
#include "PlanetTerrain.h"

using namespace Urho3D;
using namespace osp;

class OSPApplication : public Application {
public:
    int m_framecount;
    float m_time;

    SharedPtr<Scene> m_scene;
    SharedPtr<OspUniverse> m_osp;
    Vector<String> m_runImmediately;

    /**
     * This happens before the engine has been initialized
     * so it's usually minimal code setting defaults for
     * whatever instance variables you have.
     * You can also do this in the Setup method.
     */
    OSPApplication(Context * context) : Application(context), m_framecount(0),
                                        m_time(0)
    {
        ActiveArea::RegisterObject(context);
        //AstronomicalBody::RegisterObject(context);
        //Entity::RegisterObject(context);
        GLTFFile::RegisterObject(context);
        PlanetTerrain::RegisterObject(context);

        MachineRocket::RegisterObject(context);
        Machine::RegisterObject(context);
    }

    /**
     * A function used to get the OspUniverse instance.
     * Used only by Angelscript so far.
     * @return The OspUniverse
     */
    OspUniverse* GetOsp()
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

        // and the one that
        FileSystem* fileSystem = GetSubsystem<FileSystem>();

        // Use Default Urho3D UI style
        UIElement* root = GetSubsystem<UI>()->GetRoot();
        root->SetDefaultStyle(
                    cache->GetResource<XMLFile>("UI/DefaultStyle.xml"));

        // Create the loading screen image and add it to the UI root
        BorderImage* loading = new BorderImage(context_);
        loading->SetTexture(
                    cache->GetResource<Texture2D>("Textures/TempLoad.png"));
        loading->SetSize(1280, 720);
        root->AddChild(loading);

        // Initialize OSP system
        m_osp = new OspUniverse(context_);

        // Create empty scene
        m_scene = new Scene(context_);

        // Add a single viewport
        Renderer* renderer = GetSubsystem<Renderer>();
        renderer->SetHDRRendering(true);



        SharedPtr<Viewport> viewport(new Viewport(context_));
        viewport->SetScene(m_scene);
        renderer->SetViewport(0, viewport);

        // Get Angelscript working
        context_->RegisterSubsystem(new Script(context_));

        // Register "osp" as a global in AngelScript
        asIScriptEngine* scriptEngine =
                GetSubsystem<Script>()->GetScriptEngine();

        RegisterObject<OspUniverse>(scriptEngine, "OspUniverse");

        scriptEngine->RegisterObjectMethod("OspUniverse",
                "Scene@+ get_hiddenScene() const",
                asMETHOD(OspUniverse, get_hidden_scene), asCALL_THISCALL);

        scriptEngine->RegisterObjectMethod("OspUniverse",
                "void make_craft(Node@+) const",
                asMETHOD(OspUniverse, make_craft), asCALL_THISCALL);

        scriptEngine->RegisterObjectMethod("OspUniverse",
                "void debug_function(StringHash) const",
                asMETHOD(OspUniverse, debug_function), asCALL_THISCALL);

        // call GetOsp when osp is accessed from angelscript
        // See https://www.angelcode.com/angelscript/sdk/...
        //     docs/manual/doc_register_func.html
        scriptEngine->RegisterGlobalFunction("OspUniverse@+ get_osp()",
                                             asMETHOD(OSPApplication, GetOsp),
                                             asCALL_THISCALL_ASGLOBAL, this);


        // Path to OSPData directory
        String ospDir = cache->GetResourceDirs()[2];

        // List directories in OSPData
        Vector<String> subDirs;
        fileSystem->ScanDir(subDirs, ospDir, String("*"), SCAN_DIRS, false);

        // Remove invalid folders (there should be a better way to do this)
        {
            Vector<String>::Iterator pathIterator = subDirs.Begin();
            while (pathIterator != subDirs.End())
            {
                // Test to remove, put more conditions here
                if ((*pathIterator == ".") || (*pathIterator == "..")) {
                    pathIterator = subDirs.Erase(pathIterator);
                } else {
                    pathIterator ++;
                }
            }
        }

        // Call process_directory on every valid folder
        for(String name : subDirs)
        {
            m_osp->process_directory(ospDir + name);
        }

        // Don't grab the mouse
        GetSubsystem<Input>()->SetMouseGrabbed(false);
        GetSubsystem<Input>()->SetMouseVisible(true);

        // We subscribe to the events we'd like to handle.
        // Some of these are unused, and much more fresh from the example
        SubscribeToEvent(E_KEYDOWN,
                         URHO3D_HANDLER(OSPApplication, HandleKeyDown));
        SubscribeToEvent(E_UPDATE,
                         URHO3D_HANDLER(OSPApplication, HandleUpdate));
        SubscribeToEvent(E_RESOURCEBACKGROUNDLOADED,
                         URHO3D_HANDLER(OSPApplication, HandleResourceLoaded));
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
     * Urho3D Handler called each time a key is pressed
     * Most of this should be debug code, maybe open a console some day?
     * @param eventType
     * @param eventData
     */
    void HandleKeyDown(StringHash eventType, VariantMap& eventData) {

        using namespace KeyDown;

        int key = eventData[P_KEY].GetInt();

        // Exit with ESC
        if (key == KEY_ESCAPE)
        {
            engine_->Exit();
        }

        // Toggle planet material wireframe
        if (key == KEY_Q)
        {
            Material* m = GetSubsystem<ResourceCache>()
                            ->GetResource<Material>("Materials/Planet.xml");
            m->SetFillMode((m->GetFillMode() == FILL_WIREFRAME)
                                ? FILL_SOLID : FILL_WIREFRAME );
        }

        // Toggle mouse cursor when pressing tab
        if (key == KEY_TAB)
        {
            GetSubsystem<Input>()->SetMouseVisible(
                        !GetSubsystem<Input>()->IsMouseVisible());
            GetSubsystem<Input>()->SetMouseGrabbed(
                        !GetSubsystem<Input>()->IsMouseGrabbed());
        }
    }

    /**
     * Urho3D Handler called when the close button is pressed
     * @param eventType
     * @param eventData
     */
    void HandleClosePressed(StringHash eventType,VariantMap& eventData) {
        engine_->Exit();
    }

    /**
     * Urho3D Handler called every (physics) update
     * @param eventType
     * @param eventData
     */
    void HandleUpdate(StringHash eventType,VariantMap& eventData) {
        float timeStep = eventData[Update::P_TIMESTEP].GetFloat();
        m_framecount ++;
        m_time += timeStep;
    }

    /**
     * Urho3D Handler called each time a resource finishes loading
     * @param eventType
     * @param eventData
     */
    void HandleResourceLoaded(StringHash eventType, VariantMap& eventData) {

        if (eventData[ResourceBackgroundLoaded::P_SUCCESS].GetBool())
        {
            Resource* res = reinterpret_cast<Resource*>(
                        eventData[ResourceBackgroundLoaded::P_RESOURCE]
                            .GetVoidPtr());

            // Call the loaded() function of each script file that gets loaded
            // Remove this some day in place of a better system
            if (res->IsInstanceOf("ScriptFile"))
            {
                ScriptFile* script = reinterpret_cast<ScriptFile*>(res);
                asIScriptFunction* function =
                        script->GetFunction("void loaded()");
                //URHO3D_LOGINFOF("functopn: %p", function);
                if (function)
                {
                    VariantVector params;
                    params.Push(Variant(m_scene));
                    script->Execute(function, params);
                }
            }
            else if (res->IsInstanceOf("GLTFFile"))
            {
                GLTFFile* gltf = reinterpret_cast<GLTFFile*>(res);
                m_osp->register_parts(gltf);
            }
        } else {
            // error!
            URHO3D_LOGERRORF("Failed to load resource: %s",
                    eventData[ResourceBackgroundLoaded::P_RESOURCENAME]
                            .GetString().CString());
        }

    }

};


URHO3D_DEFINE_APPLICATION_MAIN(OSPApplication)
