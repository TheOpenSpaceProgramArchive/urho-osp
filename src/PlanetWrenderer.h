// "PlanetRenderer is a little too boring" -- Capital Asterisk, 2018
#pragma once

#include <cstdint>
#include <iostream>
#include <sstream>
#include <string>

#include <Urho3D/Core/CoreEvents.h>
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

// If this changes, then the universe is broken
static const int sc_icosahedronFaceCount = 20;

enum TriangleStats : uint8_t { E_SUBDIVIDED = 0b0001, E_VISIBLE = 0b0010, E_CHUNKED = 0b0100 };

// Index to a triangle
typedef uint32_t trindex;

// Index to a chunk
typedef unsigned chindex;

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

    // Data used when subdivided
    trindex m_children; // index to first child, always has 4 children if subdivided
    buindex m_midVerts[3]; // Bottom, Right, Left vertices in index buffer
    buindex m_index; // to index buffer

    // Data used when chunked
    chindex m_chunk; // Index to chunk. (First triangle ever chunked will be 0, second is 1)
    buindex m_chunkIndex; // Index to index data in the index buffer
    buindex m_chunkVerts; // Index to vertex data
};

struct UpdateRange
{
    buindex m_start, m_end;
    // initialize with maximum buindex value for start (2^32), and minimum buindex for end (0)
    // the first min and max operations will replace them
	UpdateRange() : m_start(Pow((int)2ul, (int)sizeof(buindex) * 8) - 1u), m_end(0) {
    }
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
    unsigned m_previewDepth; // minimum depth. guarentee to never subdivide below this

    Vector3 m_offset;
    Vector3 m_camera;
    float m_cameraDist;
    float m_threshold;

    float m_subdivAreaThreshold; // How much screen area a triangle can take before it should be subdivided
    float m_chunkAreaThreshold; // How much screen area a triangle can take before it should be chunked

    Model* m_model;

    Image* m_heightMap;

    // ** Base geometry for preview and slightly close up

    Geometry* m_geometry;

    SharedPtr<IndexBuffer> m_preview;
    SharedPtr<IndexBuffer> m_indBuf;
    PODVector<trindex> m_indDomain; // Maps index buffer indices to triangles
    buindex m_indCount;

    buindex m_maxIndices;

    SharedPtr<VertexBuffer> m_vertBuf;
    buindex m_vertCount;
    buindex m_maxVertice;

    PODVector<SubTriangle> m_triangles; // List of all triangles
    PODVector<trindex> m_trianglesFree; // Deleted triangles in the triangle class array
    buindex m_maxTriangles;

    PODVector<buindex> m_vertFree; // Deleted vertices in vertex buffer GPU data
    // use "m_indDomain[buindex]" to get a triangle index

    // Geometry for chunks

    Geometry* m_geometryChunk;

    SharedPtr<IndexBuffer> m_indBufChunk;
    unsigned m_chunkResolution; // How many vertices wide each chunk is
    unsigned m_chunkSharedCount; // How many shared verticies per chunk
    unsigned m_chunkSize; // How many vertices there are in each chunk
    unsigned m_chunkSizeInd; // How many triangles in each chunk
    chindex m_maxChunks; // Max number of chunks
    chindex m_chunkCount; // How many chunks there are right now

    PODVector<trindex> m_chunkIndDomain; // Maps chunks to triangles
    PODVector<chindex> m_chunkIndDeleteMe; // Spots in the index buffer that want to die

    SharedPtr<VertexBuffer> m_chunkVertBuf;
    buindex m_chunkMaxVert; // How large the chunk vertex buffer is in total
    buindex m_chunkMaxVertShared; // How much of it is reserved for shared vertices

    // This one is commented out because it's equal to the (# of chunks) * (# of middle vertices)
    //buindex m_vertCountChunkFixed; // Current number of fixed vertices
    buindex m_chunkVertCountShared; // Current of shared vertices (edges of chunks)
    PODVector<buindex> m_chunkVertFree; // Deleted chunk vertices that can be overwritten
    PODVector<buindex> m_chunkVertFreeShared; // Deleted shared chunk vertices that can be overwritten

    // it's impossible for a vertex to have more than 6 users
    // Delete a shared vertex when it's users goes to zero
    // And use user count to calculate normals
    PODVector<uint8_t> m_chunkVertUsers; // Count how many times each shared chunk vertex is being used
    PODVector<buindex> m_chunkSharedIndices; // Indicies that point to shared vertices.

    // Vertex buffer data is divided unevenly for chunks
    // In m_chunkVertBuf:
    // [shared vertex data] (m_chunkMaxVertShared) [middle vertices] (m_chunkMaxVert)

    // if chunk resolution is 16, then...
    // Chunks are triangles of 136 vertices (m_chunkSize)
    // There are 45 vertices on the edges, (sides + corners) = (14 + 14 + 14 + 3) = m_chunkSharedCount;
    // Which means there is 91 vertices left in the middle (m_chunkSize - m_chunkSharedCount)

public:

    PlanetWrenderer();
    ~PlanetWrenderer();
    bool is_ready() { return m_ready; }

    /**
     * Calculate initial icosahedron and initialize buffers. Call before drawing
     * @param context [in] Context used to initialize Urho3D objects
     * @param size [in] Minimum height of planet, or radius
     */
    void initialize(Context* context, Image* heightMap, double size);

    /**
     * Recalculates camera positiona and sub_recurses the main 20 triangles. Call this when the camera moves.
     * @param camera [in] Position of camera center
     */
    void update(const Vector3& camera);

    Model* get_model() { return m_model; }


protected:

    /**
     * A quick way to set neighbours of a triangle
     * @param tri [ref] Reference to triangle
     * @param bot [in] Bottom
     * @param rte [in] Right
     * @param lft [in] Left
     */
    static inline void set_neighbours(SubTriangle& tri, trindex bot, trindex rte, trindex lft);

    /**
     * A quick way to set vertices of a triangle
     * @param tri [ref] Reference to triangle
     * @param top [in] Top
     * @param lft Left
     * @param rte Right
     */
    static inline void set_verts(SubTriangle& tri, trindex top, trindex lft, trindex rte);

    /**
     * Find which side a triangle is on another triangle
     * @param [ref] tri Reference to triangle to be searched
     * @param [in] lookingFor Index of triangle to search for
     * @return Neighbour index (0 - 2), or bottom, left, or right
     */
    static inline const uint8_t neighboor_index(SubTriangle& tri, trindex lookingFor);

    void set_side_recurse(SubTriangle& tri, uint8_t side, trindex to);

    /**
     * Show or hide a triangle.
     * @param t [in] Index to the triangle
     * @param visible [in] To hide or to show
     */
    void set_visible(trindex t, bool visible, UpdateRange* gpuInd = nullptr);

    /**
     * Recursively check every triangle for which ones need (un)subdividing or chunking
     * @param t [in] Triangle to start with
     */
    void sub_recurse(trindex t);

    /**
     * Subdivide a triangle into 4 more
     * @param [in] Triangle to subdivide
     */
    void subdivide_add(trindex t, UpdateRange* gpuVert = nullptr, UpdateRange* gpuInd = nullptr);

    /**
     * Unsubdivide a triangle. Removes children and sets neighbours of neighbours
     * @param t [in] Index of triangle to unsubdivide
     */
    void subdivide_remove(trindex t, UpdateRange* gpuVert = nullptr, UpdateRange* gpuInd = nullptr);

    /**
     * Calculates and sets m_center
     * @param tri [ref] Reference to triangle
     */
    void calculate_center(SubTriangle& tri);

    /**
     *
     * @param t [in] Index of triangle to add chunk to
     * @param gpuIgnore
     */
    void chunk_add(trindex t, UpdateRange* gpuVertChunk = nullptr, UpdateRange* gpuVertInd = nullptr);

    /**
     * @brief chunk_remove
     * @param t
     * @param gpuIgnore
     */
    void chunk_remove(trindex t, UpdateRange* gpuVertChunk = nullptr, UpdateRange* gpuVertInd = nullptr);

    /**
     * Get triangle from vector of triangles
     * be careful of reallocation!
     * @param t [in] Index to triangle
     * @return Pointer to triangle
     */
    inline SubTriangle* get_triangle(trindex t) const { return m_triangles.Buffer() + t; }

    /**
     * Convert XY coordinates to a triangular number index
     *
     * 0
     * 1  2
     * 3  4
     * 6  7  8  9
     * x = right, y = down
     *
     * @param x [in]
     * @param y [in]
     * @return
     */
    inline const unsigned get_index(int x, int y) const;

    /**
     * Similar to the normal get_index, but the first possible indices returned makes a border around the triangle
     *
     * 6
     * 5  7
     * 4  9  8
     * 3  2  1  0
     * x = right, y = down
     *
     * 0, 1, 2, 3, 4, 5, 6, 7, 8 makes a ring
     *
     * @param x [in]
     * @param y [in]
     * @return
     */
    const unsigned get_index_ringed(int x, int y) const;

    /**
     * Add up memory usages from most variables associated with this instance.
     * @return Memory usage in bytes
     */
    const uint64_t get_memory_usage() const;

    /**
     * For debugging only: search for triangles that should have been deleted
     * @param tri
     */
    void find_refs(SubTriangle& tri);

};
