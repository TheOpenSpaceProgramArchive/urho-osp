#pragma once

#include "Satellite.h"


namespace osp {

/**
 * Keeps a SharedPtr of a Node, loaded as the active node when needed.
 */
class NodeSat : public Satellite
{

    URHO3D_OBJECT(NodeSat, Satellite)

public:

    NodeSat(Context* context);
    ~NodeSat() override;

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

}
