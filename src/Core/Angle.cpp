#include "Core/Angle.h"

namespace AngleLiterals
{
    Angle operator"" _deg(long double deg) { return Angle::Degrees(static_cast<float>(deg)); }
    Angle operator"" _rad(long double rad) { return Angle::Radians(static_cast<float>(rad)); }
}