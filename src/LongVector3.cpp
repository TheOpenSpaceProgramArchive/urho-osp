#include <cstdio>

#include "LongVector3.h"

using namespace Urho3D;

const LongVector3 LongVector3::ZERO;
const LongVector3 LongVector3::LEFT(-1, 0, 0);
const LongVector3 LongVector3::RIGHT(1, 0, 0);
const LongVector3 LongVector3::UP(0, 1, 0);
const LongVector3 LongVector3::DOWN(0, -1, 0);
const LongVector3 LongVector3::FORWARD(0, 0, 1);
const LongVector3 LongVector3::BACK(0, 0, -1);
const LongVector3 LongVector3::ONE(1, 1, 1);

String LongVector3::ToString() const
{
    char tempBuffer[CONVERSION_BUFFER_LENGTH];
    sprintf(tempBuffer, "%ld %ld %ld", x_, y_, z_);
    return String(tempBuffer);
}
