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
     * @param newChild [in] Pointer to Satellite to add as child
     */
    void add_child(Satellite* newChild);

    /**
     * @return Position relative to parent
     */
    LongVector3 get_position() const
    {
        return m_position;
    }

    /**
     * Set position relative to parent
     * @param pos [in] Desired value
     */
    inline void set_position(const LongVector3& pos)
    {
        m_position = pos;
    }


    inline uint64_t get_load_radius() const
    {
        return m_loadRadius;
    }

    /**
     * @return Pointer to the active node. Null if not loaded
     */
    inline Node* get_active_node() const
    {
        return m_activeNode.Get();
    }

    inline const Vector< UniquePtr<Satellite> >& get_children() const
    {
        return m_children;
    }

    inline const String& get_name() const
    {
        return m_name;
    }

    inline Satellite* get_parent() const
    {
        return m_parent;
    }

    inline unsigned get_depth() const
    {
        return m_depth;
    }

    inline unsigned get_index() const
    {
        return m_index;
    }

    /**
     * Try loading the satellite into the scene
     * Usually called when entering an ActiveArea (player gets close)
     * @param area [in] ActiveArea to load into
     * @param pos [in] Desired position in meters to load into.
     * @return Pointer to Active Node, null if load failed
     */
    virtual Node* load(ActiveArea* area, const Vector3& pos) = 0;

    /**
     * Similar to load, called when an object is visible from an ActiveArea,
     * but not close enough to be actively loaded into the scene.
     * Can be called multiple times from different ActiveAreas.
     *
     * The preview loaded should be something like the following:
     * * A bright billboard flare representing a distant object
     * * A dumb sphere textured to look like a planet without any fancy LOD
     *
     * @param area [in] ActiveArea to load into
     * @return Pointer to the newly created node in the ActiveArea's scene
     */
    virtual Node* load_preview(ActiveArea* area) = 0;

    /**
     * Unload the Satellite
     * Usually called when leaving an ActiveArea (player gets far enough)
     */
    virtual void unload() = 0;

protected:

    String m_name;

    // Position relative to parent
    LongVector3 m_position;

    // A sphere around this Satellite. When this intersects an ActiveArea's
    // sphere, then the ActiveArea will try to load this satellite.
    uint64_t m_loadRadius;

    // Position will be relative to this
    // Will be null for the root
    WeakPtr<Satellite> m_parent;

    // Pointers to Children that shouldn't spontaneously deallocate
    Vector< UniquePtr<Satellite> > m_children;
    
    // Index in parent's m_children
    unsigned m_index;

    // How deep this satellite is in the tree, or number of direct ancestors
    // Root Satellite will be 0
    unsigned m_depth = 0;

    // Associated node when loaded by an ActiveArea
    // Can only be loaded into one ActiveArea at a time
    WeakPtr<Node> m_activeNode;

    // Preview nodes beloning to many different ActiveAreas
    // Multiple ActiveAreas can see the same Satallite from a distance
    //Vector< WeakPtr<Node> > m_previews;

    // Scale for how many LongVector3 units is equal to a meter. This applies
    // to this Satellite's children. If this was 1000, then 1 = 1mm
    // smaller number means less precision, but larger
    unsigned m_unitsPerMeter = 1024;

    // orbital stuff goes here. something like:
    // OrbitType m_orbitType

};

}
