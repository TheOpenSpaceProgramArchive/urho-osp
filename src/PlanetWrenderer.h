// "PlanetRenderer is a little too boring" -- Capital Asterisk, 2018
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
     0,  2,  1,   0,  3,  2,   0,  4,  3,   0,  5,  4,   0,  1,  5,
     8,  1,  2,   2,  7,  8,   7,  2,  3,   3,  6,  7,   6,  3,  4,
     4,  10, 6,  10,  4,  5,   5,  9, 10,   9,  5,  1,   1,  8,  9,
    11,  7,  6,  11,  8,  7,  11,  9,  8,  11, 10,  9,  11,  6, 10
};

// The 20 faces of the icosahedron (Bottom, Right, Left)
static constexpr const uint8_t sc_icoTemplateneighbours[20 * 3] {
//  BB  RR  LL   BB  RR  LL   BB  RR  LL   BB  RR  LL   BB  RR  LL
     5,  4,  1,   7,  0,  2,   9,  1,  3,  11,  2,  4,  13,  3,  0,
     0,  6, 14,  16,  5,  7,   1,  8,  6,  15,  7,  9,   2, 10,  8,
    19,  9, 11,   3, 12, 10,  18, 11, 13,   4, 14, 12,  17, 13,  5,
     8, 19, 16,   6, 15, 17,  14, 16, 18,  12, 17, 19,  10, 18, 15
};

static const int sc_icosahedronFaceCount = 20;

enum TriangleStats : uint8_t { E_SUBDIVIDED = 0b0001, E_VISIBLE = 0b0010, E_CHUNKED = 0b0100 };

// Index to a triangle
typedef uint32_t trindex;

// Index to a buffer
typedef uint32_t buindex;

struct SubTriangle
{
    trindex m_parent;
    trindex m_neighbours[3];
    buindex m_corners[3]; // to vertex buffer, 3 corners of triangle

    //bool subdivided;
    uint8_t m_bitmask;
    uint8_t m_depth;
    Vector3 m_center;
    // not really sure if this is worth doing
    union {
        // Can't have chunk data and be subdivided/visible at the same time
        union {
            // Data used when subdivided
            trindex m_children; // index to first child, always has 4 children if subdivided
            buindex m_midVerts[3]; // Bottom, Right, Left vertices in index buffer
            buindex m_index; // to index buffer
        };
        union {
            // Chunk data
            // All these refer to the m_indBufFine
            buindex m_chunkSides[3]; // Sides of the triangle that can be shared with other triangles
            buindex m_chunkCorners[3]; // 3 corners of the chunk triangle
            buindex m_chunkMeat; // center vertices of the chunk (the largest of all these three)
        };
        //TriChunk m_chunkData;
    };
};

//struct TriChunk
//{
//    buindex[3]
//};

class PlanetWrenderer
{
    bool m_ready = false;
    bool m_fineMode;
    double m_radius;

    unsigned m_maxDepth;

    buindex m_maxTriangles;
    buindex m_maxVertice;
    buindex m_maxIndices;

    Vector3 m_offset;
    Vector3 m_camera;
    float m_cameraDist;
    float m_threshold;

    Model* m_model;

    // ** Base geometry for preview and slightly close up

    Geometry* m_geometry;

    SharedPtr<IndexBuffer> m_indBuf;
    buindex m_indCount;

    SharedPtr<VertexBuffer> m_vertBuf;
    buindex m_vertCount;

    // List of all triangles
    PODVector<SubTriangle> m_triangles;
    // Records empty spaces in the triangle class array
    PODVector<trindex> m_trianglesFree;
    // Records empty spaces in the vertex buffer GPU data
    PODVector<buindex> m_vertFree;
    // Map index buffer indices to triangle indices
    // m_indDomain[buindex] = trindex
    trindex* m_indDomain;

    // Geometry for fine details (chunks)

    Geometry* m_geometryFine;

    SharedPtr<IndexBuffer> m_indBufFine;
    buindex m_indCountFine;

    SharedPtr<VertexBuffer> m_vertBuffFine;
    buindex m_vertCountFine;

public:

    PlanetWrenderer();
    ~PlanetWrenderer();
    bool is_ready() {return m_ready;}

    void initialize(Context* context, double size);
    void update(const Vector3& camera);


    Model* get_model() { return m_model; }


protected:

    static inline void set_verts(SubTriangle& tri, trindex top, trindex lft, trindex rte);
    static inline void set_neighbours(SubTriangle& tri, trindex bot, trindex rte, trindex lft);
    static inline uint8_t neighboor_index(SubTriangle& tri, trindex lookingFor);

    void subdivide(trindex t);
    void unsubdivide(trindex t);
    void calculate_center(SubTriangle& tri);

    void set_side_recurse(SubTriangle& tri, uint8_t side, trindex to);
    void set_visible(trindex t, bool visible);
    void sub_recurse(trindex t);

    void generate_chunk(trindex t);

    inline SubTriangle* get_triangle(trindex t) { return m_triangles.Buffer() + t; }

    // for debugging only
    void find_refs(SubTriangle& tri);

};
