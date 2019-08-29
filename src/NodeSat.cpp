#include "ActiveArea.h"
#include "NodeSat.h"

using namespace osp;

NodeSat::NodeSat(Context* context) : Satellite(context)
{
    m_name = "Probably a rocket";
}

NodeSat::~NodeSat()
{
    //Satellite::~Satellite();
}

Node* NodeSat::load(ActiveArea* area, const Vector3& pos)
{
    Satellite::load(area, pos);

    if (m_node.Null())
    {
        return nullptr;
    }

    // Move the node into the ActiveArea scene
    m_node->SetParent(area->get_active_node());

    m_activeNode = m_node;

    return m_node.Get();
}

Node* NodeSat::load_preview(ActiveArea* area)
{

}

void NodeSat::unload()
{

}
