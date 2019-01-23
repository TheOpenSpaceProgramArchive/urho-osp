#pragma once

#include <Urho3D/Core/Context.h>
#include <Urho3D/Scene/LogicComponent.h>
#include <Urho3D/Resource/JSONValue.h>
#include <Urho3D/Container/HashMap.h>

#include "PerformanceCurves.h"

using namespace Urho3D;

namespace osp {

/**
 * Base class for machines
 */
class Machine : public LogicComponent
{
    URHO3D_OBJECT(Machine, LogicComponent)

public:

    static void parse_factors(PerformanceCurves& curves, const JSONObject& factors);

    Machine(Context* context) : LogicComponent(context) {}

    virtual void load_json(const JSONObject& machine) {}

    static void RegisterObject(Context* context)
    {
        context->RegisterFactory<Machine>();
    }

protected:

    HashMap<StringHash, float> m_curveInputs;

};

}
