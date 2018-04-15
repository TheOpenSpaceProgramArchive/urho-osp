#include <string>
#include <sstream>
#include <iostream>

#include <Urho3D/Core/CoreEvents.h>
#include <Urho3D/Engine/Application.h>
#include <Urho3D/Engine/Engine.h>
#include <Urho3D/Input/Input.h>
#include <Urho3D/Input/InputEvents.h>
#include <Urho3D/Resource/ResourceCache.h>
#include <Urho3D/Resource/XMLFile.h>
#include <Urho3D/IO/Log.h>
#include <Urho3D/UI/UI.h>
#include <Urho3D/UI/Text.h>
#include <Urho3D/UI/Font.h>
#include <Urho3D/UI/Button.h>
#include <Urho3D/UI/UIEvents.h>
#include <Urho3D/Scene/Scene.h>
#include <Urho3D/Scene/SceneEvents.h>
#include <Urho3D/Graphics/Graphics.h>
#include <Urho3D/Graphics/Camera.h>
#include <Urho3D/Graphics/Geometry.h>
#include <Urho3D/Graphics/IndexBuffer.h>
#include <Urho3D/Graphics/Renderer.h>
#include <Urho3D/Graphics/DebugRenderer.h>
#include <Urho3D/Graphics/Octree.h>
#include <Urho3D/Graphics/Light.h>
#include <Urho3D/Graphics/Model.h>
#include <Urho3D/Graphics/StaticModel.h>
#include <Urho3D/Graphics/Material.h>
#include <Urho3D/Graphics/Skybox.h>
#include <Urho3D/Graphics/VertexBuffer.h>
#include <Urho3D/Math/MathDefs.h>

#include "PlanWren.h"
#include "config.h"

using namespace Urho3D;

/**
* Using the convenient Application API we don't have
* to worry about initializing the engine or writing a main.
* You can probably mess around with initializing the engine
* and running a main manually, but this is convenient and portable.
*/
class MyApp : public Application {
public:
    int framecount_;
    float time_;
    SharedPtr<Text> text_;
    SharedPtr<Scene> scene_;
    SharedPtr<Node> boxNode_;
    SharedPtr<Node> cameraNode_;
    PlanWren* planet_; // maybe make this work some time
    bool updatePlanet_ = true;

    /**
    * This happens before the engine has been initialized
    * so it's usually minimal code setting defaults for
    * whatever instance variables you have.
    * You can also do this in the Setup method.
    */
    MyApp(Context * context) : Application(context),framecount_(0),time_(0) {
    }

    /**
    * This method is called before the engine has been initialized.
    * Thusly, we can setup the engine parameters before anything else
    * of engine importance happens (such as windows, search paths,
    * resolution and other things that might be user configurable).
    */
    virtual void Setup() {
        // These parameters should be self-explanatory.
        // See http://urho3d.github.io/documentation/1.5/_main_loop.html
        // for a more complete list.
        engineParameters_["FullScreen"]=false;
        engineParameters_["WindowWidth"]=1280;
        engineParameters_["WindowHeight"]=720;
        engineParameters_["WindowResizable"]=true;
    }

    /**
    * This method is called after the engine has been initialized.
    * This is where you set up your actual content, such as scenes,
    * models, controls and what not. Basically, anything that needs
    * the engine initialized and ready goes in here.
    */
    virtual void Start() {
        // We will be needing to load resources.
        // All the resources used in this example comes with Urho3D.
        // If the engine can't find them, check the ResourcePrefixPath (see http://urho3d.github.io/documentation/1.5/_main_loop.html).
        ResourceCache* cache=GetSubsystem<ResourceCache>();

        // Let's use the default style that comes with Urho3D.
        GetSubsystem<UI>()->GetRoot()->SetDefaultStyle(cache->GetResource<XMLFile>("UI/DefaultStyle.xml"));
        // Let's create some text to display.
        text_=new Text(context_);
        // Text will be updated later in the E_UPDATE handler. Keep readin'.
        text_->SetText("....");
        // If the engine cannot find the font, it comes with Urho3D.
        // Set the environment variables URHO3D_HOME, URHO3D_PREFIX_PATH or
        // change the engine parameter "ResourcePrefixPath" in the Setup method.
        text_->SetFont(cache->GetResource<Font>("Fonts/Anonymous Pro.ttf"), 12);
        text_->SetColor(Color(0,0,.3));
        text_->SetHorizontalAlignment(HA_LEFT);
        text_->SetVerticalAlignment(VA_TOP);
        text_->SetPosition(4,0);
        GetSubsystem<UI>()->GetRoot()->AddChild(text_);
        // Add a button, just as an interactive UI sample.
        //Button* button=new Button(context_);
        // Note, must be part of the UI system before SetSize calls!
        //GetSubsystem<UI>()->GetRoot()->AddChild(button);
        //button->SetName("Button Quit");
        //button->SetStyle("Button");
        //button->SetSize(32,32);
        //button->SetPosition(16,116);
        // Subscribe to button release (following a 'press') events
        //SubscribeToEvent(button,E_RELEASED,URHO3D_HANDLER(MyApp,HandleClosePressed));

        // Let's setup a scene to render.
        scene_=new Scene(context_);
        // Let the scene have an Octree component!
        scene_->CreateComponent<Octree>();
        // Let's add an additional scene component for fun.
        scene_->CreateComponent<DebugRenderer>();

        // Let's put some sky in there.
        // Again, if the engine can't find these resources you need to check
        // the "ResourcePrefixPath". These files come with Urho3D.
        Node* skyNode=scene_->CreateChild("Sky");
        skyNode->SetScale(500.0f); // The scale actually does not matter
        Skybox* skybox=skyNode->CreateComponent<Skybox>();
        skybox->SetModel(cache->GetResource<Model>("Models/Box.mdl"));
        skybox->SetMaterial(cache->GetResource<Material>("Materials/Skybox.xml"));

        // Let's put a box in there.
        boxNode_=scene_->CreateChild("Box");
        boxNode_->SetPosition(Vector3(0,2,15));
        boxNode_->SetScale(Vector3(3,3,3));
        StaticModel* boxObject=boxNode_->CreateComponent<StaticModel>();
        boxObject->SetModel(cache->GetResource<Model>("Models/Box.mdl"));
        boxObject->SetMaterial(cache->GetResource<Material>("Materials/Stone.xml"));
        boxObject->SetCastShadows(true);

        // Create 400 boxes in a grid.
        /*for(int x=-30;x<30;x+=3)
            for(int z=0;z<60;z+=3)
            {
                Node* boxNode_=scene_->CreateChild("Box");
                boxNode_->SetPosition(Vector3(x,-3,z));
                boxNode_->SetScale(Vector3(2,2,2));
                StaticModel* boxObject=boxNode_->CreateComponent<StaticModel>();
                boxObject->SetModel(cache->GetResource<Model>("Models/Box.mdl"));
                boxObject->SetMaterial(cache->GetResource<Material>("Materials/Stone.xml"));
                boxObject->SetCastShadows(true);
            }*/

        planet_ = new PlanWren();
        planet_->Initialize(context_, 60.0f, scene_, cache);

        //sindBuf_ = SharedPtr<IndexBuffer>(indBuf);
        //vertData.Reserve(12 * 3);

        // Vertex Buffer:
        // [12 fundementals, (x) shared lines, (x)*20 face indicies]

        Node* boxNode_=scene_->CreateChild("Box");
        //boxNode_->SetPosition(Vector3(0,-60,0));
        boxNode_->SetScale(Vector3(1.0f,1.0f,1.0f));
        StaticModel* boxObjectB=boxNode_->CreateComponent<StaticModel>();
        boxObjectB->SetModel(planet_->GetModel());
        Material* m = cache->GetResource<Material>("Materials/Stone.xml");
        m->SetFillMode(FILL_WIREFRAME);
        boxObjectB->SetMaterial(m);
        boxObjectB->SetCastShadows(true);

        // We need a camera from which the viewport can render.
        cameraNode_=scene_->CreateChild("Camera");
        Camera* camera=cameraNode_->CreateComponent<Camera>();
        camera->SetFarClip(2000);

        // Create a red directional light (sun)
        {
            Node* lightNode=scene_->CreateChild();
            lightNode->SetDirection(Vector3::FORWARD);
            lightNode->Yaw(50);     // horizontal
            lightNode->Pitch(10);   // vertical
            Light* light=lightNode->CreateComponent<Light>();
            light->SetLightType(LIGHT_DIRECTIONAL);
            light->SetBrightness(1.6);
            light->SetColor(Color(1.0,.6,0.3,1));
            light->SetCastShadows(true);
        }
        // Create a blue point light
        {
            Node* lightNode=scene_->CreateChild("Light");
            lightNode->SetPosition(Vector3(-10,2,5));
            Light* light=lightNode->CreateComponent<Light>();
            light->SetLightType(LIGHT_POINT);
            light->SetRange(25);
            light->SetBrightness(1.7);
            light->SetColor(Color(0.5,.5,1.0,1));
            light->SetCastShadows(true);
        }
        // add a green spot light to the camera node
        {
            Node* node_light=cameraNode_->CreateChild();
            Light* light=node_light->CreateComponent<Light>();
            node_light->Pitch(15);  // point slightly downwards
            light->SetLightType(LIGHT_SPOT);
            light->SetRange(20);
            light->SetColor(Color(.6,1,.6,1.0));
            light->SetBrightness(2.8);
            light->SetFov(25);
        }

        // Now we setup the viewport. Of course, you can have more than one!
        Renderer* renderer=GetSubsystem<Renderer>();
        SharedPtr<Viewport> viewport(new Viewport(context_,scene_,cameraNode_->GetComponent<Camera>()));
        renderer->SetViewport(0,viewport);

        // We subscribe to the events we'd like to handle.
        // In this example we will be showing what most of them do,
        // but in reality you would only subscribe to the events
        // you really need to handle.
        // These are sort of subscribed in the order in which the engine
        // would send the events. Read each handler method's comment for
        // details.
        SubscribeToEvent(E_BEGINFRAME,URHO3D_HANDLER(MyApp,HandleBeginFrame));
        SubscribeToEvent(E_KEYDOWN,URHO3D_HANDLER(MyApp,HandleKeyDown));
        SubscribeToEvent(E_UPDATE,URHO3D_HANDLER(MyApp,HandleUpdate));
        SubscribeToEvent(E_POSTUPDATE,URHO3D_HANDLER(MyApp,HandlePostUpdate));
        SubscribeToEvent(E_RENDERUPDATE,URHO3D_HANDLER(MyApp,HandleRenderUpdate));
        SubscribeToEvent(E_POSTRENDERUPDATE,URHO3D_HANDLER(MyApp,HandlePostRenderUpdate));
        SubscribeToEvent(E_ENDFRAME,URHO3D_HANDLER(MyApp,HandleEndFrame));
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
    void HandleKeyDown(StringHash eventType,VariantMap& eventData) {
        using namespace KeyDown;
        int key=eventData[P_KEY].GetInt();
        if(key==KEY_ESCAPE)
            engine_->Exit();

        if(key==KEY_Q) {
            Material* m = GetSubsystem<ResourceCache>()->GetResource<Material>("Materials/Stone.xml");
            m->SetFillMode((m->GetFillMode() == FILL_WIREFRAME) ? FILL_SOLID : FILL_WIREFRAME );
        }

        if(key==KEY_R) {
            planet_->birb_ = Max(planet_->birb_ - 1, 0);
            printf("caw %u\n", planet_->birb_);
        }

        if(key==KEY_F) {
            planet_->birb_ = Min(planet_->birb_ + 1, 7);
            printf("chirp %u\n", planet_->birb_);
        }

        if(key==KEY_T) {
            updatePlanet_ = !updatePlanet_;
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
        float timeStep=eventData[Update::P_TIMESTEP].GetFloat();
        framecount_++;
        time_+=timeStep;
        // Movement speed as world units per second
        float MOVE_SPEED=10.0f;
        // Mouse sensitivity as degrees per pixel
        const float MOUSE_SENSITIVITY=0.1f;

        if (updatePlanet_) {
            // Subtract 
            Vector3 dir(cameraNode_->GetPosition());
            float dist = dir.Length();
            dir /= dist;
            planet_->Update(dist, dir);
        }

        if(time_ >=0.2) {

            std::string str;
            str.append("OpenSpaceProgram " GIT_BRANCH "-" GIT_COMMIT_HASH ", too early\n"
                       "Tab: Toggle mouse, WASD: Move, Shift: Fast mode, Q: Toggle Wireframe\n"
                       "T: toggle planet update, R/F: LoD up/down, Esc: quit\n"
                       "--------------------------------------------------------------------\n");
            /*{
                std::ostringstream ss;
                ss<<framecount_;
                std::string s(ss.str());
                str.append(s.substr(0,6));
            }
            str.append(" frames in ");
            {
                std::ostringstream ss;
                ss<<time_;
                std::string s(ss.str());
                str.append(s.substr(0,6));
            }
            str.append(" seconds = ");
            */
            {
                std::ostringstream ss;
                ss<<(float)framecount_/time_;
                std::string s(ss.str());
                str.append("Frame R8    : ");
                str.append(s.substr(0,6));
                str.append("\n");
            }
            {
                std::ostringstream ss;
                ss<<planet_->GetVisibleCount();
                str.append("Tris Visible: ");
                std::string s(ss.str());
                str.append(s.substr(0,6));
            }
            {
                std::ostringstream ss;
                ss<<planet_->GetVisibleMax();
                std::string s(ss.str());
                str.append("/");
                str.append(s.substr(0,6));
                str.append("\n");
            }
            {
                std::ostringstream ss;
                ss<<planet_->GetTriangleCount();
                //ss<<",";
                //ss<<planet_->GetTriangleCount();
                str.append("Triangles   : ");
                std::string s(ss.str());
                str.append(s.substr(0,6));
            }
            {
                std::ostringstream ss;
                ss<<planet_->GetTriangleMax();
                std::string s(ss.str());
                str.append("/");
                str.append(s.substr(0,6));
                str.append("\n");
            }
            String s(str.c_str(),str.size());
            text_->SetText(s);
            ///URHO3D_LOGINFO(s);     // this show how to put stuff into the log
            framecount_=0;
            time_=0;
        }

        // Rotate the box thingy.
        // A much nicer way of doing this would be with a LogicComponent.
        // With LogicComponents it is easy to control things like movement
        // and animation from some IDE, console or just in game.
        // Alas, it is out of the scope for our simple example.
        boxNode_->Rotate(Quaternion(8*timeStep,16*timeStep,0));

        Input* input=GetSubsystem<Input>();
        if(input->GetQualifierDown(1))  // 1 is shift, 2 is ctrl, 4 is alt
            MOVE_SPEED*=100;
        if(input->GetKeyDown('W'))
            cameraNode_->Translate(Vector3(0,0, 1)*MOVE_SPEED*timeStep);
        if(input->GetKeyDown('S'))
            cameraNode_->Translate(Vector3(0,0,-1)*MOVE_SPEED*timeStep);
        if(input->GetKeyDown('A'))
            cameraNode_->Translate(Vector3(-1,0,0)*MOVE_SPEED*timeStep);
        if(input->GetKeyDown('D'))
            cameraNode_->Translate(Vector3( 1,0,0)*MOVE_SPEED*timeStep);

        if(input->GetKeyDown('E')) {
            //static uint e = 0;
            //uint xz[3];
            //xz[0] = 0;
            //xz[1] = 0;
            //xz[2] = 0;
            //planet_->indBuf_->SetDataRange(&xz, (e ++) * 3, 3);
        }

        if(!GetSubsystem<Input>()->IsMouseVisible()) {
            // Use this frame's mouse motion to adjust camera node yaw and pitch. Clamp the pitch between -90 and 90 degrees
            IntVector2 mouseMove=input->GetMouseMove();
            static float yaw_=0;
            static float pitch_=0;
            yaw_+=MOUSE_SENSITIVITY*mouseMove.x_;
            pitch_+=MOUSE_SENSITIVITY*mouseMove.y_;
            pitch_=Clamp(pitch_,-90.0f,90.0f);
            // Reset rotation and set yaw and pitch again
            cameraNode_->SetDirection(Vector3::FORWARD);
            cameraNode_->Yaw(yaw_);
            cameraNode_->Pitch(pitch_);
        }
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
        // scene_->GetComponent<Octree>()->DrawDebugGeometry(true);
    }
    /**
    * All good things must come to an end.
    */
    void HandleEndFrame(StringHash eventType,VariantMap& eventData) {
        // We really don't have anything useful to do here for this example.
        // Probably shouldn't be subscribing to events we don't care about.
    }
};

/**
* This macro is expanded to (roughly, depending on OS) this:
*
* > int RunApplication()
* > {
* > Urho3D::SharedPtr<Urho3D::Context> context(new Urho3D::Context());
* > Urho3D::SharedPtr<className> application(new className(context));
* > return application->Run();
* > }
* >
* > int main(int argc, char** argv)
* > {
* > Urho3D::ParseArguments(argc, argv);
* > return function;
* > }
*/
URHO3D_DEFINE_APPLICATION_MAIN(MyApp)
