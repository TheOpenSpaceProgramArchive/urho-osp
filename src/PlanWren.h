#pragma once

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

typedef uint32_t trindex;
typedef uint32_t buindex;

struct SubTriangle
{
    trindex m_parent;
    trindex m_children; // always has 4 children if subdivided
    trindex m_neighbors[3];
    buindex m_midVerts[3];
    //bool subdivided;
    uint8_t m_bitmask; // subdivided,
    uint8_t m_depth;
};

class PlanWren
{

    bool m_ready = false;
    double m_size;

    buindex m_maxFaces;
    buindex m_maxVertice;
    buindex m_maxIndices;

    Model* m_model;
    Geometry* m_geometry;

    SharedPtr<IndexBuffer> m_indBuf;
    trindex m_indDomain[];
    buindex m_indCount;

    SharedPtr<VertexBuffer> m_vertBuf;
    PODVector<buindex> m_vertFree;
    buindex m_vertCount;

    PODVector<SubTriangle> m_triangles;
    PODVector<trindex> trianglesFree;

public:

    ushort birb_ = 4;

    //PlanWren();
    bool IsReady();

    void Initialize(Context* context, double size, Scene* scene, ResourceCache* cache);

    Model* GetModel();

protected:



};
