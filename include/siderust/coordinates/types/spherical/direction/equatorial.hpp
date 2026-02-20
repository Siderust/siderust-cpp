#pragma once

#include "../../../spherical.hpp"

namespace siderust {
namespace spherical {
namespace direction {
using ICRS                 = Direction<frames::ICRS>;
using ICRF                 = Direction<frames::ICRF>;
using EquatorialMeanJ2000  = Direction<frames::EquatorialMeanJ2000>;
using EquatorialMeanOfDate = Direction<frames::EquatorialMeanOfDate>;
using EquatorialTrueOfDate = Direction<frames::EquatorialTrueOfDate>;
} // namespace direction
} // namespace spherical
} // namespace siderust
