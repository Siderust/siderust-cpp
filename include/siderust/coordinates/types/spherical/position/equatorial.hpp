#pragma once

#include "../../../spherical.hpp"

namespace siderust {
namespace spherical {
namespace position {
template<typename U>
using ICRS = Position<centers::Barycentric, frames::ICRS, U>;

template<typename U>
using GCRS = Position<centers::Geocentric, frames::ICRS, U>;

template<typename U>
using HCRS = Position<centers::Heliocentric, frames::ICRS, U>;
} // namespace position
} // namespace spherical
} // namespace siderust
