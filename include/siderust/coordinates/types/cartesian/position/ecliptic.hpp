#pragma once

#include "../../../cartesian.hpp"

namespace siderust {
namespace cartesian {
namespace position {
template <typename U>
using EclipticMeanJ2000 = Position<centers::Heliocentric, frames::EclipticMeanJ2000, U>;

template <typename U>
using HelioBarycentric = Position<centers::Barycentric, frames::EclipticMeanJ2000, U>;

template <typename U>
using GeoBarycentric = Position<centers::Barycentric, frames::EclipticMeanJ2000, U>;

template <typename U>
using MoonGeocentric = Position<centers::Geocentric, frames::EclipticMeanJ2000, U>;
} // namespace position
} // namespace cartesian
} // namespace siderust
