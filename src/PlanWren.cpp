#include "OSP.h"

PlanWren::PlanWren() : m_triangles(), m_trianglesFree(), m_vertFree()
{
    m_indCount = 0;
}

PlanWren::~PlanWren()
{
    //m_vertBuf->Release();
    //m_indBuf->Release();
    printf("pppp %p;\n", m_indDomain);
    delete[] m_indDomain;
}

/**
* Initialize PlanWren, allocates buffers and calculates where the base verticies
* should be
*/
void PlanWren::initialize(Context* context, double size, Scene* scene, ResourceCache* cache) {

    // calculate proper numbers later
    m_maxFaces = 5000;
    m_maxIndices = 1000;
    m_maxVertice = 1000;

    float s = size / 280.43394944265;
    float h = 114.486680448;

    // Pentagon stuff, from wolfram alpha
    // "X pointing" pentagon goes on the top

    float ca = 79.108350559987f;
    float cb = 207.10835055999f;
    float sa = 243.47046817156f;
    float sb = 150.47302458687f;

    float* vertInit = new float[m_maxVertice * 6];

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

    m_indBuf = new IndexBuffer(context);
    m_vertBuf = new VertexBuffer(context);
    m_geometry = new Geometry(context);
    m_model = new Model(context);

    PODVector<VertexElement> elements;
    elements.Push(VertexElement(TYPE_VECTOR3, SEM_POSITION));
    elements.Push(VertexElement(TYPE_VECTOR3, SEM_NORMAL));

    m_vertBuf->SetSize(m_maxVertice * 6, elements);
    m_vertBuf->SetData(vertInit);
    m_vertBuf->SetShadowed(true);

    m_indBuf->SetSize(m_maxFaces * 3, true, true);
    //indBuf_->SetData();
    m_indBuf->SetShadowed(true);

    m_geometry->SetNumVertexBuffers(1);
    m_geometry->SetVertexBuffer(0, m_vertBuf);
    m_geometry->SetIndexBuffer(m_indBuf);
    m_geometry->SetDrawRange(TRIANGLE_LIST, 0, 20 * 3);

    m_model->SetNumGeometries(1);
    m_model->SetGeometry(0, 0, m_geometry);
    m_model->SetBoundingBox(BoundingBox(Sphere(Vector3(0, 0, 0), size * 2)));
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

    m_indDomain = new trindex[m_maxIndices * 3];
    m_triangles.Reserve(180);
    //m_triangles.Resize(20);

    for (int i = 0; i < 20; i ++) {
        // Set trianglesz
        SubTriangle tri;
        //printf("Triangle: %p\n", t);
        tri.m_parent = 0;
        tri.m_verts[0] = sc_icoTemplateTris[i * 3 + 0];
        tri.m_verts[1] = sc_icoTemplateTris[i * 3 + 1];
        tri.m_verts[2] = sc_icoTemplateTris[i * 3 + 2];
        tri.m_bitmask = 0;
        tri.m_depth = 1;
        m_triangles.Push(tri);
        set_visible(i, true);
    }

    delete[] vertInit;
}

void PlanWren::set_visible(trindex t, bool visible)
{
    printf("Setting visible: %u\n", t);
    SubTriangle* tri = get_triangle(t);
    if (visible)
    {
        if (!(tri->m_bitmask & TriangleStats::E_VISIBLE))
        {
            printf("yea\n");
            m_indDomain[m_indCount] = t;
            tri->m_index = m_indCount * 3;
            uint xz[3];
            xz[0] = tri->m_verts[0];
            xz[1] = tri->m_verts[1];
            xz[2] = tri->m_verts[2];
            m_indBuf->SetDataRange(&xz, tri->m_index, 3);
            m_indCount ++;
            tri->m_bitmask ^= TriangleStats::E_VISIBLE;
        }
    } else {



    }

}
