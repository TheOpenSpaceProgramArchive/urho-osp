#include <Urho3D/IO/Log.h>

#include "Machine.h"

using namespace osp;

void Machine::parse_factors(PerformanceCurves& curves, const JSONObject& factors)
{


    for (auto factorIt = factors.Begin(); factorIt != factors.End(); factorIt ++)
    {

        if (factorIt->second_.IsArray())
        {
            const JSONArray& factorPointsJson = factorIt->second_.GetArray();
            FactorCurve* factor = curves.add_factor(factorIt->first_, 100, 0);
            //factor = curves.get_factor(factorIt->first_);
            PODVector<uint16_t>& factorPoints = factor->get_points();
            factorPoints.Resize(factorPointsJson.Size());
            // Support multiple curve formats, just 0-100 for now

            for (int i = 0; i < factorPoints.Size(); i ++)
            {
                float value = Clamp(factorPointsJson[i].GetFloat() / 100.0f, 0.0f, 1.0f);
                factorPoints[i] = (value * 65535);
            }
        }
    }
}
