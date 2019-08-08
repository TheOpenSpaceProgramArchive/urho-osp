#pragma once

#include <Urho3D/Scene/Node.h>
#include <Urho3D/Container/Ptr.h>
#include <Urho3D/Container/RefCounted.h>

#include "LongVector3.h"

using namespace Urho3D;

namespace osp {

class AstronomicalBody;
class ActiveArea;

/**
 * Base class for any physical object in the large universe
 */
class Satellite : public RefCounted
{

public:

    virtual ~Satellite();

    /**
     * Calculate the position of a Satellite relative to another Satellite
     * located anywhere
     * @param from [in] Reference frame
     * @param to [in] Satellite to calculate position for
     * @param unitsPerMeter [in] Desired precision of return value
     * @return Position of 'to' relative to 'from'
     */
    static LongVector3 calculate_relative_position(const Satellite* from,
                                                   const Satellite* to,
                                                       unsigned unitsPerMeter);
    
    /**
     * Add a child Satellite
     * @param newChild Pointer to Satellite to add as child
     */
    void add_child(Satellite* newChild);

    /**
     * Set position relative to parent
     * @param pos [in] Desired value
     */
    void set_position(const LongVector3& pos)
    {
        m_position = pos;
    }

    /**
     * @return Position relative to parent
     */
    LongVector3 get_position() const
    {
        return m_position;
    };

    /**
     * @return Pointer to the active node. Null if not loaded
     */
    Node* get_active_node() const
    {
        return m_activeNode.Get();
    };

    /**
     * Try loading the satellite into the scene
     * Usually called when entering an ActiveArea (player gets close)
     * @param area [in] ActiveArea to load into
     */
    virtual void load(ActiveArea* area) = 0;

    /**
     * Try loading the satellite into the scene
     * Usually called when leaving an ActiveArea (player gets far enough)
     * @param area [in] ActiveArea to load into
     */
    virtual void unload() = 0;

    /**
     * Discount version of load called when an object is far away, but visible.
     * a preview model of distant planet?
     * a light flare?
     * @param area [in] ActiveArea to load into
     */
    //virtual void load_preview(ActiveArea* area) const = 0;

protected:

    String m_name;

    // Position relative to parent
    LongVector3 m_position;

    // Position will be relative to this
    // Will be null for the root
    WeakPtr<Satellite> m_parent;

    // Pointers to Children that shouldn't spontaneously deallocate
    Vector< UniquePtr<Satellite> > m_children;
    
    // How deep this satellite is in the tree, or number of direct ancestors
    // Root Satellite will be 0
    unsigned m_depth = 0;

    // Associated node when loaded by an ActiveArea
    WeakPtr<Node> m_activeNode;

    // Scale for how many LongVector3 units is equal to a meter. This applies
    // to this Satellite's children. If this was 1000, then 1 = 1mm
    // smaller number means less precision, but larger
    unsigned m_unitsPerMeter = 1024;

    // orbital stuff goes here. something like:
    // OrbitType m_orbitType

};

}
