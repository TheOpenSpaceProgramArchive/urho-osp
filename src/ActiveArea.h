#pragma once

#include <Urho3D/Scene/Scene.h>

#include "Satellite.h"

using namespace Urho3D;

namespace osp {

class ActiveArea;


/**
 * Turns an ordinary Urho3D scene into a window to the larger OSP universe
 * Loads and unloads Satellites, and handles floating origin
 */
class ActiveArea : public Satellite
{


    URHO3D_OBJECT(ActiveArea, Satellite)

public:

    ActiveArea(Context* context);
    ActiveArea(Context* context, Scene* scene);
    ~ActiveArea();

    void update(float timeStep);

    //void relocate(AstronomicalBody* body, const LongVector3& localBodyPos);

    Satellite* get_focus() const
    {
        return m_focus;
    }

    void set_focus(Satellite* sat)
    {
        m_focus = sat;
    }

    Node* load(ActiveArea* area, const Vector3& pos) override;

    Node* load_preview(ActiveArea* area) override;

    void unload() override;

private:

    /**
     * See if m_loadRadius intersects another Satellite's m_radius, then load
     * if they do. Relative position should already be calculated. Called in
     * FixedUpdate when looping through all the Satellites.
     * @param sat [in] Pointer to Satellite to load
     * @param relative [in] Relative Position
     * @param overflowCount [in] Count for how many times relative position
     *                           overflowed or underflowed.
     */
    void distance_check_then_load(Satellite* sat, LongVector3& relative,
                                  IntVector3& overflowCount);

    // The Satellite to follow around. ActiveArea will try keeping this
    // Satellite's active node at the scene's center by translating everything,
    // and moving the ActiveArea.
    WeakPtr<Satellite> m_focus;

    // Satellites that are currently loaded into the scene
    Vector< WeakPtr<Satellite> > m_loaded;

    // Satellites that have previews loaded into the scene.
    Vector< WeakPtr<Satellite> > m_previews;

    // Nodes associated with the previewed satellites above
    // This vector is parallel to m_previews.
    // eg. m_previewNodes[3] should be associated with m_previews[3]
    Vector< WeakPtr<Satellite> > m_previewNodes;

};

}
