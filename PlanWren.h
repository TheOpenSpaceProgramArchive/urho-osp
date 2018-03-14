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

class PlanWren;

class SubTriangle {
public:
    unsigned char inuseBitmask_; // only first 4 bits are used
    unsigned char set_;

    // Indicies in set, Indicies in buffer, Index in gpu buffer
    uint    lod_, glIndex_,
            x_, y_;

    SubTriangle* children_;
    Vector3 normal_;

    SubTriangle();

    void calculateNormal(PlanWren& wren);

};

class PlanWren {

    ushort maxLOD_;
    uint shared_;
    uint owns_;
    uint setCount_;
    uint verticies_;
    //uint fundemental_ = 12;
    double size_;

    unsigned char* lines_;
    signed char* triangleSets_;
    float* vertData_;
    uint* triangleMap_;

    Model* model_;
    SharedPtr<VertexBuffer> vrtBuf_;
    Vector<uint> indData_;
    Geometry* geometry_;

    uint maxTriangles_;
    SubTriangle* triangles_;

public:

    // here to test if things work
    SharedPtr<IndexBuffer> indBuf_;

    PlanWren();
    uint GetIndex(unsigned char set, uint input);
    uint GetIndex(unsigned char set, uint x, uint y);
    const float* GetVertData();

    void Initialize(Context* context, double size, Scene* scene, ResourceCache* cache);
    void Update(float distance, Vector3& dir);

    Model* GetModel();

protected:

    void RecursiveSightTest(float distance, Vector3& dir, uint triIndex);
    void RecursiveSubdivide(unsigned char set, uint basex, uint basey, uint size, bool down);

};

PlanWren::PlanWren() {
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
    //   1
    //   2  3
    //   4  5  6
    //   7  8  9  10
    //   11 12 13 14 15

    // Indicies in this space are referred to as a "Local triangle index"
    // Through different algorithms, can be converted to and from "buffer
    // index" which is the actual xyz vertex data in the buffer.
}

// Get buffer index from a set's local triangle index
// Returns index in buffer
// Should be deprecated
uint PlanWren::GetIndex(unsigned char set, uint input) {
    // should be using urho logs but i'm lazy
    //printf("T: %u, ", input);
    
    // This is set by some of the if statements. Used at the end if input is
    // one of the sides (bottom, left, right)
    //uint bufferLocation = 0;
    //uint offset = Abs(triangleSets_[set * 3] - 1);
    //bool reversed = false;
    
    // Test for different sides of the triangle
    // no, test for 3 corners first
    if (input == 0) {
        //printf("TOP\n"); 
        return (lines_[(Abs(triangleSets_[set * 3 + 1]) - 1) * 2 + (triangleSets_[set * 3 + 1] < 0)]) * 6;
    } else if (input == setCount_ - 1) {
        //printf("BOTTOM RIGHT %i\n", lines_[(Abs(triangleSets_[set * 3]) - 1) * 2 + (triangleSets_[set * 3] > 0)]);
        //      get set's bottom line index 0,                 add 1 if reversed
        return (lines_[(Abs(triangleSets_[set * 3]) - 1) * 2 + (triangleSets_[set * 3] > 0)]) * 6;
    } else if (input == setCount_ - 2 - shared_) {
        //printf("BOTTOM LEFT %i\n", lines_[(Abs(triangleSets_[set * 3]) - 1) * 2 + (triangleSets_[set * 3] < 0)]);
        // Exactly the same as the one above, but reversed
        //      get set's bottom line index 0,                 don't add 1 if reversed
        return (lines_[(Abs(triangleSets_[set * 3]) - 1) * 2 + (triangleSets_[set * 3] < 0)]) * 6;
    } else if (input > setCount_ - shared_ - 3) {
        uint b = input - (setCount_ - shared_ - 1);
        //printf("BOTTOM %u %u\n", b, (12 + shared_ * (Abs(triangleSets_[set * 3]) - 1) +
        //    ((triangleSets_[set * 3] < 0) ? b : (shared_ - 1 - b))));
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
            //printf("RIGHT %u %u\n", b, (shared_ - 1 - b));
            return (12 + shared_ * (Abs(triangleSets_[set * 3 + 2]) - 1) +
                    ((triangleSets_[set * 3 + 2] < 0) ? b : (shared_ - 1 - b))) * 6;
        } else if (input == b) {
            // is on left edge
            //printf("LEFT %u\n", shared_ - a);
            // variable reuse
            // Same as above 
            b = shared_ - a;
            //printf("LEFT %u %u\n", b, (shared_ - 1 - b));
            return (12 + shared_ * (Abs(triangleSets_[set * 3 + 1]) - 1) +
                    ((triangleSets_[set * 3 + 1] < 0) ? b : (shared_ - 1 - b))) * 6;
        } else {
            b = (a - 2) * (a - 1) / 2;
            //printf("CENTER %u %u\n", input, triangleMap_[input]);
            //printf("C2 %u\n", (12 + shared_ * 30 + owns_ * uint(set) + b));
            return (12 + shared_ * 30 + owns_ * uint(set) + triangleMap_[input]) * 6;
        }
        
    }
    return 0;
}

// Same, and simpler than above. inputs x y coordinates instead
uint PlanWren::GetIndex(unsigned char set, uint x, uint y) {
    printf("XY: %u, (%u, %u) ", set, x, y);
    if (y == 0) {
        printf("TOP %u\n", (Abs(triangleSets_[set * 3 + 1]) - 1)); 
        //set = 0;
        return (lines_[(Abs(triangleSets_[set * 3 + 1]) - 1) * 2 + (triangleSets_[set * 3 + 1] < 0)]) * 6;
    } else if (y == shared_ + 1) {
        if (x == 0) {
            printf("BOTTOM LEFT\n");
            return (lines_[(Abs(triangleSets_[set * 3]) - 1) * 2
                      + (triangleSets_[set * 3] < 0)]) * 6;
        } else if (x == shared_ + 1) {
            printf("BOTTOM RIGHT\n");
            return (lines_[(Abs(triangleSets_[set * 3]) - 1) * 2
                      + (triangleSets_[set * 3] > 0)]) * 6;
        } else {
            printf("BOTTOM\n");
            return (12 + shared_ * (Abs(triangleSets_[set * 3]) - 1) +
                      ((triangleSets_[set * 3] < 0) ? x - 1 : (shared_ - x))) * 6;
        }
    } else if (y < shared_ + 1) {
        if (x == 0) {
            printf("LEFT\n");
            return (12 + shared_ * (Abs(triangleSets_[set * 3 + 1]) - 1) +
                  ((triangleSets_[set * 3 + 1] > 0) ? y - 1 : (shared_ - y))) * 6;
        } else if (x == y) {
            printf("RIGHT\n");
            return (12 + shared_ * (Abs(triangleSets_[set * 3 + 2]) - 1) +
                  ((triangleSets_[set * 3 + 2] < 0) ? y - 1 : (shared_ - y))) * 6;
        } else {
            printf("CENTER\n");
            return (12 + shared_ * 30 + owns_ * uint(set) + (y - 2) * (y - 1) / 2 - 1 + x) * 6;
        }
    } else {
        printf("error\n");
    }
}

const float* PlanWren::GetVertData() {
    return vertData_;
}

void PlanWren::Initialize(Context* context, double size, Scene* scene, ResourceCache* cache) {

    size_ = size;
    maxLOD_ = 2;

    model_ = new Model(context);

    float s = size / 280.43394944265;
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
    printf("PLAN WREN INITIALIZATION\n");
    printf("SET COUNT: %u PER SHARED: %u OWNS: %u EXPLODE: %u\n", setCount_, shared_, owns_, explode) ;
    printf("SIZE: %u EEE: %u O: %u\n", verticies_, lines_[5], owns_);

    triangleMap_ = new uint[setCount_];

    uint j = 0;
    for (int i = 0; i < setCount_; i ++) {
        uint a = (Sqrt(8 * (i + 1) + 1) - 1) / 2;
        uint b = a * (a + 1) / 2;
        //if (i + 1 != b && i != b)
        //    printf("center: %u %u\n", i, j);
        triangleMap_[i] = (i + 1 != b && i != b) ? j++ : 0;
    }

    vertData_ = new float[verticies_ * 6];

    for(int i = 0; i < verticies_ * 6; i ++) {
        vertData_[i] = 0;
    }

    // There should be a better way to do this
    vertData_[0] = 0; // Top vertex 0
    vertData_[1] = 280.43394944265;
    vertData_[2] = 0;
    vertData_[6] = 280.43394944265; // Pentagon top aligned point 1
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
        //printf("WOOOT: %u\n", setCount_ - shared_ - 2);
        RecursiveSubdivide(i, 0, shared_ + 1, shared_ + 2, false);
        //indData_.Push((lines_[(Abs(triangleSets_[i * 3]) - 1) * 2 + (triangleSets_[i * 3] > 0)]));
        //indData_.Push((lines_[(Abs(triangleSets_[i * 3 + 1]) - 1) * 2 + (triangleSets_[i * 3 + 1] > 0)]));
        //indData_.Push((lines_[(Abs(triangleSets_[i * 3 + 2]) - 1) * 2 + (triangleSets_[i * 3 + 2] > 0)]));
    }

    for (uint i = 0; i < 20; i++) {
        uint coA = 0, coB = 1, coC = 1, coD = 0;
        while (coA !=  (setCount_ - (shared_ + 2))) {
            indData_.Push(GetIndex(i, coB) / 6);
            indData_.Push(GetIndex(i, coA) / 6);
            indData_.Push(GetIndex(i, coB + 1) / 6);
            coA ++;
            coB ++;
            if (coA - coD == coC) {
              coD += coC;
              coC ++;
              coB ++;
              
            }
            
            //printf("EE: %u:  %u  %u  %u\n", i, GetIndex(i, coB) / 6, GetIndex(i, coA) / 6, GetIndex(i, coB + 1) / 6);
        }
    }

    for (uint k = 0; k < 20; k++) {
        uint tc = 0;
        for (uint i = 0; i < shared_ + 2; i++) {
            for (uint j = 0; j < i + 1; j++) {
                printf("T: %u (%u == %u)\n", tc, GetIndex(k, j, i), GetIndex(k, tc));
                tc ++;
            }
        }
    }

    // Normalizing, making it into a sphere, and adding normals
    double vx, vy, vz;
    for (int i = 0; i < verticies_ * 6; i += 6) {
        
        vx = vertData_[i + 0];
        vy = vertData_[i + 1];
        vz = vertData_[i + 2];
        double mag = Sqrt(vx * vx
                          + vy * vy
                          + vz * vz);
        vertData_[i + 0] = float(vx / mag * size_);
        vertData_[i + 1] = float(vy / mag * size_);
        vertData_[i + 2] = float(vz / mag * size_);
        vertData_[i + 3] = float(vx / mag);
        vertData_[i + 4] = float(vy / mag);
        vertData_[i + 5] = float(vz / mag);
        
    }

    PODVector<VertexElement> elements;
    elements.Push(VertexElement(TYPE_VECTOR3, SEM_POSITION));
    elements.Push(VertexElement(TYPE_VECTOR3, SEM_NORMAL));

    vrtBuf_->SetSize(verticies_, elements);
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

    //for(int i=0;i<verticies_ * 6;i+=6) {
        //printf("xyz: %.3f %.3f %.3f\n", vertData_[i], vertData_[i + 1], vertData_[i + 2]);
        //if (vertData_[i] + vertData_[i + 1] + vertData_[i + 2] != 0) {
            //std::cout << vertData[i] << " " << vertData[i + 1] << " " << vertData[i + 2] << "\n";
            //Node* boxNode_=scene->CreateChild("Box");
            //boxNode_->SetPosition(Vector3(vertData_[i], vertData_[i + 1],vertData_[i + 2]));
            //boxNode_->SetScale(Vector3(0.04f,0.04f,0.04f));
            //StaticModel* boxObject=boxNode_->CreateComponent<StaticModel>();
            //boxObject->SetModel(cache->GetResource<Model>("Models/Box.mdl"));
            //boxObject->SetModel(model);
            //boxObject->SetMaterial(cache->GetResource<Material>("Materials/Stone.xml"));
            //boxObject->SetCastShadows(true);
        //}
    //}


    maxTriangles_ = 30;
    triangles_ = new SubTriangle[maxTriangles_];

}

// Dir is outwards from the center, and is normalized
void PlanWren::Update(float distance, Vector3& dir) {
    for (uint i = 0; i < 12; i ++) {
        RecursiveSightTest(distance, dir, i);
    }
}

Model* PlanWren::GetModel() {
    return model_;
}

void PlanWren::RecursiveSightTest(float distance, Vector3& dir, uint triIndex) {
    SubTriangle* tri = triangles_ + triIndex;
    float thing = size_ / distance;
    //float dot = dir.DotProduct(Vector3(tri->normal_, 0, 0));
    
}

// triangle set index, left side of triangle, length of each side, top of the triangle, is pointing down
void PlanWren::RecursiveSubdivide(unsigned char set, uint basex, uint basey, uint size, bool down) {
    // Fucnction would only stack only up to the maxLOD, so overflow is unlikely

    // C is between of A and B. all in buffer index
    //uint a = (Sqrt(8 * (base) + 1) - 1) / 2;
    //uint b = (Sqrt(8 * (top) + 1) - 1) / 2;
    uint index = 0;

    uint halfe, vBotRit, vBotLft, vTop, vBtm, vLft, vRit;

    // TODO do boolean multiplication stuff
    if (down) {
        // If triangle is pointing down, MIRROR VERTICALLY
        //halfe = a + (size - 1) / 2;
        //halfe = halfe * (halfe + 1) / 2 + (base - a * (a + 1) / 2) + (b - halfe);
        halfe = basey + (size - 1) / 2;

        vBotRit = GetIndex(set, basex + size - 1, basey);
        vBotLft = GetIndex(set, basex, basey);
        vTop = GetIndex(set, basex, basey + size - 1);

        vBtm = GetIndex(set, basex + (size - 1) / 2, basey);
        vLft = GetIndex(set, basex, halfe);
        vRit = GetIndex(set, basex + (size - 1) / 2, halfe);
    } else {
        // Normal pointing up triangle
        //halfe = a - (size - 1) / 2;
        //halfe = halfe * (halfe + 1) / 2 + (base - a * (a + 1) / 2);
        halfe = basey - (size - 1) / 2;

        vBotRit = GetIndex(set, basex + size - 1, basey);
        vBotLft = GetIndex(set, basex, basey);
        vTop = GetIndex(set, basex, basey - size + 1);

        vBtm = GetIndex(set, basex + (size - 1) / 2, basey);
        vLft = GetIndex(set, basex, halfe);
        vRit = GetIndex(set, basex + (size - 1) / 2, halfe);
    }

    // Start with bottom side
    uint vertA = vBotLft,
         vertB = vBotRit,
         vertC = vBtm;
    //printf("AAAA: SIZE: %u BASE: %u %u TOP: %u %u HALF: %uL%u\n", size, base, a, top, b, halfe, a - (size - 1) / 2);
    
    // Loop for all 3 sides
    for (unsigned char i = 0; i < 3; i ++) {
        switch (i) {
            case 1:
                // left
                vertB = vTop;
                vertC = vLft;
                break;
            case 2:
                // right
                vertA = vBotRit;
                vertC = vRit;
                break;
        }
        // Set vertex C to the average of A and B
        double vx, vy, vz;
        vx = (vertData_[vertA + 0] + vertData_[vertB + 0]) / 2.0f;
        vy = (vertData_[vertA + 1] + vertData_[vertB + 1]) / 2.0f;
        vz = (vertData_[vertA + 2] + vertData_[vertB + 2]) / 2.0f;
        double mag = Sqrt(vx * vx
                          + vy * vy
                          + vz * vz);
        vertData_[vertC + 0] = vx;//float(vx / mag * double(size_));
        vertData_[vertC + 1] = vy;//float(vy / mag * double(size_));
        vertData_[vertC + 2] = vz;//float(vz / mag * double(size_));
    }
    if (size != 3) {
        // 0, 3, 5
        //printf("EEE %u\n", (size + 1) / 2);
        // Bottom right
        //RecursiveSubdivide(set, base + (size - 1) / 2, (size + 1) / 2 - 1,
        //                    halfe + (size - 1) / 2, false);
        // Bottom left
        //RecursiveSubdivide(set, base, (size + 1) / 2 - 1, halfe, false);
        // Top
        //RecursiveSubdivide(set, halfe, (size + 1) / 2 - 1, top, false);
        //RecursiveSubdivide(set, 3, 3, 0, false);
        // Center
        //RecursiveSubdivide(set, halfe, (size + 1) / 2 - 1,
        //                    base + (size - 1) / 2, !down);
        
        // Bottom Right
        RecursiveSubdivide(set, basex + (size - 1) / 2, basey, Pow(uint(2), LogBaseTwo(size - 1) - 1) + 1, down);
        // Bottom Left
        RecursiveSubdivide(set, basex, basey, (size - 1) / 2 + 1, down);
        // Top
        RecursiveSubdivide(set, basex, halfe, Pow(uint(2), LogBaseTwo(size - 1) - 1) + 1, down);
        // Center
        RecursiveSubdivide(set, basex, halfe, Pow(uint(2), LogBaseTwo(size - 1) - 1) + 1, !down);
    }
};

SubTriangle::SubTriangle() {
    inuseBitmask_ = 0;
    set_ = 20;
    lod_ = 0;
    glIndex_ = 0;
    x_ = 1;
    y_ = 0;
}

void SubTriangle::calculateNormal(PlanWren& wren) {
    uint size = Pow(uint(2), lod_);
    uint a = wren.GetIndex(set_, x_ + size, y_);
    uint b = wren.GetIndex(set_, x_, y_);
    uint c = wren.GetIndex(set_, x_, y_ + size);
    const float* vdata = wren.GetVertData();
    normal_.x_ = vdata[a + 3];
    normal_.y_ = vdata[a + 4];
    normal_.z_ = vdata[a + 5];
    normal_.x_ += vdata[b + 3];
    normal_.y_ += vdata[b + 4];
    normal_.z_ += vdata[b + 5];
    normal_.x_ += vdata[c + 3];
    normal_.y_ += vdata[c + 4];
    normal_.z_ += vdata[c + 5];
    normal_.Normalize();
}
