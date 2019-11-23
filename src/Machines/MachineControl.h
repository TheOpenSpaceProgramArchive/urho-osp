#pragma once

#include "Machine.h"

using namespace Urho3D;

namespace osp
{

/**
 * A Machine that outputs user inputs to control the craft. For now this
 * directly writes keyboard controls to the WireOutputs
 */
class MachineControl : public Machine
{
    URHO3D_OBJECT(MachineControl, Machine)

public:
    /* Forward base class constructor */
    MachineControl(Context* context);
    ~MachineControl() = default;

    static void RegisterObject(Context* context)
    {
        context->RegisterFactory<MachineControl>();

    }

    // TODO: implement these
    void loaded_active() override;
    void loaded_editor() override {}
    void unload() override {};
    void update_outputs() override;

    void load_json(const JSONObject& machine) override;

private:

    WireOutput m_outRotation;
    WireOutput m_outThrottle;
    //WireOutput m_outPrimary;

    // Stage control, sequentialy fire WIRE_POWER ON with an array of outputs
    Vector<WireOutput> m_outStages;

};

}
