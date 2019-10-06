#include <Urho3D/IO/Log.h>

#include "ActiveArea.h"
#include "NodeSat.h"

namespace osp
{

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

} // namespace osp
