#pragma once

#include <Urho3D/Container/HashMap.h>
#include <Urho3D/Core/Context.h>
#include <Urho3D/Physics/PhysicsEvents.h>
#include <Urho3D/Resource/JSONValue.h>
#include <Urho3D/Scene/LogicComponent.h>

#include "../Resource/PerformanceCurves.h"
#include "../Satellites/ActiveArea.h"


using namespace Urho3D;

enum MachStates : int
{
    E_NOTLOADED = 0,
    E_ACTIVE = 1,
    E_EDITOR = 2,
};

namespace osp {

/**
 * Base class for Machines
 * See wiki page (if it exists)
 */
class Machine : public Component
{
    URHO3D_OBJECT(Machine, Component)

public:

    Machine(Context* context);
    ~Machine() {}

    /**
     * Load data from a JSONObject
     * Called by OspUniverse after creating a new machine
     *
     * @param machine [in] JSONObject directly from the "machines" array in the
     *                      sturdy file
     */
    virtual void load_json(const JSONObject& machine) {}

    /**
     * Physics update bound to E_PHYSICSPRESTEP
     * @param timestep
     */
    //virtual void update_active(StringHash eventType, VariantMap& eventData) {};

    // TODO: in-editor functions somehow
    //virtual void update_editor();

    /**
     * Called when being loaded into an ActiveArea
     */
    virtual void loaded_active() {}

    /**
     * Called when loaded into the editor
     */
    virtual void loaded_editor() {}

    /**
     * Unloaded from either active or editor
     */
    virtual void unload() {}

    static void RegisterObject(Context* context)
    {
        context->RegisterFactory<Machine>();
    }

    /**
     * Called by Urho3D when the scene is canged
     *
     * @param scene
     */
    void OnSceneSet(Scene* scene) override;

    /**
     * Called by Urho3D when enabled is changed
     *
     */
    void OnNodeSetEnabled(Node* node) override;

protected:

    ActiveArea* m_area;

    MachStates m_state;
    HashMap<StringHash, float> m_curveInputs;

    //void resubscribe();
};

}
