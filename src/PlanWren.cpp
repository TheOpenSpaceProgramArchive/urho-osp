#include "OSP.h"

/**
* Initialize PlanWren, allocates buffers and calculates where the base verticies
* should be
*/
void PlanWren::Initialize(Context* context, double size, Scene* scene, ResourceCache* cache) {

    // calculate proper numbers later
    m_maxFaces = 5000;
    m_maxIndices = 10000;
    m_maxVertice = 10000;

    m_model = new Model(context);

    float s = size / 280.43394944265;
    float h = 114.486680448;

    // Pentagon stuff, from wolfram alpha
    // "X pointing" pentagon goes on the top

    float ca = 79.108350559987f;
    float cb = 207.10835055999f;
    float sa = 243.47046817156f;
    float sb = 150.47302458687f;

    m_indBuf = new IndexBuffer(context);
    m_vertBuf = new VertexBuffer(context);
    m_geometry = new Geometry(context);

    float* vertInit = new float[m_maxVertice * 6];

    for(int i = 0; i < m_maxVertice * 6; i ++) {
        vertInit[i] = 0;
    }

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

    m_triangles.Reserve(180);

    for (int i = 0; i < 20; i++) {
        // Set trianglesz
    }

    PODVector<VertexElement> elements;
    elements.Push(VertexElement(TYPE_VECTOR3, SEM_POSITION));
    elements.Push(VertexElement(TYPE_VECTOR3, SEM_NORMAL));

    m_vertBuf->SetSize(m_maxVertice * 3, elements);
    m_vertBuf->SetData(vertInit);
    m_vertBuf->SetShadowed(true);

    m_indBuf->SetSize(m_maxVertice * 3, true, true);
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

    delete[] vertInit;
}
