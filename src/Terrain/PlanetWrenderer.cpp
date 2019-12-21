// "PlanetRenderer is a little too boring" -- Capital Asterisk, 2018

#include "PlanetWrenderer.h"

namespace osp
{

void IcoSphereTree::initialize()
{

    // Set preferences to some magic numbers
    // TODO: implement a planet config file or something
    m_maxDepth = 5;
    m_minDepth = 0;

    m_maxVertice = 512;
    m_maxTriangles = 256;

    // Pentagon stuff, from wolfram alpha
    // This part is kind of messy and should be revised
    // "X pointing" pentagon goes on the top

    // TODO: Replace constants with compile-time calculations.
    // Enables the compiler to recognize higher level math
    // operations and re-arrange some computations appropriately.
    static constexpr float h = 114.486680448f;
    static constexpr float ca = 79.108350559987f;
    static constexpr float cb = 207.10835055999f;
    static constexpr float sa = 243.47046817156f;
    static constexpr float sb = 150.47302458687f;

    // Reserve some space on the vertex buffer
    m_vertBuf.Reserve(m_maxVertice * m_vertCompCount);
    m_vertBuf.Resize(m_maxVertice * 6);

    float* vertInit = m_vertBuf.Buffer();

    // VERT XYZ, NORMAL XYZ, VERT XYZ, NORMAL XYZ, ...
    // Set initial data for the normal icosahedron
    // There should be a better way to do this (as of now i'm still too lazy)
    vertInit[0] = 0; // Top vertex 0
    vertInit[1] = 280.43394944265f;
    vertInit[2] = 0;
    vertInit[6] = 280.43394944265f; // Pentagon top aligned point 1
    vertInit[7] = h;
    vertInit[8] = 0;
    vertInit[12] = ca; // going clockwise from top 2
    vertInit[13] = h;
    vertInit[14] = -sa;
    vertInit[18] = -cb; // 3
    vertInit[19] = h;
    vertInit[20] = -sb;
    vertInit[24] = -cb; // 4
    vertInit[25] = h;
    vertInit[26] = sb;
    vertInit[30] = ca; // 5
    vertInit[31] = h;
    vertInit[32] = sa;
    vertInit[36] = -256; // Pentagon bottom aligned 6
    vertInit[37] = -h;
    vertInit[38] = 0;
    vertInit[42] = -ca; // 7
    vertInit[43] = -h;
    vertInit[44] = -sa;
    vertInit[48] = cb; // 8
    vertInit[49] = -h;
    vertInit[50] = -sb;
    vertInit[54] = cb; // 9
    vertInit[55] = -h;
    vertInit[56] = sb;
    vertInit[60] = -ca; // 10
    vertInit[61] = -h;
    vertInit[62] = sa;
    vertInit[66] = 0; // Bottom vertex 11
    vertInit[67] = -256;
    vertInit[68] = 0;

    m_vertCount = 12; // 12 Vertices make up a basic icosahedron

    // Normalize into the right sized sphere
    float vx, vy, vz;
    for (int i = 0; i < 68; i += 6)
    {

        vx = vertInit[i + 0];
        vy = vertInit[i + 1];
        vz = vertInit[i + 2];
        float mag = Urho3D::Sqrt(  vx * vx
                                 + vy * vy
                                 + vz * vz);
        vertInit[i + 0] = vx / mag * float(m_radius);
        vertInit[i + 1] = vy / mag * float(m_radius);
        vertInit[i + 2] = vz / mag * float(m_radius);
        vertInit[i + 3] = vx / mag;
        vertInit[i + 4] = vy / mag;
        vertInit[i + 5] = vz / mag;
    }

    // Allocate some space on empty triangles array
    m_triangles.Reserve(3000);


    // This part is instuctions saying that
    // position and normal data is interleved
    //PODVector<VertexElement> elements;
    //elements.Push(VertexElement(TYPE_VECTOR3, SEM_POSITION));
    //elements.Push(VertexElement(TYPE_VECTOR3, SEM_NORMAL));


    // Initialize first 20 triangles, indices from sc_icoTemplateTris
    for (int i = 0; i < gc_icosahedronFaceCount; i ++)
    {
        // Set triangles
        SubTriangle tri;
        //printf("Triangle: %p\n", t);
        //tri.m_parent = 0;

        // indices were already calculated beforehand
        set_verts(tri, sc_icoTemplateTris[i * 3 + 0],
                        sc_icoTemplateTris[i * 3 + 1],
                        sc_icoTemplateTris[i * 3 + 2]);

        // which triangles neighboor which were calculated beforehand too
        set_neighbours(tri, sc_icoTemplateneighbours[i * 3 + 0],
                            sc_icoTemplateneighbours[i * 3 + 1],
                            sc_icoTemplateneighbours[i * 3 + 2]);

        tri.m_bitmask = 0;
        tri.m_depth = 0;
        calculate_center(tri);
        m_triangles.Push(tri);
        //if (i != 0)
        //set_visible(i, true);
    }
}


void IcoSphereTree::set_neighbours(SubTriangle& tri,
                                   trindex bot,
                                   trindex rte,
                                   trindex lft)
{
    tri.m_neighbours[0] = bot;
    tri.m_neighbours[1] = rte;
    tri.m_neighbours[2] = lft;
}

void IcoSphereTree::set_verts(SubTriangle& tri, trindex top,
                              trindex lft, trindex rte)
{
    tri.m_corners[0] = top;
    tri.m_corners[1] = lft;
    tri.m_corners[2] = rte;
}

int IcoSphereTree::neighbour_side(const SubTriangle& tri,
                                   const trindex lookingFor)
{
    // Loop through neighbours on the edges. child 4 (center) is not considered
    // as all it's neighbours are its siblings
    if(tri.m_neighbours[0] == lookingFor) return 0;
    if(tri.m_neighbours[1] == lookingFor) return 1;
    if(tri.m_neighbours[2] == lookingFor) return 2;

    // this means there's an error
    assert(false);
    return 255;
}

void PlanetWrenderer::initialize(Urho3D::Context* context,
                                 Urho3D::Image* heightMap, double size)
{

    // Set preferences to some magic numbers
    // TODO: implement a planet config file or something

    m_subdivAreaThreshold = 0.02f;
    m_chunkMaxVertShared = 10000;
    m_maxChunks = 300;

    m_chunkAreaThreshold = 0.04f;
    m_chunkResolution = 31;
    m_chunkVertsPerSide = m_chunkResolution - 1;

    // Make the subdividable icosphere that acts like a skeleton for
    // PlanetWrenderer to stitch chunks over
    m_icoTree = Urho3D::SharedPtr<IcoSphereTree>(new IcoSphereTree());
    m_icoTree->m_radius = size;

    m_model = new Urho3D::Model(context);
    m_model->SetNumGeometries(1);

    // Set bounding box to a sphere centered in the middle of the model with a
    // diameter of (radius * 2)
    m_model->SetBoundingBox(Urho3D::BoundingBox(
                                Urho3D::Sphere(Urho3D::Vector3::ZERO,
                                       float(size) * 2.0f)));

    m_icoTree->initialize();

    // Chunks
    {
        m_chunkCount = 0;

        m_chunkSize = m_chunkResolution * (m_chunkResolution + 1) / 2;

        // This is how many triangles is in a chunk
        m_chunkSizeInd = Urho3D::Pow(m_chunkVertsPerSide, 2u);
        m_chunkSharedCount = m_chunkVertsPerSide * 3;


        m_chunkMaxVert = m_chunkMaxVertShared
                        + m_maxChunks * (m_chunkSize - m_chunkSharedCount);

        m_chunkVertCountShared = 0;

        // Initialize objects for dealing with chunks
        m_indBufChunk = new Urho3D::IndexBuffer(context);
        m_chunkVertBuf = new Urho3D::VertexBuffer(context);
        m_geometryChunk = new Urho3D::Geometry(context);
        m_chunkIndDomain.Resize(m_maxChunks);
        m_chunkVertUsers.Resize(m_chunkMaxVertShared);

        // Say that each vertex has position, normal, and tangent data
        Urho3D::PODVector<Urho3D::VertexElement> elements;
        elements.Push(Urho3D::VertexElement(Urho3D::TYPE_VECTOR3,
                                            Urho3D::SEM_POSITION));
        elements.Push(Urho3D::VertexElement(Urho3D::TYPE_VECTOR3,
                                            Urho3D::SEM_NORMAL));
        //elements.Push(VertexElement(TYPE_VECTOR3, SEM_TEXCOORD));
        //elements.Push(VertexElement(TYPE_VECTOR3, SEM_COLOR));

        m_chunkVertBuf->SetSize(m_chunkMaxVert, elements);
        m_chunkVertBuf->SetShadowed(true);

        m_indBufChunk->SetSize(m_maxChunks * m_chunkSizeInd * 3, true, true);
        m_indBufChunk->SetShadowed(true);

        // Create the geometry, urho3d specific
        m_geometryChunk->SetNumVertexBuffers(1);
        m_geometryChunk->SetVertexBuffer(0, m_chunkVertBuf);
        m_geometryChunk->SetIndexBuffer(m_indBufChunk);
        m_geometryChunk->SetDrawRange(Urho3D::TRIANGLE_LIST, 0,
                                      gc_icosahedronFaceCount * 3);

        // Add geometry to model, urho3d specific
        m_model->SetGeometry(0, 0, m_geometryChunk);

        // Calculate m_chunkSharedIndices;  use:

        // Example of verticies in a chunk:
        // 0
        // 1  2
        // 3  4  5
        // 6  7  8  9

        // Chunk index data is stored as an array of (top, left, right):
        // { 0, 1, 2,  1, 3, 4,  4, 2, 1,  2, 4, 5,  3, 6, 7, ... }
        // Each chunk has the same number of triangles,
        // and are equally spaced in the buffer.
        // There are duplicates of the same index

        // Vertices in the edges of a chunk are considered "shared vertices,"
        // shared with the edges of the chunk's neighboors.
        // (in the example above: all of the vertices except for #4 are shared)
        // Shared vertices are added and removed in an unspecified order.
        // Their reserved space in m_chunkVertBuf is [0 - m_chunkSharedCount]

        // Vertices in the middle are only used by one chunk
        // (only 4), at larger sizes, they outnumber the shared ones
        // They are equally spaced in the vertex buffer
        // Their reserved space in m_chunkVertBuf is [m_chunkSharedCount - max]

        // It would be convinient if the indicies of the edges of the chunk
        // (shared verticies) can be accessed as if they were ordered in a
        // clockwise fasion around the edge of the triangle.
        // (see get_index_ringed).

        // so m_chunkSharedIndices is filled with indices to indices
        // indexToSharedVertex = tri->m_chunkIndex + m_chunkSharedIndices[0]

        // An alterative to this would be storing a list of shared vertices per
        // chunk, which takes more memory

        m_chunkSharedIndices.Resize(m_chunkSharedCount);

        int i = 0;
        for (int y = 0; y < int(m_chunkVertsPerSide); y ++)
        {
            for (int x = 0; x < y * 2 + 1; x ++)
            {
                // Only consider the up-pointing triangle
                // All of the shared vertices can still be covered
                if (!(x % 2))
                {
                    for (int j = 0; j < 3; j ++)
                    {
                        unsigned localIndex;

                        switch (j)
                        {
                        case 0:
                            localIndex = get_index_ringed(x / 2, y);
                            break;
                        case 1:
                            localIndex = get_index_ringed(x / 2, y + 1);
                            break;
                        case 2:
                            localIndex = get_index_ringed(x / 2 + 1, y + 1);
                            break;

                        }

                        if (localIndex < m_chunkSharedCount)
                        {
                             m_chunkSharedIndices[localIndex]
                                     = unsigned(i + j);
                        }

                    }
                }
                i += 3;
            }
        }

    }

    // Not sure what this is doing, urho3d specific
    Urho3D::Vector<Urho3D::SharedPtr<Urho3D::VertexBuffer> > vrtBufs;
    Urho3D::Vector<Urho3D::SharedPtr<Urho3D::IndexBuffer> > indBufs;
    vrtBufs.Push(m_chunkVertBuf);
    indBufs.Push(m_indBufChunk);
    Urho3D::PODVector<unsigned> morphRangeStarts;
    Urho3D::PODVector<unsigned> morphRangeCounts;
    morphRangeStarts.Push(0);
    morphRangeCounts.Push(0);
    m_model->SetVertexBuffers(vrtBufs, morphRangeStarts, morphRangeCounts);
    m_model->SetIndexBuffers(indBufs);

    m_ready = true;

    // don't mind this debug code
    m_icoTree->subdivide_add(0);
    //chunk_add(0);

    for (trindex i = 1; i < 24; i ++)
    {
        chunk_add(i);
    }
}

void IcoSphereTree::subdivide_add(trindex t)
{
    // if bottom triangle is deeper, use that vertex
    // same with left and right

    SubTriangle* tri = get_triangle(t);

    // Add the 4 new triangles
    // Top Left Right Center
    unsigned freeSize = m_trianglesFree.Size();
    if (freeSize == 0)
    {
        // Make new triangles
        tri->m_children = m_triangles.Size();
        m_triangles.Resize(tri->m_children + 4);

        // Reassign pointer in case of reallocation
        tri = get_triangle(t);
        //tri->children[1] = freeSize + 1;
        //tri->children[2] = freeSize + 2;
        //tri->children[3] = freeSize + 3;
    }
    else
    {
        // Free triangles always come in groups of 4
        // as triangles are always deleted in groups of 4
        // pop doesn't return the value for Urho, so save last element then pop
        const trindex j = m_trianglesFree[m_trianglesFree.Size() - 1];
        m_trianglesFree.Pop();
        tri->m_children = j;
        //tri->children[1] = j + 1;
        //tri->children[2] = j + 2;
        //tri->children[3] = j + 3;
    }

    SubTriangle* children = get_triangle(tri->m_children);

    // Set the neighboors of the top triangle to:
    // bottom neighboor = new middle triangle
    // right neighboor  = right neighboor of parent (tri)
    // left neighboor   = left neighboor of parent (tri)
    set_neighbours(children[0], tri->m_children + 3,
                                tri->m_neighbours[1], tri->m_neighbours[2]);
    // same but for every other triangle
    set_neighbours(children[1], tri->m_neighbours[0],
                                tri->m_children + 3, tri->m_neighbours[2]);
    set_neighbours(children[2], tri->m_neighbours[0],
                                tri->m_neighbours[1], tri->m_children + 3);
    // the middle triangle is completely surrounded by its siblings
    set_neighbours(children[3], tri->m_children + 0,
                                tri->m_children + 1, tri->m_children + 2);

    // Inherit m_depth
    children[0].m_depth = children[1].m_depth
                        = children[2].m_depth
                        = children[3].m_depth
                        = tri->m_depth + 1;
    // Set m_bitmasks to 0, for not visible, not subdivided, not chunked
    children[0].m_bitmask = children[1].m_bitmask
                        = children[2].m_bitmask
                        = children[3].m_bitmask
                        = 0;
    // Subdivide lines and add verticies, or take from other triangles

    // Preparation to write to vertex buffer
    //unsigned char* vertData = m_vertBuf->GetShadowData();
    //unsigned vertSize = m_vertBuf->GetVertexSize();
    //float writeMe[3];
    //printf("Vertex size: %u\n", vertSize);

    // Loop through 3 sides of the triangle: Bottom, Right, Left
    // tri.sides refers to an index of another triangle on that side
    for (int i = 0; i < 3; i ++) {
        SubTriangle* triB = get_triangle(tri->m_neighbours[i]);
        // Check if the line is already subdivided,
        // or if there is no triangle on the other side
        if (!(triB->m_bitmask & gc_triangleMaskSubdivided)
                || (triB->m_depth != tri->m_depth)) {
            // A new vertex has to be created in the middle of the line
            if (m_vertFree.Size() == 0) {
                tri->m_midVerts[i] = m_vertCount;
                m_vertCount ++;
            } else {
                tri->m_midVerts[i] = m_vertFree[m_vertFree.Size() - 1];
                m_vertFree.Pop();
            }

            // Technique taken from an urho3D example
            // Read vertex buffer data as Vector3
            const Urho3D::Vector3& vertA =
                    (*reinterpret_cast<const Urho3D::Vector3*>(
                         m_vertBuf.Buffer()
                            + m_vertCompCount * tri->m_corners[(i + 1) % 3]));
            const Urho3D::Vector3& vertB =
                    (*reinterpret_cast<const Urho3D::Vector3*>(
                         m_vertBuf.Buffer()
                            + m_vertCompCount * tri->m_corners[(i + 2) % 3]));

            Urho3D::Vector3 vertM[2];
            vertM[1] = ((vertA + vertB) / 2).Normalized();

            vertM[0] = vertM[1] * float(m_radius);

            memcpy(m_vertBuf.Buffer() + tri->m_midVerts[i] * m_vertCompCount,
                   vertM, m_vertCompCount * sizeof(float));
        }
        else
        {
            // Which side tri is on triB
            int sideB = neighbour_side(*triB, t);
            //printf("Vertex is being shared\n");

            // Instead of creating a new vertex, use the one from triB since
            // it's already subdivided
            tri->m_midVerts[i] = triB->m_midVerts[sideB];
            //console.log(i + ": Used existing vertex");

            // Set sides
            // Side 0(bottom) corresponds to child 1(left), 2(right)
            // Side 1(right) corresponds to child 2(right), 0(top)
            // Side 2(left) corresponds to child 0(top), 1(left)

            // triX/Y refers to the two triangles on the side of tri
            // triBX/Y refers to the two triangles on the side of triB
            trindex triX = tri->m_children + trindex((i + 1) % 3);
            trindex triY = tri->m_children + trindex((i + 2) % 3);
            trindex triBX = triB->m_children + trindex((sideB + 1) % 3);
            trindex triBY = triB->m_children + trindex((sideB + 2) % 3);

            // Assign the face of each triangle to the other triangle beside it
            m_triangles[triX].m_neighbours[i] = triBY;
            m_triangles[triY].m_neighbours[i] = triBX;
            //m_triangles[triBX].m_neighbours[sideB] = triY;
            //m_triangles[triBY].m_neighbours[sideB] = triX;
            set_side_recurse(m_triangles[triBX], uint8_t(sideB), triY);
            set_side_recurse(m_triangles[triBY], uint8_t(sideB), triX);

            //printf("Set Tri%u %u to %u", triBX)
        }

    }

    // Set verticies
    set_verts(children[0],
            tri->m_corners[0], tri->m_midVerts[2], tri->m_midVerts[1]);
    set_verts(children[1],
            tri->m_midVerts[2], tri->m_corners[1], tri->m_midVerts[0]);
    set_verts(children[2],
            tri->m_midVerts[1], tri->m_midVerts[0], tri->m_corners[2]);
    // The center triangle is made up of purely middle vertices.
    set_verts(children[3],
            tri->m_midVerts[0], tri->m_midVerts[1], tri->m_midVerts[2]);

    // Calculate centers
    calculate_center(children[0]);
    calculate_center(children[1]);
    calculate_center(children[2]);
    calculate_center(children[3]);

    tri->m_bitmask ^= gc_triangleMaskSubdivided;

    // subdivide if below minimum depth
    if (tri->m_depth < m_minDepth)
    {
        // Triangle vector might reallocate, so accessing tri after
        // subdivide_add will cause undefiend behaviour
        //URHO3D_LOGINFOF("depth: %i tri: %i", tri->m_depth, t);
        trindex childs = tri->m_children;
        subdivide_add(childs + 0);
        subdivide_add(childs + 1);
        subdivide_add(childs + 2);
        subdivide_add(childs + 3);
    }
}

void IcoSphereTree::subdivide_remove(trindex t)
{
    SubTriangle* tri = get_triangle(t);

    if (!(tri->m_bitmask & gc_triangleMaskSubdivided))
    {
        // If not subdivided
        return;
    }

    // unsubdiv children if subdivided, and hide if hidden
    for (trindex i = 0; i < 4; i ++)
    {
        if (m_triangles[tri->m_children + i].m_bitmask
                & gc_triangleMaskSubdivided)
        {
            subdivide_remove(tri->m_children + i);
        }
        else
        {
            //find_refs(m_triangles[tri->m_children + i]);
        }

        //chunk_remove(tri->m_children + i);
    }


    // Loop through all sides but the middle
    for (int i = 0; i < 3; i ++)
    {
        SubTriangle* triB = get_triangle(tri->m_neighbours[i]);

        // If the triangle on the other side is not subdivided
        // it means that the vertex will have no more users
        if (!(triB->m_bitmask & gc_triangleMaskSubdivided)
                || triB->m_depth != tri->m_depth)
        {
            // Mark side vertex for replacement
            m_vertFree.Push(tri->m_midVerts[i]);

        }
        // else leave it alone, other triangle beside is still using the vertex

        // Set neighbours, so that they don't reference deleted triangles
        if (triB->m_depth == tri->m_depth)
        {
            int sideB = neighbour_side(triB[0], t);
            set_side_recurse(triB[0], sideB, t);
        }
    }

    // Now mark triangles for removal. they're always in groups of 4.
    m_trianglesFree.Push(tri->m_children);

    tri->m_bitmask ^= gc_triangleMaskSubdivided;
    tri->m_children = unsigned(-1);
}

void IcoSphereTree::calculate_center(SubTriangle &tri)
{
    const float* vertData = m_vertBuf.Buffer();
    const Urho3D::Vector3& vertA = (*reinterpret_cast<const Urho3D::Vector3*>(
                                vertData
                                + m_vertCompCount * tri.m_corners[0]));
    const Urho3D::Vector3& vertB = (*reinterpret_cast<const Urho3D::Vector3*>(
                                vertData
                                + m_vertCompCount * tri.m_corners[1]));
    const Urho3D::Vector3& vertC = (*reinterpret_cast<const Urho3D::Vector3*>(
                                vertData
                                + m_vertCompCount * tri.m_corners[2]));

    tri.m_center = (vertA + vertB + vertC) / 3.0f;
    //printf("Center: %s\n", tri.m_center.ToString().CString());
}

/**
 * Set a neighbour of a triangle, and apply for all of it's children's
 * @param tri [ref] Reference to triangle
 * @param side [in] Which side to set
 * @param to [in] Neighbour to operate on
 */
void IcoSphereTree::set_side_recurse(SubTriangle& tri, int side, trindex to)
{
    tri.m_neighbours[side] = to;
    if (tri.m_bitmask & gc_triangleMaskSubdivided) {
        set_side_recurse(m_triangles[tri.m_children + ((side + 1) % 3)],
                side, to);
        set_side_recurse(m_triangles[tri.m_children + ((side + 2) % 3)],
                side, to);
    }
}

void PlanetWrenderer::update(const Urho3D::Vector3& camera)
{
    m_camera = camera;
    m_cameraDist = camera.Length();

    // size/distance is equal to the dot product between the camera position
    // vector, and the surface normal at the viewd edge of a perfect sphere
    // 0.45f is added because the triangles at the edge of the spehere are
    // curved, and are still visible even when they point away from the camera.
    m_threshold = float(m_icoTree->m_radius) / m_cameraDist - 0.45f;

    //printf("Camera! %s\n", camera.ToString().CString());
    //printf("vert count: %ux\n", m_vertCount);
    for (trindex i = 0; i < gc_icosahedronFaceCount; i ++) {
        sub_recurse(i);
    }

    //URHO3D_LOGINFOF("Memory Usage: %fMb",
    //                float(get_memory_usage()) / 1000000.0f);

    assert(m_icoTree->m_triangles.Size() < m_icoTree->m_maxTriangles);
}

void PlanetWrenderer::sub_recurse(trindex t)
{
    SubTriangle* tri = m_icoTree->get_triangle(t);

    bool shouldSubdivide, shouldChunk;

    // Icosahedron edge length equations
    // let r = radius of circumscribed sphere
    // let a = edge length
    // from this equation: r = (a / 4) * sqrt(10 + 2 * sqrt(5))
    // arrange to this:    a = 4r / sqrt(10 + 2 * sqrt(5))
    // this equation now calculates edge length from radius
    // divide by 2^depth because the edge has been subdivided in powers of two

    // close enough approximation
    // (should be a bit higher because it's spherical)
    float edgeLength = float(4.0 * m_icoTree->m_radius)
                       / Urho3D::Sqrt(10.0f + 2.0f * Urho3D::Sqrt(5.0f))
                       / Urho3D::Pow(2, int(tri->m_depth));

    // Approximation of the triangle's area
    // (Area of equalateral triangle)
    float triArea = Urho3D::Sqrt(3) * edgeLength * edgeLength / 4;

    // Distance squared from viewer
    float distanceSquared = (tri->m_center - m_camera).LengthSquared();

    // How much space this triangle takes up on screen using inverse square law
    // InverseSquareDistance * Area -> area / distancesquared
    // 0.2 is magic number to nicely fit things on screen
    float screenArea = triArea / (distanceSquared * 0.2f);

    // Maximum screen area a triangle can take before it's subdivided
    shouldSubdivide = screenArea > m_subdivAreaThreshold;

    // Same but for chunks
    shouldChunk = screenArea > m_chunkAreaThreshold;


    //chunk_add(t);

    // Check if already subdivided
    if (tri->m_bitmask & gc_triangleMaskSubdivided)
    {

        if (shouldSubdivide)
        {
            if (tri->m_depth < m_icoTree->m_maxDepth)
            {
                // triangle vector might reallocate making tri invalid
                // so keep a copy of m_children
                trindex childs = tri->m_children;
                sub_recurse(childs + 0);
                sub_recurse(childs + 1);
                sub_recurse(childs + 2);
                sub_recurse(childs + 3);
            }
        }
        else if (tri->m_depth > m_icoTree->m_minDepth)
        {
            m_icoTree->subdivide_remove(t);
        }
    }
    else
    {
        if (shouldSubdivide)
        {
            if (tri->m_depth < m_icoTree->m_maxDepth)
            {
                m_icoTree->subdivide_add(t);
                return;
            }
        }

        if (shouldChunk)
        {
            chunk_add(t);
        }
        else
        {
            chunk_remove(t);

        }
    }
}

unsigned PlanetWrenderer::get_index_ringed(unsigned x, unsigned y) const
{
    // || (x == y) ||
    if (y == m_chunkVertsPerSide)
    {
        // Bottom edge
        return x;
    }
    else if (x == 0)
    {
        // Left edge
        return m_chunkVertsPerSide * 2 + y;
    }
    else if (x == y)
    {
        // Right edge
        return m_chunkVertsPerSide * 2 - y;
    }
    else
    {
        // Center
        return (m_chunkSharedCount) + get_index(x - 1, y - 2);
    }

}

void PlanetWrenderer::chunk_add(trindex t, UpdateRange* gpuVertChunk,
                                UpdateRange* gpuVertInd)
{
    SubTriangle* tri = m_icoTree->get_triangle(t);

    if (m_chunkCount >= m_maxChunks)
    {
        URHO3D_LOGERRORF("Chunk limit reached");
        return;
    }

    if (tri->m_bitmask & (gc_triangleMaskChunked | gc_triangleMaskSubdivided))
    {
        // return if already chunked, or is subdivided
        return;
    }

    // Think of tri as a right triangle like this
    //
    // dirDown  dirRight--->
    // |
    // |   o
    // |   oo
    // V   ooo
    //     oooo
    //     ooooo    o = a single vertex
    //     oooooo
    //
    //     <----> m_chunkResolution

    float* vertData = m_icoTree->m_vertBuf.Buffer();

    unsigned char* vertDataChunk = m_chunkVertBuf->GetShadowData();
    unsigned vertSizeChunk = m_chunkVertBuf->GetVertexSize();

    // top, left, and right vertices of triangle from IcoSphereTree
    const Urho3D::Vector3 verts[3] = {
        (*reinterpret_cast<const Urho3D::Vector3*>(vertData
                        + m_icoTree->m_vertCompCount * tri->m_corners[0])),
        (*reinterpret_cast<const Urho3D::Vector3*>(vertData
                        + m_icoTree->m_vertCompCount * tri->m_corners[1])),
        (*reinterpret_cast<const Urho3D::Vector3*>(vertData
                        + m_icoTree->m_vertCompCount * tri->m_corners[2]))
    };

    const Urho3D::Vector3 dirRight = (verts[2] - verts[1])
                                     / m_chunkVertsPerSide;
    const Urho3D::Vector3 dirDown = (verts[1] - verts[0])
                                    / m_chunkVertsPerSide;

    // Loop through neighbours and see which ones are already chunked to share
    // vertices with

    //uint8_t neighbourDepths[3];
    SubTriangle* neighbours[3];
    trindex neighbourSide[3]; // Side of tri relative to neighbour's

    for (int i = 0; i < 3; i ++)
    {
        neighbours[i] = m_icoTree->get_triangle(tri->m_neighbours[i]);
        neighbourSide[i] = m_icoTree->neighbour_side(*neighbours[i], t);
        //neighbourDepths[i] = (triB->m_bitmask & gc_triangleMaskChunked);
    }

    // Take the space at the end of the chunk buffer
    tri->m_chunk = m_chunkCount;

    if (m_chunkVertFree.Size() == 0) {
        //
        tri->m_chunkVerts = m_chunkMaxVertShared + m_chunkCount
                            * (m_chunkSize - m_chunkSharedCount);
    }
    else
    {
        // Use empty space available in the chunk vertex buffer
        tri->m_chunkVerts = m_chunkVertFree.Back();
        m_chunkVertFree.Pop();
    }

    Urho3D::PODVector<unsigned> indices(m_chunkSize);

    unsigned middleIndex = 0;
    int i = 0;
    for (int y = 0; y < int(m_chunkResolution); y ++)
    {
        // Loops once, then twice, then thrice, etc...
        // because a triangle starts with 1 vertex at the top
        // 2 next row, 3 next, 4 next, 5 next, etc...
        for (int x = 0; x <= y; x ++)
        {
            unsigned vertIndex;
            unsigned localIndex = get_index_ringed(x, y);
            bool shared = false;

            // Check if on edge
            if (localIndex < m_chunkSharedCount)
            {
                shared = true;

                // Both of these should get optimized into a single div op
                unsigned side = localIndex / m_chunkVertsPerSide;
                unsigned sideInd = localIndex % m_chunkVertsPerSide;
                // side 0: Bottom
                // side 1: Right
                // side 2: Left

                // Get shared vertex from neighbour, on corresponding side
                //m_chunkSharedIndices[];

            }

            //URHO3D_LOGINFOF("X:%i Y:%i I:%i", x, y, i);

            if (shared)
            {

                if (false)
                {
                    // TODO: Take a vertex from a neighbour

                    // Get edge that localIndex is on
                    // Get corresponding neighbour's edge
                    // Set vertIndex to neighbour's shared vertex
                    // Increase user count for that vertex
                }
                else
                {
                    // Make a new shared vertex

                    // Indices from 0 to m_chunkMaxVertShared
                    if (m_chunkVertCountShared + 1 >= m_chunkMaxVertShared)
                    {
                        URHO3D_LOGERROR("Max Shared Vertices for Chunk");
                        return;
                    }
                    if (m_chunkVertFreeShared.Size() == 0) {
                        vertIndex = m_chunkVertCountShared;
                    }
                    else
                    {
                        vertIndex = m_chunkVertFreeShared.Back();
                        m_chunkVertFreeShared.Pop();
                    }

                    m_chunkVertCountShared ++;
                    m_chunkVertUsers[vertIndex] = 1;
                }
            }
            else
            {
                // Use a vertex from the space defined earler
                vertIndex = tri->m_chunkVerts + middleIndex;

                // Keep track of which middle index is being looped through
                middleIndex ++;
            }

            Urho3D::Vector3 pos = verts[0] + (dirRight * x + dirDown * y);
            Urho3D::Vector3 normal = pos.Normalized();

            pos = normal * float(m_icoTree->m_radius);

            // Position and normal
            Urho3D::Vector3 vertM[2] = {pos, normal};

            if (gpuVertChunk)
            {
                // Copy vertex data to shadow buffer, then set min/max update
                // range for sending to the gpu later
                // Intended to reduce buffer update calls to the gpu

                memcpy(vertDataChunk + vertIndex * vertSizeChunk,
                       vertM, vertSizeChunk);

                //gpuVertChunk->m_start =
                //      Min(gpuVertChunk->m_start, vertSizeChunk);
                //gpuVertChunk->m_end =
                //      Max(gpuVertChunk->m_start, (tri->m_midVerts[i] + 1));
            }
            else
            {
                // Update gpu buffers right away
                m_chunkVertBuf->SetDataRange(vertM, vertIndex, 1);
            }


            indices[localIndex] = vertIndex;
            i ++;
        }
    }

    // The data that will be pushed directly into the chunk index buffer
    // * 3 because there are 3 indices in a triangle
    Urho3D::PODVector<unsigned> chunkIndData(m_chunkSizeInd * 3);

    i = 0;
    // indices array is now populated, connect the dots!
    for (int y = 0; y < int(m_chunkVertsPerSide); y ++)
    {
        for (int x = 0; x < y * 2 + 1; x ++)
        {
            // alternate between true and false
            if (x % 2)
            {
                // upside down triangle
                // top, left, right
                chunkIndData[i + 0]
                        = indices[get_index_ringed(x / 2 + 1, y + 1)];
                chunkIndData[i + 1]
                        = indices[get_index_ringed(x / 2 + 1, y)];
                chunkIndData[i + 2]
                        = indices[get_index_ringed(x / 2, y)];
            }
            else
            {
                // up pointing triangle
                // top, left, right
                chunkIndData[i + 0]
                        = indices[get_index_ringed(x / 2, y)];
                chunkIndData[i + 1]
                        = indices[get_index_ringed(x / 2, y + 1)];
                chunkIndData[i + 2]
                        = indices[get_index_ringed(x / 2 + 1, y + 1)];

                //URHO3D_LOGINFOF("Triangle: %u %u %u", chunkIndData[i + 0],
                //chunkIndData[i + 1], chunkIndData[i + 2]);
            }
            //URHO3D_LOGINFOF("I: %i", i / 3);
            i += 3;
        }
    }

    // Keep track of which part of the index buffer refers to which triangle
    m_chunkIndDomain[m_chunkCount] = t;

    // Put the index data at the end of the buffer
    tri->m_chunkIndex = m_chunkCount * chunkIndData.Size();
    m_indBufChunk->SetDataRange(chunkIndData.Buffer(), tri->m_chunkIndex,
                                m_chunkSizeInd * 3);

    m_chunkCount ++;

    m_geometryChunk->SetDrawRange(Urho3D::TRIANGLE_LIST, 0,
                                  m_chunkCount * chunkIndData.Size());

    // The triangle is now chunked
    tri->m_bitmask ^= gc_triangleMaskChunked;

}

void PlanetWrenderer::chunk_remove(trindex t, UpdateRange* gpuVertChunk,
                                   UpdateRange* gpuVertInd)
{
    SubTriangle* tri = m_icoTree->get_triangle(t);

    if (!bool(tri->m_bitmask & gc_triangleMaskChunked))
    {
        // If not chunked
        return;
    }

    // Reduce chunk count. now m_chunkCount is equal to the chindex of the
    // last chunk
    m_chunkCount --;

    //URHO3D_LOGINFOF("Chunk being deleted: %i", tri->m_chunk);

    // Delete Indices, same method in set_visible
    // (maybe optimize this later by filling the empty spaces after all the
    // chunks have been processed)

    // The last triangle in the buffer
    SubTriangle* lastTriangle =
            m_icoTree->get_triangle(m_chunkIndDomain[m_chunkCount]);

    // Get positions in index buffer
    unsigned* lastTriIndData = reinterpret_cast<unsigned*>(
                                m_indBufChunk->GetShadowData())
                                + lastTriangle->m_chunkIndex;

    // Replace tri's domain location with lastTriangle
    m_chunkIndDomain[tri->m_chunk] = m_chunkIndDomain[m_chunkCount];

    // Change lastTriangle's chunk index to tri's
    lastTriangle->m_chunkIndex = tri->m_chunkIndex;
    lastTriangle->m_chunk = tri->m_chunk;


    // Delete Verticies

    // Mark middle vertices for replacement
    m_chunkVertFree.Push(tri->m_chunkVerts);

    // Now delete shared vertices

    unsigned* triIndData = reinterpret_cast<unsigned*>(
                            m_indBufChunk->GetShadowData())
                            + tri->m_chunkIndex;

    for (unsigned i = 0; i < m_chunkSharedCount; i ++)
    {
        buindex sharedIndex = *(triIndData + m_chunkSharedIndices[i]);

        // Decrease number of users
        m_chunkVertUsers[sharedIndex] --;

        if (m_chunkVertUsers[sharedIndex] == 0)
        {
            // If users is zero, then delete
            m_chunkVertFreeShared.Push(sharedIndex);
            m_chunkVertCountShared --;
        }
    }


    // Setting index data

    // Move lastTri's index data to replace tri's data
    m_indBufChunk->SetDataRange(lastTriIndData, tri->m_chunkIndex,
                                m_chunkSizeInd * 3);

    // Update draw range
    m_geometryChunk->SetDrawRange(Urho3D::TRIANGLE_LIST, 0,
                                  m_chunkCount * m_chunkSizeInd * 3);

    // Set chunked bit
    tri->m_bitmask ^= gc_triangleMaskChunked;
}

uint64_t PlanetWrenderer::get_memory_usage() const
{
    uint64_t total = sizeof(PlanetWrenderer);
    if (m_ready)
    {
        //total += m_preview->GetIndexCount() * m_preview->GetIndexSize();
        //total += m_indBuf->GetIndexCount() * m_indBuf->GetIndexSize();
        total += m_indBufChunk->GetIndexCount()
                    * m_indBufChunk->GetIndexSize();

        //total += m_vertBuf->GetVertexCount()
        //            * m_vertBuf->GetVertexSize();

        total += m_chunkVertBuf->GetVertexCount()
                    * m_chunkVertBuf->GetVertexSize();

        //total += m_indDomain.Capacity() * sizeof(trindex);
        //total += m_triangles.Capacity() * sizeof(SubTriangle);
        //total += m_trianglesFree.Capacity() * sizeof(trindex);
        //total += m_vertFree.Capacity() * sizeof(buindex);
        //total += m_chunkFree.Capacity() * sizeof(buindex);
        total += m_chunkIndDomain.Capacity() * sizeof(trindex);
        total += m_chunkVertFreeShared.Capacity() * sizeof(buindex);
    }
    return total;
}

void PlanetWrenderer::log_stats() const
{
    // Spaghetti print some useful information into the console
    URHO3D_LOGINFOF("\nIcoSphereTree Info:\n"
            " - Vertices:     [%u/%u, %u free]\n"
            " - Triangles:    [%u/%u, %u free]\n"
            "Chunk Info\n"
            " - Chunks:       [%u/%u, %u free]\n"
            " - Shared Vert:  [%u/%u, %u free]\n"
            " - Total Vert:   [%u/%u]",
            m_icoTree->m_vertCount, m_icoTree->m_maxVertice, m_icoTree->m_vertFree.Size(),
            m_icoTree->m_triangles.Size(), m_icoTree->m_maxTriangles, m_icoTree->m_trianglesFree.Size(),
            m_chunkCount, m_maxChunks, m_chunkVertFree.Size(),
            m_chunkVertCountShared, m_chunkMaxVertShared, m_chunkVertFreeShared.Size(),
            m_chunkVertCountShared + m_chunkCount * m_chunkSize, m_chunkMaxVert);
}

} // namespace osp
