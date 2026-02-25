#pragma once

#include "../../../spherical.hpp"

namespace siderust {
namespace spherical {
namespace position {
template <typename U>
using EclipticMeanJ2000 =
    Position<centers::Heliocentric, frames::EclipticMeanJ2000, U>;
} // namespace position
} // namespace spherical
} // namespace siderust
