#pragma once

#include <Urho3D/Container/HashMap.h>
#include <Urho3D/Core/Context.h>
#include <Urho3D/Physics/PhysicsEvents.h>
#include <Urho3D/Resource/JSONValue.h>
#include <Urho3D/Scene/LogicComponent.h>

#include "../Resource/PerformanceCurves.h"
#include "../Satellites/ActiveArea.h"
#include "Wire.h"


using namespace Urho3D;

enum MachStates : int
{
    E_NOTLOADED  = 0,
    E_ACTIVE = 1,
    E_EDITOR = 2,
};

namespace osp
{

class Machine;
class WireInput;
class WireOutput;

/**
 * Base class for Machines
 * See wiki page (if it exists)
 */
class Machine : public Component
{
    URHO3D_OBJECT(Machine, Component)

public:


    static void RegisterObject(Context* context)
    {
        //context->RegisterFactory<Machine>();
    }

    using Component::Component;
    ~Machine() = default;

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
     * Update all of the WireOutputs, if output depends on the inputs, then
     * call update_outputs on the connected Machines
     */
    virtual void update_outputs() = 0;

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

    /**
     * @return Reference to list of available WireOutputs
     */
    PODVector< WireOutput* > const& get_wire_outputs()
    {
        return m_wireOutputs;
    }

    /**
     * @return Reference to list of available WireOutputs
     */
    PODVector< WireInput* > const& get_wire_inputss()
    {
        return m_wireInputs;
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

    // List of wire inputs and outputs only to be listed by other classes
    // WireIn/Outputs should be member variables of the class
    PODVector< WireInput* > m_wireInputs;
    PODVector< WireOutput* > m_wireOutputs;

    unsigned m_prevOutputUpdateFrame;

    //void resubscribe();
};

}
