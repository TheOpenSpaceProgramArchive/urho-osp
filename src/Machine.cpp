#include <Urho3D/IO/Log.h>

#include "Machine.h"

using namespace osp;

void Machine::parse_factors(PerformanceCurves& curves, const JSONObject& factors)
{


    for (auto factorIt = factors.Begin(); factorIt != factors.End(); factorIt ++)
    {
        URHO3D_LOGINFOF("aaaasfhiouhiuohohiouiuhouh %s", factorIt->first_.CString());

        curves.add_factor(factorIt->first_);

    }
}
