#pragma once

#include <Urho3D/Core/Context.h>
#include <Urho3D/Scene/LogicComponent.h>
#include <Urho3D/Resource/JSONValue.h>
#include <Urho3D/Container/HashMap.h>

#include "../Resource/PerformanceCurves.h"

using namespace Urho3D;

namespace osp {

/**
 * Base class for Machines
 * See wiki page (if it exists)
 */
class Machine : public LogicComponent
{
    URHO3D_OBJECT(Machine, LogicComponent)

public:



    Machine(Context* context) : LogicComponent(context) {}

    /**
     * Load data from a JSONObject
     * Called by OspUniverse after creating a new machine
     *
     * @param machine [in] JSONObject directly from the "machines" array in the
     *                      sturdy file
     */
    virtual void load_json(const JSONObject& machine) {}

    static void RegisterObject(Context* context)
    {
        context->RegisterFactory<Machine>();
    }

protected:

    HashMap<StringHash, float> m_curveInputs;

};

}
