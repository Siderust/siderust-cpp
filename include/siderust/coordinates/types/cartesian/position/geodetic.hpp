#pragma once

#include "../../../cartesian.hpp"

namespace siderust {
namespace cartesian {
namespace position {
template<typename U>
using ECEF = Position<centers::Geocentric, frames::ECEF, U>;
} // namespace position
} // namespace cartesian
} // namespace siderust
