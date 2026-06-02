#pragma once

/**
 * @file stream.hpp
 * @brief Shared helpers for human-readable coordinate stream output.
 */

#include "../../centers.hpp"
#include "../../frames.hpp"

#include <ostream>

namespace siderust {
namespace coordinates {
namespace detail {

template <typename C, typename F> inline void write_center_frame(std::ostream &os) {
  os << centers::CenterTraits<C>::name() << ' ' << frames::FrameTraits<F>::name();
}

template <typename F> inline void write_frame(std::ostream &os) {
  os << frames::FrameTraits<F>::name();
}

} // namespace detail
} // namespace coordinates
} // namespace siderust
