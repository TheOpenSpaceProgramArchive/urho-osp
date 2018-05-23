#ifndef PLANWREN_H
#define PLANWREN_H

#include <cstdint>
#include <iostream>
#include <sstream>
#include <string>

#include <Urho3D/Core/CoreEvents.h>
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
#include <Urho3D/Input/InputEvents.h>
#include <Urho3D/Input/Input.h>
#include <Urho3D/IO/Log.h>
#include <Urho3D/Math/MathDefs.h>
#include <Urho3D/Resource/ResourceCache.h>
#include <Urho3D/Resource/XMLFile.h>
#include <Urho3D/Scene/SceneEvents.h>
#include <Urho3D/Scene/Scene.h>
#include <Urho3D/UI/Button.h>
#include <Urho3D/UI/Font.h>
#include <Urho3D/UI/Text.h>
#include <Urho3D/UI/UIEvents.h>
#include <Urho3D/UI/UI.h>

using namespace Urho3D;


// these lines are point to point indexed to 12 verticies of the
// icosahedron. All of these make up a complete wireframe. These buffers
// is not modified / reallocated in any way.
static constexpr const uint8_t lines_[60] { 
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
static constexpr const int8_t triangleSets_[60] {
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


class PlanWren;

class SubTriangle {
public:
    // 1,      2,        4,            8,           16,  32,   64, 128
    // Exists, Drawable, DownPointing, HasChildren, WhichChild, 
    uint8_t bitmask_;
    uint8_t set_;
    int lod_;

    // Indicies in set, Indicies in buffer, Index in gpu buffer
    uint glIndex_, x_, y_;

    uint children_[4];
    uint parent_;
    Vector3 normal_;

    SubTriangle();

    void CalculateNormal(PlanWren* wren);

};

class PlanWren {

    bool ready_ = false;
    ushort maxLOD_;
    uint shared_;
    uint owns_;
    uint setCount_;
    uint verticies_;
    //uint fundemental_ = 12;
    double size_;

    float* vertData_;
    uint* triangleMap_;

    Model* model_;
    SharedPtr<VertexBuffer> vrtBuf_;
    Vector<uint> indData_;
    Geometry* geometry_;

    uint maxTriangles_;

    SubTriangle* triangles_;
    uint* triBin_;
    uint triLeft_;

    uint* bufDomain_;
    uint bufActive_;
    uint bufMax_;

public:

    // here to test if things work
    SharedPtr<IndexBuffer> indBuf_;
    ushort birb_ = 4;

    //PlanWren();
    bool IsReady();
    uint GetIndex(uint8_t set, uint input);
    uint GetIndex(uint8_t set, uint x, uint y);
    const uint GetTriangleCount();
    const uint GetTriangleMax();
    const uint GetVisibleCount();
    const uint GetVisibleMax();
    const float* GetVertData();

    void Initialize(Context* context, double size, Scene* scene, ResourceCache* cache);
    void TriangleHide(SubTriangle* tri);
    void TriangleRemoveChildren(SubTriangle* tri);
    void TriangleRemove(uint index);
    void TriangleShow(SubTriangle* tri, uint index);
    void TriangleSubdivide(uint t);
    void Update(float distance, Vector3& dir);

    Model* GetModel();

protected:

    void RecursiveSightTest(uint triIndex, uint top, float distance, Vector3& dir);
    void RecursiveSubdivide(uint8_t set, uint basex, uint basey, uint size, bool down);

};

#endif
