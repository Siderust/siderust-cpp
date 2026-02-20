#pragma once

#include "../../../spherical.hpp"

namespace siderust {
namespace spherical {
namespace position {
template <typename U>
using Horizontal = Position<centers::Topocentric, frames::Horizontal, U>;
} // namespace position
} // namespace spherical
} // namespace siderust
