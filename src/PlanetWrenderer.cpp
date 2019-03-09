// "PlanetRenderer is a little too boring" -- Capital Asterisk, 2018
#include "OSP.h"


inline void PlanetWrenderer::set_neighbours(SubTriangle& tri, trindex bot, trindex rte, trindex lft)
{
    tri.m_neighbours[0] = bot;
    tri.m_neighbours[1] = rte;
    tri.m_neighbours[2] = lft;
}

inline void PlanetWrenderer::set_verts(SubTriangle& tri, trindex top, trindex lft, trindex rte)
{
    tri.m_corners[0] = top;
    tri.m_corners[1] = lft;
    tri.m_corners[2] = rte;
}

inline const uint8_t PlanetWrenderer::neighboor_index(SubTriangle& tri, trindex lookingFor)
{
    // Loop through neighbours on the edges. child 4 (center) is not considered as all it's neighbours are its siblings
    for (int i = 0; i < 3; i ++)
    {
        if (tri.m_neighbours[i] == lookingFor)
        {
            return i;
        }
    }
    // this means there's an error
    assert(0);
    return 255;
}

PlanetWrenderer::PlanetWrenderer() : m_triangles(), m_trianglesFree(), m_vertFree(), m_chunkVertFreeShared()
{
    // Numbers that work nicely, though not ideal

    m_chunkAreaThreshold = 0.04f;
    m_subdivAreaThreshold = 0.02f;

    m_maxTriangles = 480000;

    m_maxIndices = 480000;
    m_maxVertice = 1600000;

    m_maxDepth = 5;
    m_previewDepth = 3;

    m_chunkResolution = 17;

    m_maxChunks = 300;
    m_chunkMaxVertShared = 10000;

}

PlanetWrenderer::~PlanetWrenderer()
{
    //m_vertBuf->Release();
    //m_indBuf->Release();
    //printf("pppp %p;\n", m_indDomain);
    //delete[] m_indDomain;
}

void PlanetWrenderer::initialize(Context* context, Image* heightMap, double size) {

    m_radius = size;
    m_heightMap = heightMap;

    // calculate proper numbers later, use magic numbers for now

    m_model = new Model(context);
    m_model->SetNumGeometries(2);
    m_model->SetBoundingBox(BoundingBox(Sphere(Vector3(0, 0, 0), size * 2)));

    {

        // Pentagon stuff, from wolfram alpha
        // This part is kind of messy and should be revised
        // "X pointing" pentagon goes on the top

        float s = size / 280.43394944265;
        float h = 114.486680448;

        float ca = 79.108350559987f;
        float cb = 207.10835055999f;
        float sa = 243.47046817156f;
        float sb = 150.47302458687f;

        float* vertInit = new float[m_maxVertice * 6];

        // VERT XYZ, NORMAL XYZ, VERT XYZ, NORMAL XYZ, ...
        // Set initial data for the normal icosahedron
        // There should be a better way to do this
        vertInit[0] = 0; // Top vertex 0
        vertInit[1] = 280.43394944265;
        vertInit[2] = 0;
        vertInit[6] = 280.43394944265; // Pentagon top aligned point 1
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
        m_indCount = 0; // Start with 0 faces, set_visible increases this

        // Shape into the right sized sphere
        double vx, vy, vz;
        for (int i = 0; i < 68; i += 6)
        {

            vx = vertInit[i + 0];
            vy = vertInit[i + 1];
            vz = vertInit[i + 2];
            double mag = Sqrt(vx * vx
                              + vy * vy
                              + vz * vz);
            vertInit[i + 0] = float(vx / mag * size);
            vertInit[i + 1] = float(vy / mag * size);
            vertInit[i + 2] = float(vz / mag * size);
            vertInit[i + 3] = float(vx / mag);
            vertInit[i + 4] = float(vy / mag);
            vertInit[i + 5] = float(vz / mag);
        }

        // Urho3D specific, make buffers and stuff
        m_indBuf = new IndexBuffer(context);
        m_vertBuf = new VertexBuffer(context);
        m_geometry = new Geometry(context);

        // Allocate index domain, some space on empty triangles array
        m_indDomain.Resize(m_maxIndices * 3);
        m_triangles.Reserve(3000);

        // This part is instuctions saying that position and normal data is interleved
        PODVector<VertexElement> elements;
        elements.Push(VertexElement(TYPE_VECTOR3, SEM_POSITION));
        elements.Push(VertexElement(TYPE_VECTOR3, SEM_NORMAL));

        // Set size and data of vertex data
        m_vertBuf->SetSize(m_maxVertice, elements);
        m_vertBuf->SetShadowed(true);
        m_vertBuf->SetData(vertInit); // This line causes random sigsegvs sometimes, TODO: investigate

        // Same but with index buffer
        m_indBuf->SetSize(m_maxIndices * 3, true, true);
        //indBuf_->SetData();
        m_indBuf->SetShadowed(true);

        // Create the geometry, urho3d specific
        m_geometry->SetNumVertexBuffers(1);
        m_geometry->SetVertexBuffer(0, m_vertBuf);
        m_geometry->SetIndexBuffer(m_indBuf);
        m_geometry->SetDrawRange(TRIANGLE_LIST, 0, sc_icosahedronFaceCount * 3);

        // Add geometry to model, urho3d specific
        m_model->SetGeometry(0, 0, m_geometry);

        // Initialize triangles, indices from sc_icoTemplateTris
        for (int i = 0; i < sc_icosahedronFaceCount; i ++)
        {
            // Set trianglesz
            SubTriangle tri;
            //printf("Triangle: %p\n", t);
            //tri.m_parent = 0;
            set_verts(tri, sc_icoTemplateTris[i * 3 + 0], sc_icoTemplateTris[i * 3 + 1], sc_icoTemplateTris[i * 3 + 2]);
            set_neighbours(tri, sc_icoTemplateneighbours[i * 3 + 0], sc_icoTemplateneighbours[i * 3 + 1], sc_icoTemplateneighbours[i * 3 + 2]);
            tri.m_bitmask = 0;
            tri.m_depth = 0;
            calculate_center(tri);
            m_triangles.Push(tri);
            //if (i != 0)
            //set_visible(i, true);
        }

        // Not leaking memory
        delete[] vertInit;
    }

    // Preview model
    {

        UpdateRange vertRange, indRange;
        // Generate preview
        for (int i = 0; i < sc_icosahedronFaceCount; i ++)
        {
            subdivide_add(i, &vertRange, &indRange);
            //URHO3D_LOGINFOF("Vert update: %u, %u", vertRange.m_start, vertRange.m_end);
            //URHO3D_LOGINFOF("Indx update: %u, %u", indRange.m_start, indRange.m_end);
        }

        // update to GPU
        if (vertRange.m_start < vertRange.m_end)
        {
            m_vertBuf->SetDataRange(m_vertBuf->GetShadowData() + vertRange.m_start * m_vertBuf->GetVertexSize(), vertRange.m_start, vertRange.m_end - vertRange.m_start);
        }
        if (indRange.m_start < indRange.m_end)
        {
            m_indBuf->SetDataRange(m_indBuf->GetShadowData() + indRange.m_start * m_indBuf->GetIndexSize(), indRange.m_start, indRange.m_end - indRange.m_start);
        }

        m_geometry->SetDrawRange(TRIANGLE_LIST, 0, m_indCount * 3);

        // unused for now
        m_preview = new IndexBuffer(context);
    }

    // Chunks
    {
        m_chunkCount = 0;

        m_chunkSize = m_chunkResolution * (m_chunkResolution + 1) / 2;

        // This is how many triangles is in a chunk
        m_chunkSizeInd = Pow(m_chunkResolution - 1, 2u);
        m_chunkSharedCount = (m_chunkResolution - 1) * 3;


        m_chunkMaxVert = m_chunkMaxVertShared + m_maxChunks * (m_chunkSize - m_chunkSharedCount);

        m_chunkVertCountShared = 0;

        // Initialize objects for dealing with chunks
        m_indBufChunk = new IndexBuffer(context);
        m_chunkVertBuf = new VertexBuffer(context);
        m_geometryChunk = new Geometry(context);
        m_chunkIndDomain.Resize(m_maxChunks);
        m_chunkVertUsers.Resize(m_chunkMaxVertShared);

        // Say that each vertex has position, normal, and tangent data
        PODVector<VertexElement> elements;
        elements.Push(VertexElement(TYPE_VECTOR3, SEM_POSITION));
        elements.Push(VertexElement(TYPE_VECTOR3, SEM_NORMAL));
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
        m_geometryChunk->SetDrawRange(TRIANGLE_LIST, 0, sc_icosahedronFaceCount * 3);

        // Add geometry to model, urho3d specific
        m_model->SetGeometry(1, 0, m_geometryChunk);

        // Calculate m_chunkSharedIndices;
        // use:

        // 0
        // 1  2
        // 3  4  5
        // 6  7  8  9

        // Chunk index data is stored as an array of (top, left right):
        // { 0, 1, 2,  1, 3, 4,  4, 2, 1,  2, 4, 5,  3, 6, 7, ... }
        // Each chunk has the same number of triangles, and are equally spaced in the buffer.
        // There are duplicates of the same index

        // Vertices in the edges of the chunk are considered shared vertices.
        // (all of the vertices except for 4)
        // They are added and removed in an unspecified order.
        // Their reserved space in the vertex buffer is [0 - m_chunkSharedCount]

        // Vertices in the middle are only used by one chunk
        // (only 4), at larger sizes, they outnumber the shared ones
        // They are equally spaced in the vertex buffer
        // Their reserved space in the vertex buffer is [m_chunkSharedCount - max]

        // It would be convinient if shared vertices can be accessed as a ring
        // around the edge of the triangle (See get_index_ringed).

        // so m_chunkSharedIndices is filled with indices to indices
        // ex: indexToSharedVertex = tri->m_chunkIndex + m_chunkSharedIndices[0];

        // An alterative to this would be storing a list of shared vertices per chunk.
        // this takes more space

        m_chunkSharedIndices.Resize(m_chunkSharedCount);

        unsigned i = 0;
        for (int y = 0; y < m_chunkResolution - 1; y ++)
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
                             m_chunkSharedIndices[localIndex] = i + j;
                        }

                    }
                }
                i += 3;
            }
        }

        // For Debugging
        //for (int i = 0; i < m_chunkSharedCount; i ++)
        //{
        //    URHO3D_LOGINFOF("Index: %i", m_chunkSharedIndices[i]);
        //}

    }

    // Not sure what this is doing, urho3d specific
    Vector<SharedPtr<VertexBuffer> > vrtBufs;
    Vector<SharedPtr<IndexBuffer> > indBufs;
    vrtBufs.Push(m_vertBuf);
    vrtBufs.Push(m_chunkVertBuf);
    indBufs.Push(m_indBuf);
    indBufs.Push(m_indBufChunk);
    PODVector<unsigned> morphRangeStarts;
    PODVector<unsigned> morphRangeCounts;
    morphRangeStarts.Push(0);
    morphRangeCounts.Push(0);
    m_model->SetVertexBuffers(vrtBufs, morphRangeStarts, morphRangeCounts);
    m_model->SetIndexBuffers(indBufs);

    m_ready = true;
}

void PlanetWrenderer::set_visible(trindex t, bool visible, UpdateRange* gpuInd)
{
    //printf("Setting visible: %u\n", t);
    SubTriangle* tri = get_triangle(t);

    // Don't even touch subdiveded triangles, would lead to problems
    if (tri->m_bitmask & E_SUBDIVIDED)
    {
        return;
    }

    // Check if the triangle is already visible/invisible
    if (bool(tri->m_bitmask & E_VISIBLE) == visible)
    {
        return;
    }

    if (visible)
    {

        // Put new vertex indices into the end of the index buffer

        // indDomain keeps track of which vertices belong to what triangle
        m_indDomain[m_indCount] = t;
        tri->m_index = m_indCount * 3;

        // Put data into indBuf
        if (gpuInd)
        {
            unsigned* indData = (unsigned*)(m_indBuf->GetShadowData()) + tri->m_index;
            indData[0] = tri->m_corners[0];
            indData[1] = tri->m_corners[1];
            indData[2] = tri->m_corners[2];

            gpuInd->m_start = Min(gpuInd->m_start, tri->m_index);
            gpuInd->m_end = Max(gpuInd->m_end, tri->m_index + 3);
        }
        else
        {
            unsigned xz[3];
            xz[0] = tri->m_corners[0];
            xz[1] = tri->m_corners[1];
            xz[2] = tri->m_corners[2];
            m_indBuf->SetDataRange(&xz, tri->m_index, 3);
        }
        //printf("Showing: %u %u %u \n", xz[0], xz[1], xz[2]);

        // Increment m_indCount as a new element was added to the end
        m_indCount ++;


    } else {

        //console.log("removed!");
        // How to remove a triangle from the buffer:
        // Move the last element of the buffer (3 ints) into the location of the triangle
        // that is suppose o be this. removed keeps holes out of the index buffer so
        // the gpu doesn't have to deal with holes, and holes dont have to be seeked out
        // for a new triangle to be inserted in.

        // decrement the index count, this is now the index of the last triangle
        m_indCount --;
        // change the last triangle class's index to new location.
        //console.log(this.indexDomain[this.indexCount]);
        m_triangles[m_indDomain[m_indCount]].m_index = tri->m_index;
        // move the location of the last element's domain to the new location
        m_indDomain[tri->m_index / 3] = m_indDomain[m_indCount];

        // get the index buffer data of the last triangle (last 3 ints), and put it into the new location

        unsigned* xz = reinterpret_cast<unsigned*>(m_indBuf->GetShadowData() + (m_indCount * 3 * sizeof(unsigned)));
        //printf("Hiding: %u, %u, %u\n", xz[0], xz[1], xz[2]);
        if (gpuInd)
        {

        }
        else
        {
            m_indBuf->SetDataRange(xz, tri->m_index, 3);
        }
        // indicates that tri is now invisible

    }

    if (gpuInd)
    {

    }
    else
    {
        m_geometry->SetDrawRange(TRIANGLE_LIST, 0, m_indCount * 3);
    }

    // toggle visibility
    tri->m_bitmask ^= E_VISIBLE;
}

void PlanetWrenderer::subdivide_add(trindex t, UpdateRange* gpuVert, UpdateRange* gpuInd)
{
    // if bottom triangle is deeper, use that vertex
    // same with left and right

    SubTriangle* tri = get_triangle(t);

    // if visible, then hide
    set_visible(t, false);

    // Remove chunks if currently chunked
    chunk_remove(t);

    // Add the 4 new triangles
    // Top Left Right Center
    unsigned freeSize = m_trianglesFree.Size();
    if (freeSize == 0) {
        // Make new triangles
        tri->m_children = m_triangles.Size();
        m_triangles.Resize(tri->m_children + 4);

        // Reassign pointer in case of reallocation
        tri = get_triangle(t);
        //tri->children[1] = freeSize + 1;
        //tri->children[2] = freeSize + 2;
        //tri->children[3] = freeSize + 3;
    } else {
        // Free triangles always come in groups of 4
        // as triangles are always deleted in groups of 4
        // pop doesn't return the value for some reason, get last element then pop
        trindex j = m_trianglesFree[m_trianglesFree.Size() - 1];
        m_trianglesFree.Pop();
        tri->m_children = j;
        //tri->children[1] = j + 1;
        //tri->children[2] = j + 2;
        //tri->children[3] = j + 3;
    }

    // Most of these constructors are useless, ignote comments below
    // For example
    // [tri.children[3], tri.sides[1], tri.sides[2]] means that the top triangle has the center triangle on it's bottom side, and it's left and right side are the same as the parent triangle
    // [tri.verts[0], tri.midVerts[2], tri.midVerts[1] means that the top vertex of the parent triangle, and the left and right middle vertices make up the top triangle
    //this.triangles[tri.children[0]] = new this.Triangle([-1, -1, -1], [-1, -1, -1], tri.myDepth + 1);
    //this.triangles[tri.children[1]] = new this.Triangle([-1, -1, -1], [-1, -1, -1], tri.myDepth + 1);
    //this.triangles[tri.children[2]] = new this.Triangle([-1, -1, -1], [-1, -1, -1], tri.myDepth + 1);
    SubTriangle* children = get_triangle(tri->m_children);

    set_neighbours(children[0], tri->m_children + 3, tri->m_neighbours[1], tri->m_neighbours[2]);
    set_neighbours(children[1], tri->m_neighbours[0], tri->m_children + 3, tri->m_neighbours[2]);
    set_neighbours(children[2], tri->m_neighbours[0], tri->m_neighbours[1], tri->m_children + 3);
    set_neighbours(children[3], tri->m_children + 0, tri->m_children + 1, tri->m_children + 2);
    children[0].m_depth = children[1].m_depth = children[2].m_depth = children[3].m_depth = tri->m_depth + 1;
    children[0].m_bitmask = children[1].m_bitmask = children[2].m_bitmask = children[3].m_bitmask = 0;
    // Subdivide lines and add verticies, or take from other triangles

    // Preparation to write to vertex buffer
    unsigned char* vertData = m_vertBuf->GetShadowData();
    unsigned vertSize = m_vertBuf->GetVertexSize();
    //float writeMe[3];
    //printf("Vertex size: %u\n", vertSize);

    // Loop through 3 sides of the triangle: Bottom, Right, Left
    // tri.sides refers to an index of another triangle on that side
    for (int i = 0; i < 3; i ++) {
        SubTriangle* triB = get_triangle(tri->m_neighbours[i]);
        // Check if the line is already subdivided, or if there is no triangle on the other side
        if (!(triB->m_bitmask & E_SUBDIVIDED) || (triB->m_depth != tri->m_depth)) {
            // A new vertex has to be created in the middle of the line
            if (m_vertFree.Size() == 0) {
                tri->m_midVerts[i] = m_vertCount;
                m_vertCount ++;
            } else {
                tri->m_midVerts[i] = m_vertFree[m_vertFree.Size() - 1];
                m_vertFree.Pop();
            }

            // Technique taken from an urho3D example
            //Vector3& vertM = (*reinterpret_cast<Vector3*>(&vertData[vertSize * tri.midVerts[i]]));
            const Vector3& vertA = (*reinterpret_cast<const Vector3*>(vertData + vertSize * tri->m_corners[(i + 1) % 3]));
            const Vector3& vertB = (*reinterpret_cast<const Vector3*>(vertData + vertSize * tri->m_corners[(i + 2) % 3]));

            Vector3 vertM[2];
            vertM[1] = ((vertA + vertB) / 2).Normalized();

            vertM[0] = vertM[1] * m_radius;
            // * (m_radius + (m_heightMap->GetPixelBilinear(Mod(Atan2(vertM[1].z_, vertM[1].x_) + 360.0f, 360.0f) / 360.0f, Acos(vertM[1].y_) / 180.0f).r_ * 100.0f));
            //printf("VA %s, VB: %s, \n", vertA.ToString().CString(), vertB.ToString().CString());

            //printf("DataRange: %u\n", tri->m_midVerts[i]);
            if (gpuVert)
            {
                // Set the min and max range the vertex buffer will be updated at the main thread
                memcpy(vertData + tri->m_midVerts[i] * vertSize, vertM, vertSize);

                gpuVert->m_start = Min(gpuVert->m_start, tri->m_midVerts[i]);
                gpuVert->m_end = Max(gpuVert->m_start, (tri->m_midVerts[i] + 1));
            }
            else
            {
                m_vertBuf->SetDataRange(vertM, tri->m_midVerts[i], 1);
            }

            //console.log(i + ": Created new vertex");
        } else {
            // Which side tri is on triB
            int sideB = neighboor_index(*triB, t);
            //printf("Vertex is being shared\n");
 
            // Instead of creating a new vertex, use the one from triB since it's already subdivided
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

            // Assign the face of each triangle to the other triangle right beside it
            m_triangles[triX].m_neighbours[i] = triBY;
            m_triangles[triY].m_neighbours[i] = triBX;
            //m_triangles[triBX].m_neighbours[sideB] = triY;
            //m_triangles[triBY].m_neighbours[sideB] = triX;
            set_side_recurse(m_triangles[triBX], sideB, triY);
            set_side_recurse(m_triangles[triBY], sideB, triX);

            //printf("Set Tri%u %u to %u", triBX)
        }

    }

    // Set verticies
    set_verts(children[0], tri->m_corners[0], tri->m_midVerts[2], tri->m_midVerts[1]);
    set_verts(children[1], tri->m_midVerts[2], tri->m_corners[1], tri->m_midVerts[0]);
    set_verts(children[2], tri->m_midVerts[1], tri->m_midVerts[0], tri->m_corners[2]);
    // The center triangle is made up of purely middle vertices.
    set_verts(children[3], tri->m_midVerts[0], tri->m_midVerts[1], tri->m_midVerts[2]);

    // Calculate centers
    calculate_center(children[0]);
    calculate_center(children[1]);
    calculate_center(children[2]);
    calculate_center(children[3]);

    tri->m_bitmask ^= E_SUBDIVIDED;

    // subdivide if below minimum depth
    if (tri->m_depth < m_previewDepth)
    {
        // Triangle vector might reallocate, so accessing tri after subdivide_add will cause undefiend behaviour
        //URHO3D_LOGINFOF("depth: %i tri: %i", tri->m_depth, t);
        trindex childs = tri->m_children;
        subdivide_add(childs + 0, gpuVert, gpuInd);
        subdivide_add(childs + 1, gpuVert, gpuInd);
        subdivide_add(childs + 2, gpuVert, gpuInd);
        subdivide_add(childs + 3, gpuVert, gpuInd);

    }
    else
    {
        set_visible(tri->m_children + 0, true, gpuInd);
        set_visible(tri->m_children + 1, true, gpuInd);
        set_visible(tri->m_children + 2, true, gpuInd);
        set_visible(tri->m_children + 3, true, gpuInd);
    }

}

void PlanetWrenderer::find_refs(SubTriangle& tri)
{
    //uint ohno = neighboor_index(tri, what);

    for (int i = 0; i < 3; i ++)
    {
        unsigned whateven = m_trianglesFree.IndexOf(int(tri.m_neighbours[i] / 4) * 4);
        if (whateven != m_trianglesFree.Size() )
        {
            printf("Deleted triangle %u referenced on side: %u\n", m_trianglesFree[whateven], i);
        }
    }


    if (tri.m_bitmask & E_SUBDIVIDED) {

        for (int i = 0; i < 4; i ++)
        {
            unsigned whateven = m_trianglesFree.IndexOf(tri.m_children + i);
            if (whateven != m_trianglesFree.Size() )
            {
                printf("Deleted triangle %u referenced on child: %u\n", m_trianglesFree[whateven], i);
            } else {
                find_refs(m_triangles[tri.m_children + i]);
            }
        }
    }
}

void PlanetWrenderer::subdivide_remove(trindex t, UpdateRange* gpuVert, UpdateRange* gpuInd)
{
    SubTriangle* tri = get_triangle(t);

    if (!(tri->m_bitmask & E_SUBDIVIDED))
    {
        // If not subdivided
        return;
    }

    // unsubdiv children if subdivided, and hide if hidden
    for (int i = 0; i < 4; i ++)
    {
        if (m_triangles[tri->m_children + i].m_bitmask & E_SUBDIVIDED) {
            subdivide_remove(tri->m_children + i);
        } else {
            //find_refs(m_triangles[tri->m_children + i]);
        }
        chunk_remove(tri->m_children + i);
        set_visible(tri->m_children + i, false);
    }


    // Loop through all sides but the middle
    for (int i = 0; i < 3; i ++)
    {
        SubTriangle* triB = get_triangle(tri->m_neighbours[i]);

        // If the triangle on the other side is not subdivided, it means that the vertex will have no more users
        if (!(triB->m_bitmask & E_SUBDIVIDED) || triB->m_depth != tri->m_depth) {
            // Mark side vertex for replacement
            m_vertFree.Push(tri->m_midVerts[i]);

        }
        // else leave it alone, the other triangle beside is still useing the vertex

        // Set neighbours, so that they don't reference deleted triangles
        if (triB->m_depth == tri->m_depth)
        {
            uint8_t sideB = neighboor_index(triB[0], t);
            set_side_recurse(triB[0], sideB, t);
        }
    }

    // Now mark triangles for removal. they're always in groups of 4.
    m_trianglesFree.Push(tri->m_children);
    //printf("Triangles Killed: %u - %u\n", tri->m_children, tri->m_children + 3);
    //this.triangles[tri.children[0]] = null;
    //this.triangles[tri.children[1]] = null;
    //this.triangles[tri.children[2]] = null;
    //this.triangles[tri.children[3]] = null;

    tri->m_bitmask ^= E_SUBDIVIDED;
    tri->m_children = -1;

    set_visible(t, true);

}

void PlanetWrenderer::calculate_center(SubTriangle &tri)
{
    const unsigned char* vertData = m_vertBuf->GetShadowData();
    const unsigned vertSize = m_vertBuf->GetVertexSize();
    const Vector3& vertA = (*reinterpret_cast<const Vector3*>(vertData + vertSize * tri.m_corners[0]));
    const Vector3& vertB = (*reinterpret_cast<const Vector3*>(vertData + vertSize * tri.m_corners[1]));
    const Vector3& vertC = (*reinterpret_cast<const Vector3*>(vertData + vertSize * tri.m_corners[2]));

    tri.m_center = (vertA + vertB + vertC) / 3.0f;
    //printf("Center: %s\n", tri.m_center.ToString().CString());
}

/**
 * Set a neighbour of a triangle, and apply for all of it's children's
 * @param tri [ref] Reference to triangle
 * @param side [in] Which side to set
 * @param to [in] Neighbour to operate on
 */
void PlanetWrenderer::set_side_recurse(SubTriangle& tri, uint8_t side, trindex to)
{
    tri.m_neighbours[side] = to;
    if (tri.m_bitmask & E_SUBDIVIDED) {
        set_side_recurse(m_triangles[tri.m_children + ((side + 1) % 3)], side, to);
        set_side_recurse(m_triangles[tri.m_children + ((side + 2) % 3)], side, to);
    }
}

void PlanetWrenderer::update(const Vector3& camera)
{
    m_camera = camera;
    m_cameraDist = camera.Length();

    // size/distance is equal to the dot product between the camera position vector, and the surface normal at the viewd edge of a perfect sphere
    // 0.45f is added because the triangles at the edge of the spehere are curved, and are still visible even when they point away from the camera.
    m_threshold = m_radius / m_cameraDist - 0.45f;

    //printf("Camera! %s\n", camera.ToString().CString());
    //printf("vert count: %ux\n", m_vertCount);
    // printf("Triangle count: %u Visible: %u\n", m_triangles.Size(), m_indCount);
    for (int i = 0; i < sc_icosahedronFaceCount; i ++) {
        sub_recurse(i);
    }

    //URHO3D_LOGINFOF("Memory Usage: %fMb", float(get_memory_usage()) / 1000000.0f);

    assert(m_indCount < m_maxIndices);
    assert(m_vertCount < m_maxVertice);
    assert(m_triangles.Size() < m_maxTriangles);
}

void PlanetWrenderer::sub_recurse(trindex t)
{
    SubTriangle* tri = get_triangle(t);

    bool shouldSubdivide, shouldChunk;

    //if (tri->m_depth < m_hqDepth)
    //{
    //  // Test to see if this part of the sphere is visible, using the face normal's dot product with the camera
    //  float dot = tri->m_center.DotProduct(m_camera) / (m_cameraDist * tri->m_center.Length());
    //  shouldSubdivide = (dot > m_threshold / tri->m_depth);
    //  shouldBeSubdivided = (dot > m_threshold + 0.2f * (float(tri->m_depth) + 3.0f));
    //} else {
    //    // Measure distance instead of using angles, again more temporary code
    //    shouldSubdivide = ((tri->m_center - m_camera).LengthSquared() < Pow(float(m_radius * 7.05), 2.0f));
    //}
    // Measure angle
    //printf("DOT: %f\n", dot);
    // calculate dot product thing
    //if (dot > m_threshold)
    //{
    //    subdivide(t);
    //}

    // Icosahedron edge length equations
    // let r = radius of circumscribed sphere
    // let a = edge length
    // from this equation: r = (a / 4) * sqrt(10 + 2 * sqrt(5))
    // arrange to this:    a = 4r / sqrt(10 + 2 * sqrt(5))
    // this equation now calculates edge length from radius
    // divide by 2^depth because the edge has been subdivided in powers of two

    // close enough approximation (should be a bit higher because it's spherical)
    float edgeLength = (4.0 * m_radius)
                       / Sqrt(10.0 + 2.0 * Sqrt(5.0))
                       / Pow(2, int(tri->m_depth));

    // Approximation of the triangle's area
    // (Area of equalateral triangle)
    float triArea = Sqrt(3) * edgeLength * edgeLength / 4;

    // Distance squared from viewer
    float distanceSquared = (tri->m_center - m_camera).LengthSquared();

    // How much space this triangle takes up on screen, using inverse square law
    // InverseSquareDistance * Area -> area / distancesquared
    // 0.2 is magic number to nicely fit things on screen
    float screenArea = triArea / (distanceSquared * 0.2f);

    // Maximum screen area a triangle can take before it's subdivided
    shouldSubdivide = screenArea > m_subdivAreaThreshold;

    // Same but for chunks
    shouldChunk = screenArea > m_chunkAreaThreshold;


    //chunk_add(t);

    // Check if already subdivided
    if (tri->m_bitmask & E_SUBDIVIDED)
    {

        if (shouldSubdivide)
        {
            if (tri->m_depth < m_maxDepth)
            {
                // triangle vector might reallocate making tri invalid, so keep a copy of m_children
                trindex childs = tri->m_children;
                sub_recurse(childs + 0);
                sub_recurse(childs + 1);
                sub_recurse(childs + 2);
                sub_recurse(childs + 3);
            }
        }
        else if (tri->m_depth > m_previewDepth)
        {
            subdivide_remove(t);
        }
    }
    else
    {
        if (shouldSubdivide)
        {
            if (tri->m_depth < m_maxDepth)
            {
                subdivide_add(t);
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
            set_visible(t, true);

        }
    }
}

inline const unsigned PlanetWrenderer::get_index(int x, int y) const
{
    return y * (y + 1) / 2 + x;
}

const unsigned PlanetWrenderer::get_index_ringed(int x, int y) const
{
    // || (x == y) ||
    if (y == m_chunkResolution - 1)
    {
        // Bottom edge
        return m_chunkResolution - x - 1;
    }
    else if (x == 0)
    {
        // Left edge
        return (m_chunkResolution - 1) * 2 - y;
    }
    else if (x == y)
    {
        // Right edge
        return (m_chunkResolution - 1) * 2 + y;
    }
    else
    {
        // Center
        return m_chunkSharedCount + get_index(x - 1, y - 2);
    }

}

void PlanetWrenderer::chunk_add(trindex t, UpdateRange* gpuVertChunk, UpdateRange* gpuVertInd)
{
    SubTriangle* tri = get_triangle(t);

    if (m_chunkCount >= m_maxChunks)
    {
        URHO3D_LOGERRORF("Chunk limit reached");
        return;
    }

    if (tri->m_bitmask & (E_CHUNKED | E_SUBDIVIDED))
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

    unsigned char* vertData = m_vertBuf->GetShadowData();
    unsigned vertSize = m_vertBuf->GetVertexSize();

    unsigned char* vertDataChunk = m_chunkVertBuf->GetShadowData();
    unsigned vertSizeChunk = m_chunkVertBuf->GetVertexSize();

    // top, left, right
    const Vector3 verts[3] = {
        (*reinterpret_cast<const Vector3*>(vertData + vertSize * tri->m_corners[0])),
        (*reinterpret_cast<const Vector3*>(vertData + vertSize * tri->m_corners[1])),
        (*reinterpret_cast<const Vector3*>(vertData + vertSize * tri->m_corners[2]))
    };

    const Vector3 dirRight = (verts[2] - verts[1]) / (m_chunkResolution - 1);
    const Vector3 dirDown = (verts[1] - verts[0]) / (m_chunkResolution - 1);

    // Loop through neighbours and see which ones are already chunked to share vertices with

    bool chunkedNeighbours[3];

    for (int i = 0; i < 3; i ++)
    {
        SubTriangle* triB = get_triangle(tri->m_neighbours[i]);
        chunkedNeighbours[i] = (triB->m_bitmask & E_CHUNKED);
    }

    //URHO3D_LOGINFOF("DirDown: %f %f %f", dirDown.x_, dirDown.y_, dirDown.z_);

    tri->m_chunk = m_chunkCount;

    if (m_chunkVertFree.Size() == 0) {
        tri->m_chunkVerts = m_chunkMaxVertShared + m_chunkCount * (m_chunkSize - m_chunkSharedCount);
    }
    else
    {
        tri->m_chunkVerts = m_chunkVertFree.Back();
        m_chunkVertFree.Pop();
    }

    PODVector<unsigned> indices(m_chunkSize);

    int middleIndex = 0;
    int i = 0;
    for (int y = 0; y < m_chunkResolution; y ++)
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
                if (chunkedNeighbours[0])
                {
                    // Bottom can be shared
                    //URHO3D_LOGINFO("Bottom is chunked");
                }
                else if (chunkedNeighbours[1])
                {
                    // Left can be shared
                    //URHO3D_LOGINFO("Left is chunked");
                }
                else if (chunkedNeighbours[2])
                {
                    // Right can be shared
                    //URHO3D_LOGINFO("Right is chunked");
                }
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

            Vector3 pos = verts[0] + (dirRight * x + dirDown * y);
            Vector3 normal = pos.Normalized();

            pos = normal * m_radius;
            // * (m_radius + (100.0f * m_heightMap->GetPixelBilinear(Mod(Atan2(normal.z_, normal.x_) + 360.0f, 360.0f) / 360.0f, Acos(normal.y_) / 180.0f).r_));

            // Position and normal
            Vector3 vertM[2] = {pos, normal};

            if (gpuVertChunk)
            {
                // Copy vertex data to shadow buffer, then set min/max update range for sending to the gpu later
                // Intended to reduce buffer update calls to the gpu

                memcpy(vertDataChunk + vertIndex * vertSizeChunk, vertM, vertSizeChunk);

                //gpuVertChunk->m_start = Min(gpuVertChunk->m_start, vertSizeChunk);
                //gpuVertChunk->m_end = Max(gpuVertChunk->m_start, (tri->m_midVerts[i] + 1));
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
    PODVector<unsigned> chunkIndData(m_chunkSizeInd * 3);

    i = 0;
    // indices array is now populated, connect the dots!
    for (int y = 0; y < m_chunkResolution - 1; y ++)
    {
        for (int x = 0; x < y * 2 + 1; x ++)
        {
            // alternate between true and false
            if (x % 2)
            {
                // upside down triangle
                // top, left, right
                chunkIndData[i + 0] = indices[get_index_ringed(x / 2 + 1, y + 1)];
                chunkIndData[i + 1] = indices[get_index_ringed(x / 2 + 1, y)];
                chunkIndData[i + 2] = indices[get_index_ringed(x / 2, y)];
            }
            else
            {
                // up pointing triangle
                // top, left, right
                chunkIndData[i + 0] = indices[get_index_ringed(x / 2, y)];
                chunkIndData[i + 1] = indices[get_index_ringed(x / 2, y + 1)];
                chunkIndData[i + 2] = indices[get_index_ringed(x / 2 + 1, y + 1)];

                //URHO3D_LOGINFOF("Triangle: %u %u %u", chunkIndData[i + 0], chunkIndData[i + 1], chunkIndData[i + 2]);
            }
            //URHO3D_LOGINFOF("I: %i", i / 3);
            i += 3;
        }
    }

    // Hide triangle
    set_visible(t, false);

    // Keep track of which part of the index buffer refers to which triangle
    m_chunkIndDomain[m_chunkCount] = t;

    // Put the index data at the end of the buffer
    tri->m_chunkIndex = m_chunkCount * chunkIndData.Size();
    m_indBufChunk->SetDataRange(chunkIndData.Buffer(), tri->m_chunkIndex, m_chunkSizeInd * 3);

    m_chunkCount ++;

    m_geometryChunk->SetDrawRange(TRIANGLE_LIST, 0, m_chunkCount * chunkIndData.Size());

    // The triangle is now chunked
    tri->m_bitmask ^= E_CHUNKED;

}


void PlanetWrenderer::chunk_remove(trindex t, UpdateRange* gpuVertChunk, UpdateRange* gpuVertInd)
{
    SubTriangle* tri = get_triangle(t);

    if (!bool(tri->m_bitmask & E_CHUNKED))
    {
        // If not chunked
        return;
    }

    // Reduce chunk count. now m_chunkCount is equal to the chindex of the last chunk
    m_chunkCount --;

    //URHO3D_LOGINFOF("Chunk being deleted: %i", tri->m_chunk);

    // Delete Indices, same method in set_visible
    // (maybe optimize this later by filling the empty spaces after all the chunks have been processed)

    // The last triangle in the buffer
    SubTriangle* lastTriangle = get_triangle(m_chunkIndDomain[m_chunkCount]);

    // Get positions in index buffer
    unsigned* lastTriIndData = reinterpret_cast<unsigned*>(m_indBufChunk->GetShadowData()) + lastTriangle->m_chunkIndex;

    // Replace tri's domain location with lastTriangle
    m_chunkIndDomain[tri->m_chunk] = m_chunkIndDomain[m_chunkCount];

    // Change lastTriangle's chunk index to tri's
    lastTriangle->m_chunkIndex = tri->m_chunkIndex;
    lastTriangle->m_chunk = tri->m_chunk;


    // Delete Verticies

    // Mark middle vertices for replacement
    m_chunkVertFree.Push(tri->m_chunkVerts);

    // Now delete shared vertices

    unsigned* triIndData = reinterpret_cast<unsigned*>(m_indBufChunk->GetShadowData()) + tri->m_chunkIndex;

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
    m_indBufChunk->SetDataRange(lastTriIndData, tri->m_chunkIndex, m_chunkSizeInd * 3);

    // Update draw range
    m_geometryChunk->SetDrawRange(TRIANGLE_LIST, 0, m_chunkCount * m_chunkSizeInd * 3);

    // Set chunked bit
    tri->m_bitmask ^= E_CHUNKED;
}

const uint64_t PlanetWrenderer::get_memory_usage() const
{
    uint64_t total = sizeof(PlanetWrenderer);
    if (m_ready)
    {
        total += m_preview->GetIndexCount() * m_preview->GetIndexSize();
        total += m_indBuf->GetIndexCount() * m_indBuf->GetIndexSize();
        total += m_indBufChunk->GetIndexCount() * m_indBufChunk->GetIndexSize();

        total += m_vertBuf->GetVertexCount() * m_vertBuf->GetVertexSize();
        total += m_chunkVertBuf->GetVertexCount() * m_chunkVertBuf->GetVertexSize();

        total += m_indDomain.Capacity() * sizeof(trindex);
        total += m_triangles.Capacity() * sizeof(SubTriangle);
        total += m_trianglesFree.Capacity() * sizeof(trindex);
        total += m_vertFree.Capacity() * sizeof(buindex);
        //total += m_chunkFree.Capacity() * sizeof(buindex);
        total += m_chunkIndDomain.Capacity() * sizeof(trindex);
        total += m_chunkVertFreeShared.Capacity() * sizeof(buindex);
    }
    return total;
}
