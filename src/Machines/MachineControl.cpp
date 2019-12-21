#include <Urho3D/Input/Input.h>

#include "MachineControl.h"

using namespace osp;

MachineControl::MachineControl(Urho3D::Context* context) :
        Machine::Machine(context),
        m_outRotation(this, "Rotation"),
        m_outThrottle(this, "Throttle")
        //m_outPrimary(this, "Primary"),
{
    m_wireOutputs.Push(&m_outRotation);
    m_wireOutputs.Push(&m_outThrottle);
    printf("Print from MachineControl!\n");
}


void MachineControl::loaded_active()
{
    //m_outThrottle.send_percentf(0.5f);



}

void MachineControl::update_outputs()
{
    Input* i = GetSubsystem<Input>();

    m_outThrottle.send_percentf(0.0f + 1.0f * i->GetKeyDown(KEY_F));

    //printf("throttle:%f\n", 0.0f + 1.0f * i->GetKeyDown(KEY_F));
}

void MachineControl::load_json(const JSONObject& machine)
{
    // TODO
}
