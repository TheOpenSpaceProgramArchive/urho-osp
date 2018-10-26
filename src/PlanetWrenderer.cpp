// "PlanetRenderer is a little too boring" -- Capital Asterisk, 2018
#include "OSP.h"

/**
 * @brief A quick way to set neighbours
 * @param tri Reference to triangle
 * @param bot Bottom
 * @param rte Right
 * @param lft Left
 */
inline void PlanetWrenderer::set_neighbours(SubTriangle& tri, trindex bot, trindex rte, trindex lft)
{
    tri.m_neighbours[0] = bot;
    tri.m_neighbours[1] = rte;
    tri.m_neighbours[2] = lft;
}

/**
 * @brief A quick way to set vertices
 * @param tri Reference to triangle
 * @param top Top
 * @param lft Left
 * @param rte Right
 */
inline void PlanetWrenderer::set_verts(SubTriangle& tri, trindex top, trindex lft, trindex rte)
{
    tri.m_verts[0] = top;
    tri.m_verts[1] = lft;
    tri.m_verts[2] = rte;
}

/**
 * @brief Find which side a triangle is on another triangle
 * @param tri Reference to triangle to be searched
 * @param lookingFor Index of triangle to search for
 * @return Neighbour index (0 - 2), or bottom, left, or right
 */
inline uint8_t PlanetWrenderer::neighboor_index(SubTriangle& tri, trindex lookingFor)
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

PlanetWrenderer::PlanetWrenderer() : m_triangles(), m_trianglesFree(), m_vertFree()
{
    // Chunk profile determines which triangles should be subdivided multiple times after a single subdivide call
    // as if the triangle can subdivide into 64 triangles instead of just 4
    // This is intended to reduce the number of distance checks
    m_chunkProfile.Push(0);
    m_chunkProfile.Push(0);
    m_chunkProfile.Push(0);
    m_chunkProfile.Push(0);
    m_chunkProfile.Push(0);
    m_chunkProfile.Push(0);
    m_chunkProfile.Push(0);
    m_chunkProfile.Push(0);
    m_chunkProfile.Push(0);
    m_chunkProfile.Push(0);
    m_indCount = 0;
    m_maxDepth = 6;
    m_hqDepth = 4;
}

PlanetWrenderer::~PlanetWrenderer()
{
    //m_vertBuf->Release();
    //m_indBuf->Release();
    //printf("pppp %p;\n", m_indDomain);
    delete[] m_indDomain;
}

/**
 * @brief PlanetWrenderer::initialize
 * @param context
 * @param size
 */
void PlanetWrenderer::initialize(Context* context, double size) {

    m_size = size;

    // calculate proper numbers later
    m_maxTriangles = 50000;
    m_maxIndices = 18000;
    m_maxVertice = 17000;

    m_vertCount = 12;
    m_indCount = 0;

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

    // Shape into the right sized sphere
    double vx, vy, vz;
    for (int i = 0; i < 68; i += 6) {

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
    m_model = new Model(context);

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
    m_geometry->SetDrawRange(TRIANGLE_LIST, 0, 20 * 3);

    // Add geometry to model, urho3d specific
    m_model->SetNumGeometries(1);
    m_model->SetGeometry(0, 0, m_geometry);
    m_model->SetBoundingBox(BoundingBox(Sphere(Vector3(0, 0, 0), size * 2)));

    // Not sure what this is doing, urho3d specific
    Vector<SharedPtr<VertexBuffer> > vrtBufs;
    Vector<SharedPtr<IndexBuffer> > indBufs;
    vrtBufs.Push(m_vertBuf);
    indBufs.Push(m_indBuf);
    PODVector<unsigned> morphRangeStarts;
    PODVector<unsigned> morphRangeCounts;
    morphRangeStarts.Push(0);
    morphRangeCounts.Push(0);
    m_model->SetVertexBuffers(vrtBufs, morphRangeStarts, morphRangeCounts);
    m_model->SetIndexBuffers(indBufs);

    //ready_ = true;

    // Create index domain, and reserve some space on empty triangles array
    m_indDomain = new trindex[m_maxIndices * 3];
    //m_triangles.Reserve(180);
    m_triangles.Reserve(3000);

    // Initialize triangles, indices from sc_icoTemplateTris
    for (int i = 0; i < sc_icosahedronFaceCount; i ++) {
        // Set trianglesz
        SubTriangle tri;
        //printf("Triangle: %p\n", t);
        //tri.m_parent = 0;
        set_verts(tri, sc_icoTemplateTris[i * 3 + 0], sc_icoTemplateTris[i * 3 + 1], sc_icoTemplateTris[i * 3 + 2]);
        set_neighbours(tri, sc_icoTemplateneighbours[i * 3 + 0], sc_icoTemplateneighbours[i * 3 + 1], sc_icoTemplateneighbours[i * 3 + 2]);
        tri.m_bitmask = 0;
        tri.m_depth = 1;
        calculate_center(tri);
        m_triangles.Push(tri);
        //if (i != 0)
        set_visible(i, true);
    }

    for (int i = 0; i < sc_icosahedronFaceCount; i ++) {
        subdivide(i);
    }

    //unsubdivide(1);
    //unsubdivide(0);
    //unsubdivide(5);
    //unsubdivide(7);
    //subdivide(5);
    //unsubdivide(25);
    //subdivide(26);
    //subdivide(21);

    //for (int i = 20; i < 20 + 20 * 4; i ++) {
    //    subdivide(i);
    //}

    // Not leaking memory
    delete[] vertInit;
}

/**
 * Show or hide a triangle.
 * @param t [in] Index to the triangle
 * @param visible [in] To hide or to show
 */
void PlanetWrenderer::set_visible(trindex t, bool visible)
{
    //printf("Setting visible: %u\n", t);
    SubTriangle* tri = get_triangle(t);

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
        unsigned xz[3];
        xz[0] = tri->m_verts[0];
        xz[1] = tri->m_verts[1];
        xz[2] = tri->m_verts[2];
        m_indBuf->SetDataRange(&xz, tri->m_index, 3);
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
        m_indBuf->SetDataRange(xz, tri->m_index, 3);
        //this.indexBuffer.set(this.indexBuffer.slice(this.indexCount * 3, this.indexCount * 3 + 3), tri.index);
        // indicates that tri is now invisible

    }

    m_geometry->SetDrawRange(TRIANGLE_LIST, 0, m_indCount * 3);

    // toggle visibility
    tri->m_bitmask ^= E_VISIBLE;
}

/**
 * @brief Subdivide a triangle into 4 more
 * @param Triangle to subdivide
 */
void PlanetWrenderer::subdivide(trindex t)
{
    // if bottom triangle is deeper, use that vertex
    // same with left and right

    SubTriangle* tri = get_triangle(t);

    // if visible, then hide
    set_visible(t, false);

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
    float writeMe[3];
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
            const Vector3& vertA = (*reinterpret_cast<const Vector3*>(vertData + vertSize * tri->m_verts[(i + 1) % 3]));
            const Vector3& vertB = (*reinterpret_cast<const Vector3*>(vertData + vertSize * tri->m_verts[(i + 2) % 3]));

            Vector3 vertM[2];
            vertM[1] = ((vertA + vertB) / 2).Normalized();
            vertM[0] = vertM[1] * m_size;
            //printf("VA %s, VB: %s, \n", vertA.ToString().CString(), vertB.ToString().CString());

            //printf("DataRange: %u\n", tri->m_midVerts[i]);
            m_vertBuf->SetDataRange(vertM, tri->m_midVerts[i], 1);

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
    set_verts(children[0], tri->m_verts[0], tri->m_midVerts[2], tri->m_midVerts[1]);
    set_verts(children[1], tri->m_midVerts[2], tri->m_verts[1], tri->m_midVerts[0]);
    set_verts(children[2], tri->m_midVerts[1], tri->m_midVerts[0], tri->m_verts[2]);
    // The center triangle is made up of purely middle vertices.
    set_verts(children[3], tri->m_midVerts[0], tri->m_midVerts[1], tri->m_midVerts[2]);

    // Calculate centers
    calculate_center(children[0]);
    calculate_center(children[1]);
    calculate_center(children[2]);
    calculate_center(children[3]);

    //printf("Midverts %u: (B%u, R%u, L%u)\n", t, tri->m_midVerts[0], tri->m_midVerts[1], tri->m_midVerts[2]);
    //printf("neighbours %u: (B%u, R%u, L%u)\n", t, tri->m_neighbours[0], tri->m_neighbours[1], tri->m_neighbours[2]);

    //console.log("MyDepth: " + tri.myDepth)
    //console.log("Sides: B" + tri.sides[0] + ", R" + tri.sides[1] + ", L" + t  ri.sides[2]);
    //console.log("MidVerts: B" + tri.midVerts[0] + ", R" + tri.midVerts[1] + ", L" + tri.midVerts[2]);

    tri->m_bitmask ^= E_SUBDIVIDED;

    // If the triangle is non-zero in the chunk profile, this means that it should be divided more
    if (m_chunkProfile[tri->m_depth - 1])
    {
        // in case a reallocation happens during a subdivide, this trindex doesn't change
        trindex childs = tri->m_children;
        subdivide(childs + 0);
        subdivide(childs + 1);
        subdivide(childs + 2);
        subdivide(childs + 3);
    } else {
        // Make them all visible

        //printf("Children to show: %u\n", tri->m_children);
        set_visible(tri->m_children + 0, true);
        set_visible(tri->m_children + 1, true);
        set_visible(tri->m_children + 2, true);
        set_visible(tri->m_children + 3, true);
    }


    //tri.myDepth ++;

    // Set subdivided bit

}

/**
 * @brief A debug function to search for use of deleted triangles
 * @param tri
 */
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

/**
 * @brief Unsubdivide a triangle. Removes children and sets neighbours of neighbours
 * @param t Index of triangle to unsubdivide
 */
void PlanetWrenderer::unsubdivide(trindex t)
{
    SubTriangle* tri = get_triangle(t);

    if (!(tri->m_bitmask & E_SUBDIVIDED))
    {
        return;
    }

    // unsubdiv children if subdivided, and hide if hidden
    for (int i = 0; i < 4; i ++)
    {
        if (m_triangles[tri->m_children + i].m_bitmask & E_SUBDIVIDED) {
            unsubdivide(tri->m_children + i);
        } else {
            //find_refs(m_triangles[tri->m_children + i]);
        }
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

/**
 * @brief Calculates and sets m_center
 * @param tri Reference to triangle
 */
void PlanetWrenderer::calculate_center(SubTriangle &tri)
{
    const unsigned char* vertData = m_vertBuf->GetShadowData();
    const unsigned vertSize = m_vertBuf->GetVertexSize();
    const Vector3& vertA = (*reinterpret_cast<const Vector3*>(vertData + vertSize * tri.m_verts[0]));
    const Vector3& vertB = (*reinterpret_cast<const Vector3*>(vertData + vertSize * tri.m_verts[1]));
    const Vector3& vertC = (*reinterpret_cast<const Vector3*>(vertData + vertSize * tri.m_verts[2]));

    tri.m_center = (vertA + vertB + vertC) / 3.0f;
    //printf("Center: %s\n", tri.m_center.ToString().CString());
}

/**
 * @brief Set a single neighbour of a triangle, and apply for all of it's children's
 * @param tri Reference to triangle
 * @param side Side to set
 * @param to Neighbour to set side to
 */
void PlanetWrenderer::set_side_recurse(SubTriangle& tri, uint8_t side, trindex to)
{
    tri.m_neighbours[side] = to;
    if (tri.m_bitmask & E_SUBDIVIDED) {
        set_side_recurse(m_triangles[tri.m_children + ((side + 1) % 3)], side, to);
        set_side_recurse(m_triangles[tri.m_children + ((side + 2) % 3)], side, to);
    }
}

/**
 * @brief Recalculates camera positiona and sub_recurses the main 20 triangles. Call this when the camera moves.
 * @param camera
 */
void PlanetWrenderer::update(const Vector3& camera)
{
    m_camera = camera;
    m_cameraDist = camera.Length();

    // size/distance is equal to the dot product between the camera position vector, and the surface normal at the viewd edge of a perfect sphere
    // 0.45f is added because the triangles at the edge of the spehere are curved, and are still visible even when they point away from the camera.
    m_threshold = m_size / m_cameraDist - 0.45f;

    //printf("Camera! %s\n", camera.ToString().CString());
    //printf("vert count: %ux\n", m_vertCount);
    // printf("Triangle count: %u Visible: %u\n", m_triangles.Size(), m_indCount);
    for (int i = 0; i < sc_icosahedronFaceCount; i ++) {
        sub_recurse(i);
    }

    assert(m_indCount < m_maxIndices);
    assert(m_vertCount < m_maxVertice);
    assert(m_triangles.Size() < m_maxTriangles);
}

/**
 * @brief Recursively check every triangle for which ones need (un)subdividing
 * @param t Triangle to start with
 */
void PlanetWrenderer::sub_recurse(trindex t)
{
    SubTriangle* tri = get_triangle(t);

    bool shouldSubdivide = false;

    if (tri->m_depth < m_hqDepth)
    {
        // Test to see if this part of the sphere is visible, using the face normal's dot product with the camera
        float dot = tri->m_center.DotProduct(m_camera) / (m_cameraDist * tri->m_center.Length());
        shouldSubdivide = (dot > m_threshold / tri->m_depth);
        //shouldBeSubdivided = (dot > m_threshold + 0.2f * (float(tri->m_depth) + 3.0f));
    } else {
        // Measure distance instead of using angles, again more temporary code
        shouldSubdivide = ((tri->m_center - m_camera).LengthSquared() < Pow(float(m_size * 7.05), 2.0f));
    }
    // Measure angle
    //printf("DOT: %f\n", dot);
    // calculate dot product thing
    //if (dot > m_threshold)
    //{
    //    subdivide(t);
    //}


    // If already subdivided
    if (tri->m_bitmask & E_SUBDIVIDED)
    {
        if (shouldSubdivide)
        {
            sub_recurse(tri->m_children + 0);
            sub_recurse(tri->m_children + 1);
            sub_recurse(tri->m_children + 2);
            sub_recurse(tri->m_children + 3);
        } else if (!bool(m_chunkProfile[tri->m_depth])) {
             unsubdivide(t);
        }
    } else {
        if (shouldSubdivide)
        {
            if (tri->m_depth < m_maxDepth)
            {
                subdivide(t);
            }
        }
    }

    //find_refs(*tri);

    //if (m_chunkProfile[tri->m_depth])
    //{
    //    sub_recurse(tri->m_children + 3);
    //}
}
