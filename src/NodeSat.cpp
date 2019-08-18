#include "ActiveArea.h"
#include "NodeSat.h"

using namespace osp;

NodeSat::NodeSat()
{
    m_name = "Probably a rocket";
}

NodeSat::~NodeSat()
{
    //Satellite::~Satellite();
}

Node* NodeSat::load(ActiveArea* area, const Vector3& pos)
{
    if (m_node.Null())
    {
        return nullptr;
    }

    // Move the node into the ActiveArea scene
    m_node->SetParent(area->GetNode());

    m_activeNode = m_node;

    return m_node.Get();
}

Node* NodeSat::load_preview(ActiveArea* area)
{

}

void NodeSat::unload()
{

}
