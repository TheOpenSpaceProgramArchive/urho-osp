#pragma once

#include "Machine.h"

using namespace Urho3D;

namespace osp
{

class MachineControl : public Machine
{
public:
    MachineControl(Context* context);
    ~MachineControl();

    static void RegisterObject(Context* context)
    {
        context->RegisterFactory<MachineControl>();

    }

    void loaded_active() override {}
    void loaded_editor() override {}

    void load_json(const JSONObject& machine) override;

private:
};

}
