#pragma once

#include <Urho3D/Scene/Node.h>
#include <Urho3D/Container/Ptr.h>
#include <Urho3D/Container/RefCounted.h>

#include "../LongVector3.h"

using namespace Urho3D;

namespace osp {

class AstronomicalBody;
class ActiveArea;

/**
 * Base class for any physical object in the large universe
 */
class Satellite : public Object
{
    friend class ActiveArea;

    URHO3D_OBJECT(Satellite, Object)

public:

    Satellite(Context* context);

    virtual ~Satellite();

    /**
     * Calculate the position of a Satellite relative to another Satellite
     * located anywhere
     * @param from [in] Reference frame
     * @param to [in] Satellite to calculate position for
     * @param precision [in] Desired precision of return value
     * @return Position of 'to' relative to 'from'
     */
    static LongVector3 calculate_relative_position(const Satellite* from,
                                                   const Satellite* to,
                                                   int precision);
    
    /**
     * Add a child Satellite
     * @param newChild [in] Pointer to Satellite to add as child
     */
    void add_child(Satellite* newChild);

    /**
     * Returns current position. This only returns the previously stored value
     * for position, but does not calculate a new one. Consider calling
     * calculate_position() instead, to account for movement due to
     * Trajectory calculations or movement of the active node if loaded.
     *
     * @return Position relative to parent
     */
    LongVector3 get_position() const
    {
        return m_position;
    }

    /**
     * Set position relative to parent. This function is probably dangerous
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
     * Shorthand for checking if m_activeNode is null
     * @return true if loaded, false if not loaded
     */
    inline bool is_loaded() const
    {
        return m_activeNode.NotNull();
    }

    /**
     * Calculates a new position for this Satellite. Calls the Trajectory for a
     * new position, or uses the position of the active node when loaded.
     *
     * @return Position relative to parent, same as get_position
     */
    virtual LongVector3 calculate_position();

    /**
     * Try loading the satellite into the scene
     * Usually called when entering an ActiveArea (player gets close)
     * @param area [in] ActiveArea to load into
     * @param pos [in] Desired position in meters to load into.
     * @return Pointer to Active Node, null if load failed
     */
    virtual Node* load(ActiveArea* area, const Vector3& pos);

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

    // Only partially implemented
    // Scale for how many LongVector3 units is equal to a meter. This only
    // applies to this Satellite's children.
    //
    // m_precision bit shifts m_position, multiplying it by powers of two
    //
    // precision = -1 ->   1  unit  = 2 meters
    // precision = 0  ->   1  units = 1 meter
    // precision = 1  ->   2  units = 1 meter
    // precision = 2  ->   4  units = 1 meter
    // ...
    // precision = 10 ->  1024 units = 1 meter (The Default)
    //
    // around 10-14 is good for a solar system
    // maybe increase the value for planets
    // Large negative values are for galactic scales
    int m_precision = 10;

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
    // Null if not loaded
    WeakPtr<Node> m_activeNode;

    // ActiveArea loaded into
    WeakPtr<ActiveArea> m_activeArea;

    // Position of the ActiveArea above the moment its loaded. Used to
    // calculate m_position quickly
    LongVector3 m_activeAreaLoadedPos;

    // Capture of m_position taken on load
    LongVector3 m_positionLoaded;

    // Allow Trajectory to control the position of the ActiveNode
    // aka: on rails
    // maybe set this to false when the active node gets nudged
    bool m_trajectoryOverride;

    // Set this to true when the active node gets interacted with, causing a
    // change in its velocity
    bool m_nudged;

    // orbital stuff goes here. something like:
    // Trajectory m_trajectory;

};

}
