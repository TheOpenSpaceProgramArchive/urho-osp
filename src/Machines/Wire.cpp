#include <Urho3D/IO/Log.h>

#include "Wire.h"

// put stuff here maybe

namespace osp
{

    void WireInput::Connect(WireOutput *data)
    {
        // TODO: do some checks
        data->m_connectedInputs.Push(WeakPtr<WireInput>(this));
        m_dataIn = data;
    }

    float WireInput::recieve_percentf()
    {
        if (m_dataIn.Null())
        {
            // Input is not connected!
            URHO3D_LOGINFOF("No Connection!");
            return 0.0f;
        }

        m_dataIn->m_machine->update_outputs();

        // If the connected output is a percent
        if (m_dataIn->m_type == WIRE_PERCENT)
        {
            // Make sure outputs are up to date on the latest frame, then
            // get the float value.
            return m_dataIn->m_data.GetFloat();
        }

        return 0.0f;
    }


    void update_to_frame();

    void WireOutput::send_percentf(float precent)
    {
        m_type = WIRE_PERCENT;
        m_data = precent;
    }
}
