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

using namespace Urho3D;

class SubTriangle {
public:
    unsigned char inuseBitmask_; // only first 4 bits are used

    SubTriangle* children_;
    Vector3 normal_;
};

class PlanWren {

    ushort maxLOD_;
    uint shared_;
    uint owns_;
    uint setCount_;
    uint verticies_;
    //uint fundemental_ = 12;
    float size_;

    unsigned char* lines_;
    signed char* triangleSets_;
    float* vertData_;
    uint* triangleMap_;

    Model* model_;
    SharedPtr<IndexBuffer> indBuf_;
    SharedPtr<VertexBuffer> vrtBuf_;
    Vector<uint> indData_;
    Geometry* geometry_;

    SubTriangle name[20];

public:

    PlanWren() {
        // these lines are point to point indexed to 12 verticies of the
        // icosahedron. All of these make up a complete wireframe. These buffers
        // is not modified / reallocated in any way.
        lines_ = new unsigned char[60] { 
            //       #        #        #        #
            0, 1,    0, 2,    0, 3,    0, 4,    0, 5,
            1, 2,    2, 3,    3, 4,    4, 5,    5, 1,
            1, 8,    8, 2,    2, 7,    7, 3,    3, 6,
            6, 4,    4, 10,   10, 5,   5, 9,    9, 1,
            6, 7,    7, 8,    8, 9,    9, 10,   10, 6,
            11, 6,   11, 7,   11, 8,   11, 9,   11, 10
            //       #        #        #        #
        };
        // Make triangles out of the lines above. [bottom, left, right]
        // Lines have direction. Triangle sets always go clockwise.
        // Since some lines may go the wrong way, negative means not reversed.
        // Index starts at 1, not zero, because there is no negative zero.
        triangleSets_ = new signed char[60] {
            //            #             #             #             #
            -6, 2, -1,    -7, 3, -2,    -8, 4, -3,    -9, 5, -4,    -10, 1, -5,
            6, -11, -12,  22, 13, 12,   7, -13, -14,  21, 15, 14,   8, -15, -16,
            25, 17, 16,   9, -17, -18,  24, 19, 18,   10, -19, -20, 23, 11, 20,
            -21, 27, -26, -22, 28, -27, -23, 29, -28, -24, 30, -29, -25, 26, -30
            //            #             #             #             #
        };

        // Each face is indexed like this. verticies on the edges are shared
        // with other triangles. The triangular numbers formula is used often
        // in this program.

        // LOD value is the number of subdivisions, divided in a very similar
        // way to the sierpinski triangle

        // # of verticies = (LOD^2 - 1) * LOD^2 / 2, separated into parts
        // in code.

        // Examples:

        // LOD: 0
        //         1
        //        2 3

        // LOD: 1
        //         1
        //        2 3
        //       4 5 6

        // LOD: 2
        //         1
        //       2   3
        //      4  5  6
        //    7  8  9  10
        //   11 12 13 14 15

        // Indicies in this space are referred to as a "Local triangle index"
        // Through different algorithms, can be converted to and from "buffer
        // index" which is the actual xyz vertex data in the buffer.
    }

    // Get buffer index from a set's local triangle index
    // Returns index in buffer
    uint GetIndex(unsigned char set, uint input) {
      // should be using urho logs but i'm lazy
      printf("T: %u, ", input);
      
      // This is set by some of the if statements. Used at the end if input is
      // one of the sides (bottom, left, right)
      //uint bufferLocation = 0;
      //uint offset = Abs(triangleSets_[set * 3] - 1);
      //bool reversed = false;
      
      // Test for different sides of the triangle
      // no, test for 3 corners first
      if (input == 0) {
          printf("TOP\n"); 
          return (lines_[(Abs(triangleSets_[set * 3 + 1]) - 1) * 2 + (triangleSets_[set * 3 + 1] < 0)]) * 6;
      } else if (input == setCount_ - 1) {
          printf("BOTTOM RIGHT %i\n", lines_[(Abs(triangleSets_[set * 3]) - 1) * 2 + (triangleSets_[set * 3] > 0)]);
          //      get set's bottom line index 0,                 add 1 if reversed
          return (lines_[(Abs(triangleSets_[set * 3]) - 1) * 2 + (triangleSets_[set * 3] > 0)]) * 6;
      } else if (input == setCount_ - 2 - shared_) {
          printf("BOTTOM LEFT %i\n", lines_[(Abs(triangleSets_[set * 3]) - 1) * 2 + (triangleSets_[set * 3] < 0)]);
          // Exactly the same as the one above, but reversed
          //      get set's bottom line index 0,                 don't add 1 if reversed
          return (lines_[(Abs(triangleSets_[set * 3]) - 1) * 2 + (triangleSets_[set * 3] < 0)]) * 6;
      } else if (input > setCount_ - shared_ - 3) {
          uint b = input - (setCount_ - shared_ - 1);
          printf("BOTTOM %u %u\n", b, (12 + shared_ * (Abs(triangleSets_[set * 3]) - 1) +
              ((triangleSets_[set * 3] < 0) ? b : (shared_ - 1 - b))));
          return (12 + shared_ * (Abs(triangleSets_[set * 3]) - 1) +
              ((triangleSets_[set * 3] < 0) ? b : (shared_ - 1 - b))) * 6;
      } else {
          // Find out which edge of the triangle it
          // inverse of right edge equation (1, 3, 6, 10, ...)
          // returns index starting at 1
          uint a = (Sqrt(8 * (input + 1) + 1) - 1) / 2;
          // After it was inversed, put it back into the original function
          // Since ints are being used, there are no fractions, and automatic
          // flooring is used. This can be used to determine which side the
          // vertex is on, (left or right)
          uint b = a * (a + 1) / 2;
          if (input + 1 == b) {
              // is on right edge
              // Triangle sets lists definitions of triangles. 3 numbers point to
              //  [0 bottom, 1 left, 2 right]
              // +2 refers to the right side
              b = a - 2;
              printf("RIGHT %u %u\n", b, (shared_ - 1 - b));
              return (12 + shared_ * (Abs(triangleSets_[set * 3 + 2]) - 1) +
                      ((triangleSets_[set * 3 + 2] < 0) ? b : (shared_ - 1 - b))) * 6;
          } else if (input == b) {
              // is on left edge
              printf("LEFT %u\n", shared_ - a);
              // variable reuse
              // Same as above 
              b = shared_ - a;
              printf("LEFT %u %u\n", b, (shared_ - 1 - b));
              return (12 + shared_ * (Abs(triangleSets_[set * 3 + 1]) - 1) +
                      ((triangleSets_[set * 3 + 1] > 0) ? b : (shared_ - 1 - b))) * 6;
          } else {
              b = (a - 2) * (a - 1) / 2 + (input - b) - 1;
              printf("CENTER %u\n", b);
              //return (12 + shared_ * 30 + owns_ * set + triangleMap_[input]);
          }
          
      }
      return 0;
    }

    void Initialize(Context* context, float size, Scene* scene, ResourceCache* cache) {

        size_ = size;
        maxLOD_ = 1;

        model_ = new Model(context);

        float s = size / 256.0f;
        float h = 114.486680448;

        // Pentagon stuff, from wolfram alpha
        // "X pointing" pentagon goes on the top

        float ca = 79.108350559987f;
        float cb = 207.10835055999f;
        float sa = 243.47046817156f;
        float sb = 150.47302458687f;

        //vertData_ = Vector<float>;
        //indData_ = new Vector<uint>;
        indBuf_ = new IndexBuffer(context);
        vrtBuf_ = new VertexBuffer(context);
        geometry_ = new Geometry(context);

        //maxLOD_ = i;
        int explode = Pow(ushort(2), maxLOD_);
        // Verticies on the lines, not including 12 fundamental icosahedron verts
        shared_ = explode - 1;
        // Verticies per triangle. (3, 6, 15, ...)
        setCount_ = (explode + 1) * (explode + 2) / 2;
        // Verticies in the middle of each face
        owns_ = (explode - 2) * (explode - 1) / 2;
        verticies_ = 12 + shared_ * 30 + owns_ * 20;
        printf("SET COUNT: %u PER SHARED: %u OWNS: %u EXPLODE: %u\n", setCount_, shared_, owns_, explode) ;
        printf("Size: %u EEE: %u O: %u\n", verticies_, lines_[5], owns_);

        triangleMap_ = new uint[owns_];

        uint j = 0;
        for (int i = 0; i < owns_; i ++) {
            uint a = (Sqrt(8 * (i + 1) + 1) - 1) / 2;
            uint b = a * (a + 1) / 2;
            if (i + 1 != b && i != b)
                printf("center: %u %u\n", i, j);
            triangleMap_[i] = (i + 1 != b && i != b) ? j++ : 0;
        }

        vertData_ = new float[verticies_ * 6];

        for(int i = 0; i < verticies_ * 6; i ++) {
            vertData_[i] = 0;
        }

        // There should be a better way to do this
        vertData_[0] = 0; // Top vertex 0
        vertData_[1] = 256;
        vertData_[2] = 0;
        vertData_[6] = 256; // Pentagon top aligned point 1
        vertData_[7] = h;
        vertData_[8] = 0;
        vertData_[12] = ca; // going clockwise from top 2
        vertData_[13] = h;
        vertData_[14] = -sa;
        vertData_[18] = -cb; // 3
        vertData_[19] = h;
        vertData_[20] = -sb;
        vertData_[24] = -cb; // 4
        vertData_[25] = h;
        vertData_[26] = sb;
        vertData_[30] = ca; // 5
        vertData_[31] = h;
        vertData_[32] = sa;
        vertData_[36] = -256; // Pentagon bottom aligned 6
        vertData_[37] = -h;
        vertData_[38] = 0;
        vertData_[42] = -ca; // 7
        vertData_[43] = -h;
        vertData_[44] = -sa;
        vertData_[48] = cb; // 8
        vertData_[49] = -h;
        vertData_[50] = -sb;
        vertData_[54] = cb; // 9
        vertData_[55] = -h;
        vertData_[56] = sb;
        vertData_[60] = -ca; // 10
        vertData_[61] = -h;
        vertData_[62] = sa;
        vertData_[66] = 0; // Bottom vertex 11
        vertData_[67] = -256;
        vertData_[68] = 0;

        for (int i = 0; i < 72; i += 6) {
            vertData_[i + 3] = vertData_[i + 0] / 256.0f;
            vertData_[i + 4] = vertData_[i + 1] / 256.0f;
            vertData_[i + 5] = vertData_[i + 2] / 256.0f;
            vertData_[i + 0] *= s;
            vertData_[i + 1] *= s;
            vertData_[i + 2] *= s;
        }

        for (int i = 0; i < 20; i++) {
            //GetIndex(0, uint(i));
            printf("WOOOT: %u\n", setCount_ - shared_ - 2);
            RecursiveSubdivide(i, setCount_ - shared_ - 2, shared_ + 2, 0, false);
            indData_.Push((lines_[(Abs(triangleSets_[i * 3]) - 1) * 2 + (triangleSets_[i * 3] > 0)]));
            indData_.Push((lines_[(Abs(triangleSets_[i * 3 + 1]) - 1) * 2 + (triangleSets_[i * 3 + 1] > 0)]));
            indData_.Push((lines_[(Abs(triangleSets_[i * 3 + 2]) - 1) * 2 + (triangleSets_[i * 3 + 2] > 0)]));
        }

        /*indData_.Push(0); // top
        indData_.Push(1);
        indData_.Push(2);

        indData_.Push(0);
        indData_.Push(2);
        indData_.Push(3);

        indData_.Push(0);
        indData_.Push(3);
        indData_.Push(4);

        indData_.Push(0);
        indData_.Push(4);
        indData_.Push(5);

        indData_.Push(0);
        indData_.Push(5);
        indData_.Push(1);

        indData_.Push(1);
        indData_.Push(9);
        indData_.Push(8); // center stuff, 8 is left, 9 is right

        indData_.Push(1);
        indData_.Push(8);
        indData_.Push(2);

        indData_.Push(2);
        indData_.Push(8);
        indData_.Push(7);

        indData_.Push(7);
        indData_.Push(3);
        indData_.Push(2);

        indData_.Push(3);
        indData_.Push(7);
        indData_.Push(6);

        indData_.Push(4);
        indData_.Push(3);
        indData_.Push(6);

        indData_.Push(4);
        indData_.Push(6);
        indData_.Push(10);

        indData_.Push(5);
        indData_.Push(4);
        indData_.Push(10);

        indData_.Push(5);
        indData_.Push(10);
        indData_.Push(9);

        indData_.Push(9);
        indData_.Push(1);
        indData_.Push(5);

        indData_.Push(11); // bottom
        indData_.Push(6);
        indData_.Push(7);

        indData_.Push(11);
        indData_.Push(7);
        indData_.Push(8);

        indData_.Push(11);
        indData_.Push(8);
        indData_.Push(9);

        indData_.Push(11);
        indData_.Push(9);
        indData_.Push(10);

        indData_.Push(11);
        indData_.Push(10);
        indData_.Push(6);*/

        PODVector<VertexElement> elements;
        elements.Push(VertexElement(TYPE_VECTOR3, SEM_POSITION));
        elements.Push(VertexElement(TYPE_VECTOR3, SEM_NORMAL));

        vrtBuf_->SetSize(verticies_ / 2, elements);
        vrtBuf_->SetData(vertData_);
        vrtBuf_->SetShadowed(true);

        indBuf_->SetSize(indData_.Size(), true, true);
        indBuf_->SetData(indData_.Buffer());
        indBuf_->SetShadowed(true);

        geometry_->SetNumVertexBuffers(1);
        geometry_->SetVertexBuffer(0, vrtBuf_);
        geometry_->SetIndexBuffer(indBuf_);
        geometry_->SetDrawRange(TRIANGLE_LIST, 0, indData_.Size());

        model_->SetNumGeometries(1);
        model_->SetGeometry(0, 0, geometry_);
        model_->SetBoundingBox(BoundingBox(Sphere(Vector3(0, 0, 0), size_)));
        Vector<SharedPtr<VertexBuffer> > vrtBufs;
        Vector<SharedPtr<IndexBuffer> > indBufs;
        vrtBufs.Push(vrtBuf_);
        indBufs.Push(indBuf_);
        PODVector<unsigned> morphRangeStarts;
        PODVector<unsigned> morphRangeCounts;
        morphRangeStarts.Push(0);
        morphRangeCounts.Push(0);
        model_->SetVertexBuffers(vrtBufs, morphRangeStarts, morphRangeCounts);
        model_->SetIndexBuffers(indBufs);

        for(int i=0;i<verticies_ * 6;i+=6) {
            printf("xyz: %.3f %.3f %.3f\n", vertData_[i], vertData_[i + 1], vertData_[i + 2]);
            //if (vertData_[i] + vertData_[i + 1] + vertData_[i + 2] != 0) {
                //std::cout << vertData[i] << " " << vertData[i + 1] << " " << vertData[i + 2] << "\n";
                Node* boxNode_=scene->CreateChild("Box");
                boxNode_->SetPosition(Vector3(vertData_[i],6 + vertData_[i + 1],vertData_[i + 2]));
                boxNode_->SetScale(Vector3(0.1f,0.1f,0.1f));
                StaticModel* boxObject=boxNode_->CreateComponent<StaticModel>();
                boxObject->SetModel(cache->GetResource<Model>("Models/Box.mdl"));
                //boxObject->SetModel(model);
                boxObject->SetMaterial(cache->GetResource<Material>("Materials/Stone.xml"));
                boxObject->SetCastShadows(true);
            //}
        }

    }
    
    Model* GetModel() {
        return model_;
    }

protected:
    // triangle set index, left side of triangle, length of each side, is pointing down
    void RecursiveSubdivide(unsigned char set, uint base, uint size, uint top, bool down) {
        // Fucnction would only stack up to the maxLOD, so overflow is unlikely

        // C is between of A and B. all in buffer index
        uint a = (Sqrt(8 * (base + 1) + 1) - 1) / 4;
        uint index = 0,
             halfe = a * (a + 1) / 2,
             vertA = GetIndex(set, base),
             vertB = GetIndex(set, base + size - 1),
             vertC = GetIndex(set, base + (size - 1) / 2);
        printf("BAASE: %u %u\n", base, halfe);
        // Loop for all 3 sides
        for (unsigned char i = 0; i < 3; i ++) {
            switch (i) {
                case 1:
                    // left
                    //vertA = GetIndex(set, base),
                    vertB = GetIndex(set, top);
                    //vertC = GetIndex(set, 1);
                    vertC = GetIndex(set, halfe);
                    //vertC = GetIndex(set, halfe + (size - 1) / 2 - 1);
                    break;
                case 2:
                    // right
                    //vertB = GetIndex(set, top);
                    vertA = GetIndex(set, base + size - 1);
                    vertC = GetIndex(set, halfe + (size - 1) / 2);
                    //vertC = GetIndex(set, 2);
                    break;
            }
            // Set vertex C to the average of A and B
            vertData_[vertC + 0] = (vertData_[vertA + 0] + vertData_[vertB + 0]) / 2.0f;
            vertData_[vertC + 1] = (vertData_[vertA + 1] + vertData_[vertB + 1]) / 2.0f;
            vertData_[vertC + 2] = (vertData_[vertA + 2] + vertData_[vertB + 2]) / 2.0f;
        }
        
    }

};

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
    SharedPtr<IndexBuffer> sindBuf_;

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
        text_->SetText("Keys: tab = toggle mouse, AWSD = move camera, Shift = fast mode, Esc = quit.\nWait a bit to see FPS.");
        // If the engine cannot find the font, it comes with Urho3D.
        // Set the environment variables URHO3D_HOME, URHO3D_PREFIX_PATH or
        // change the engine parameter "ResourcePrefixPath" in the Setup method.
        text_->SetFont(cache->GetResource<Font>("Fonts/Anonymous Pro.ttf"),20);
        text_->SetColor(Color(.3,0,.3));
        text_->SetHorizontalAlignment(HA_CENTER);
        text_->SetVerticalAlignment(VA_TOP);
        GetSubsystem<UI>()->GetRoot()->AddChild(text_);
        // Add a button, just as an interactive UI sample.
        Button* button=new Button(context_);
        // Note, must be part of the UI system before SetSize calls!
        GetSubsystem<UI>()->GetRoot()->AddChild(button);
        button->SetName("Button Quit");
        button->SetStyle("Button");
        button->SetSize(32,32);
        button->SetPosition(16,116);
        // Subscribe to button release (following a 'press') events
        SubscribeToEvent(button,E_RELEASED,URHO3D_HANDLER(MyApp,HandleClosePressed));

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

        PlanWren* planet = new PlanWren();
        planet->Initialize(context_, 2.0f, scene_, cache);

        //sindBuf_ = SharedPtr<IndexBuffer>(indBuf);
        //vertData.Reserve(12 * 3);

        // Vertex Buffer:
        // [12 fundementals, (x) shared lines, (x)*20 face indicies]

        Node* boxNode_=scene_->CreateChild("Box");
        boxNode_->SetPosition(Vector3(0,6,0));
        boxNode_->SetScale(Vector3(1.0f,1.0f,1.0f));
        StaticModel* boxObjectB=boxNode_->CreateComponent<StaticModel>();
        boxObjectB->SetModel(planet->GetModel());
        boxObjectB->SetMaterial(cache->GetResource<Material>("Materials/Stone.xml"));
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

        if(time_ >=1) {

            std::string str;
            str.append("Keys: tab = toggle mouse, AWSD = move camera, Shift = fast mode, Esc = quit.\n");
            {
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
            {
                std::ostringstream ss;
                ss<<(float)framecount_/time_;
                std::string s(ss.str());
                str.append(s.substr(0,6));
            }
            str.append(" fps");
            String s(str.c_str(),str.size());
            text_->SetText(s);
            URHO3D_LOGINFO(s);     // this show how to put stuff into the log
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
            MOVE_SPEED*=10;
        if(input->GetKeyDown('W'))
            cameraNode_->Translate(Vector3(0,0, 1)*MOVE_SPEED*timeStep);
        if(input->GetKeyDown('S'))
            cameraNode_->Translate(Vector3(0,0,-1)*MOVE_SPEED*timeStep);
        if(input->GetKeyDown('A'))
            cameraNode_->Translate(Vector3(-1,0,0)*MOVE_SPEED*timeStep);
        if(input->GetKeyDown('D'))
            cameraNode_->Translate(Vector3( 1,0,0)*MOVE_SPEED*timeStep);

        if(input->GetKeyDown('E')) {
            ushort* xz = new ushort[0, 0, 0, 0, 0, 0, 0, 0, 0];
            sindBuf_->SetDataRange(xz, 6, 9);
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
