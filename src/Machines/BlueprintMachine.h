#pragma once

#include <Urho3D/Core/Context.h>

/**
 * Stores data on how to create a Machine
 */
class BlueprintMachine : public Object
{

    URHO3D_OBJECT(BlueprintMachine, Object)

public:
    BlueprintMachine(Context* context);

protected:

    String m_machinType;

};

