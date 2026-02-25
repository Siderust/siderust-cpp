#pragma once

/**
 * @file cartesian.hpp
 * @ingroup coordinates_cartesian
 * @brief Typed cartesian coordinate templates.
 */

#include "../centers.hpp"
#include "../ffi_core.hpp"
#include "../frames.hpp"

#include <qtty/qtty.hpp>

#include <ostream>

namespace siderust {
namespace cartesian {

/**
 * @brief A unit-vector direction in Cartesian form, compile-time frame-tagged.
 *
 * Mirrors Rust's `affn::cartesian::Direction<F>`.
 *
 * @ingroup coordinates_cartesian
 * @tparam F Reference frame tag (e.g. `frames::ICRS`).
 */
template <typename F>
struct Direction {
    static_assert(frames::is_frame_v<F>, "F must be a valid frame tag");

    double x; ///< X component (unitless).
    double y; ///< Y component (unitless).
    double z; ///< Z component (unitless).

    Direction() : x(0), y(0), z(0) {}
    Direction(double x_, double y_, double z_) : x(x_), y(y_), z(z_) {}

    static constexpr siderust_frame_t frame_id() {
        return frames::FrameTraits<F>::ffi_id;
    }
};

/**
 * @brief A 3D Cartesian position, compile-time tagged by center, frame, unit.
 *
 * Mirrors Rust's `affn::cartesian::Position<C, F, U>`.
 *
 * @ingroup coordinates_cartesian
 * @tparam C Reference center tag (e.g. `centers::Geocentric`).
 * @tparam F Reference frame tag (e.g. `frames::ECEF`).
 * @tparam U Length unit (default: `qtty::Meter`).
 */
template <typename C, typename F, typename U = qtty::Meter>
struct Position {
    static_assert(frames::is_frame_v<F>, "F must be a valid frame tag");
    static_assert(centers::is_center_v<C>, "C must be a valid center tag");

    U comp_x; ///< X component.
    U comp_y; ///< Y component.
    U comp_z; ///< Z component.

    Position()
        : comp_x(U(0)), comp_y(U(0)), comp_z(U(0)) {}

    Position(U x_, U y_, U z_)
        : comp_x(x_), comp_y(y_), comp_z(z_) {}

    Position(double x_, double y_, double z_)
        : comp_x(U(x_)), comp_y(U(y_)), comp_z(U(z_)) {}

    U x() const { return comp_x; }
    U y() const { return comp_y; }
    U z() const { return comp_z; }

    static constexpr siderust_frame_t  frame_id() { return frames::FrameTraits<F>::ffi_id; }
    static constexpr siderust_center_t center_id() { return centers::CenterTraits<C>::ffi_id; }

    /// Convert to C FFI struct.
    siderust_cartesian_pos_t to_c() const {
        return {comp_x.value(), comp_y.value(), comp_z.value(),
                frame_id(), center_id()};
    }

    /// Create from C FFI struct (ignoring runtime frame/center - trust the type).
    static Position from_c(const siderust_cartesian_pos_t& c) {
        return Position(c.x, c.y, c.z);
    }
};

// ============================================================================
// Stream operators
// ============================================================================

/**
 * @brief Stream operator for Position.
 */
template <typename C, typename F, typename U>
inline std::ostream& operator<<(std::ostream& os, const Position<C, F, U>& pos) {
    return os << pos.x() << ", " << pos.y() << ", " << pos.z();
}

} // namespace cartesian
} // namespace siderust
