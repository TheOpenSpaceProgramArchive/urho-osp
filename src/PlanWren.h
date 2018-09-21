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

// The 20 faces of the icosahedron (Top, Left, Right)
// Each number pointing to a vertex
static constexpr const uint8_t sc_icoTemplateTris[20 * 3] {
//  TT  LL  RR   TT  LL  RR   TT  LL  RR   TT  LL  RR   TT  LL  RR
     0,  1,  2,   0,  2,  3,   0,  3,  4,   0,  4,  5,   0,  5,  1,
     8,  2,  1,   2,  8,  7,   7,  3,  2,   3,  7,  6,   6,  4,  3,
     4,  6, 10,  10,  5,  4,   5,  10, 9,   9,  1,  5,   1,  9,  8,
    11,  6,  7,  11,  7,  8,  11,  8,  9,  11,  9, 10,  11, 10,  6
};

// The 20 faces of the icosahedron (Bottom, Right, Left)
static constexpr const uint8_t sc_icoTemplateNeighbors[20 * 3] {
//  BB  RR  LL   BB  RR  LL   BB  RR  LL   BB  RR  LL   BB  RR  LL

};

enum TriangleStats : uint8_t { E_SUBDIVIDED = 0b0001, E_VISIBLE = 0b0010 };

typedef uint32_t trindex;
typedef uint32_t buindex;

struct SubTriangle
{
    trindex m_parent;
    trindex m_children; // always has 4 children if subdivided
    trindex m_neighbors[3];
    buindex m_midVerts[3];
    buindex m_verts[3];
    buindex m_index;
    //bool subdivided;
    uint8_t m_bitmask; // 1: subdivided, 2: visible
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
    trindex* m_indDomain;
    buindex m_indCount;

    SharedPtr<VertexBuffer> m_vertBuf;
    PODVector<buindex> m_vertFree;
    buindex m_vertCount;

    PODVector<SubTriangle> m_triangles;
    PODVector<trindex> m_trianglesFree;

public:

    ushort birb_ = 4;

    PlanWren();
    ~PlanWren();
    bool is_ready() {return m_ready;}

    void initialize(Context* context, double size, Scene* scene, ResourceCache* cache);

    Model* get_model() { return m_model; }

protected:

    void subdivide(trindex t);
    void unsubdivide(trindex t);

    void set_side_recurse(SubTriangle* tri, uint8_t side, uint8_t to);
    void set_visible(trindex t, bool visible);
    void sub_recurse(trindex t, float something, uint8_t depth);

    inline SubTriangle* get_triangle(trindex t) { return &(m_triangles[t]); }
};
