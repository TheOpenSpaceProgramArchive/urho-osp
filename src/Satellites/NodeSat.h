#pragma once

#include "Satellite.h"

namespace osp
{

/**
 * Keeps a SharedPtr of a Node, loaded as the active node when needed.
 */
class NodeSat : public Satellite
{

    URHO3D_OBJECT(NodeSat, Satellite)

public:

    NodeSat(Context* context);
    ~NodeSat() = default;

    Node* get_node()
    {
        return m_node.Get();
    }

    void set_node(Node* node)
    {
        m_node = node;
    }

    Node* load(ActiveArea* area, const Vector3& pos) override;
    Node* load_preview(ActiveArea* area) override;

    void unload() override;

protected:
    // Stored node that shouldn't deallocate
    SharedPtr<Node> m_node;
};

inline NodeSat::NodeSat(Context* context)
 : Satellite(context)
{
    m_name = "Probably a rocket";
}

inline Node* NodeSat::load_preview(ActiveArea* area)
{
    // TODO:
    return nullptr;
}

inline void NodeSat::unload()
{ }

} // namespace osp
