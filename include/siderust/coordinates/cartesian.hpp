#pragma once

/**
 * @file cartesian.hpp
 * @ingroup coordinates_cartesian
 * @brief Typed cartesian coordinate templates.
 */

#include "../centers.hpp"
#include "../ffi_core.hpp"
#include "../frames.hpp"
#include "../time.hpp"

#include <qtty/qtty.hpp>

#include <ostream>
#include <cmath>
#include <type_traits>

// Forward-declare spherical Position to avoid circular include.
namespace siderust { namespace spherical { template <typename C, typename F, typename U> struct Position; } }

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
template <typename F> struct Direction {
  static_assert(frames::is_frame_v<F>, "F must be a valid frame tag");

  double x; ///< X component (unitless).
  double y; ///< Y component (unitless).
  double z; ///< Z component (unitless).

  Direction() : x(0), y(0), z(0) {}
  Direction(double x_, double y_, double z_) : x(x_), y(y_), z(z_) {}

  static constexpr siderust_frame_t frame_id() {
    return frames::FrameTraits<F>::ffi_id;
  }

  /**
   * @brief Transform this direction to a different reference frame.
   *
   * Only enabled when a `FrameRotationProvider` exists for the pair (F, Target).
   * For time-independent (fixed-epoch) transforms, `jd` is still required but
   * its value is ignored.
   *
   * @tparam Target  Destination frame tag.
   * @param  jd      Julian Date (TT) for time-dependent rotations.
   */
  template <typename Target>
  std::enable_if_t<frames::has_frame_transform_v<F, Target>, Direction<Target>>
  to_frame(const JulianDate &jd) const {
    if constexpr (std::is_same_v<F, Target>) {
      return Direction<Target>(x, y, z);
    } else {
      siderust_cartesian_pos_t out{};
      check_status(
          siderust_cartesian_dir_transform_frame(
              x, y, z,
              frames::FrameTraits<F>::ffi_id,
              frames::FrameTraits<Target>::ffi_id,
              jd.value(), &out),
          "cartesian::Direction::to_frame");
      return Direction<Target>(out.x, out.y, out.z);
    }
  }

  /**
   * @brief Shorthand: `.to<Target>(jd)` (calls `to_frame`).
   */
  template <typename Target>
  auto to(const JulianDate &jd) const
      -> decltype(this->template to_frame<Target>(jd)) {
    return to_frame<Target>(jd);
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
template <typename C, typename F, typename U> struct Position {
  static_assert(frames::is_frame_v<F>, "F must be a valid frame tag");
  static_assert(centers::is_center_v<C>, "C must be a valid center tag");

  U comp_x; ///< X component.
  U comp_y; ///< Y component.
  U comp_z; ///< Z component.

  Position() : comp_x(U(0)), comp_y(U(0)), comp_z(U(0)) {}

  Position(U x_, U y_, U z_) : comp_x(x_), comp_y(y_), comp_z(z_) {}

  Position(double x_, double y_, double z_)
      : comp_x(U(x_)), comp_y(U(y_)), comp_z(U(z_)) {}

  U x() const { return comp_x; }
  U y() const { return comp_y; }
  U z() const { return comp_z; }

  U distance() const {
    using std::sqrt;
    const double vx = comp_x.value();
    const double vy = comp_y.value();
    const double vz = comp_z.value();
    return U(sqrt(vx * vx + vy * vy + vz * vz));
  }

  /**
   * @brief Convert this cartesian position to a spherical Position<C,F,U>.
   */
  spherical::Position<C, F, U> to_spherical() const;

  U distance_to(const Position &other) const {
    using std::sqrt;
    const double dx = comp_x.value() - other.comp_x.value();
    const double dy = comp_y.value() - other.comp_y.value();
    const double dz = comp_z.value() - other.comp_z.value();
    return U(sqrt(dx * dx + dy * dy + dz * dz));
  }

  static constexpr siderust_frame_t frame_id() {
    return frames::FrameTraits<F>::ffi_id;
  }
  static constexpr siderust_center_t center_id() {
    return centers::CenterTraits<C>::ffi_id;
  }

  /// Convert to C FFI struct.
  siderust_cartesian_pos_t to_c() const {
    return {comp_x.value(), comp_y.value(), comp_z.value(), frame_id(),
            center_id()};
  }

  /// Create from C FFI struct (ignoring runtime frame/center - trust the type).
  static Position from_c(const siderust_cartesian_pos_t &c) {
    return Position(c.x, c.y, c.z);
  }

  /**
   * @brief Transform this position to a different reference frame (same center).
   *
   * Only a pure rotation is applied; the reference center is unchanged.
   * Only enabled when a `FrameRotationProvider` exists for the pair (F, Target).
   *
   * @tparam Target  Destination frame tag.
   * @param  jd      Julian Date (TT) for time-dependent rotations.
   */
  template <typename Target>
  std::enable_if_t<frames::has_frame_transform_v<F, Target>, Position<C, Target, U>>
  to_frame(const JulianDate &jd) const {
    if constexpr (std::is_same_v<F, Target>) {
      return *this;
    } else {
      siderust_cartesian_pos_t out{};
      check_status(
          siderust_cartesian_pos_transform_frame(
              to_c(),
              frames::FrameTraits<Target>::ffi_id,
              jd.value(), &out),
          "cartesian::Position::to_frame");
      return Position<C, Target, U>(out.x, out.y, out.z);
    }
  }

  /**
   * @brief Shorthand: `.to<Target>(jd)` (calls `to_frame`).
   */
  template <typename Target>
  auto to(const JulianDate &jd) const
      -> decltype(this->template to_frame<Target>(jd)) {
    return to_frame<Target>(jd);
  }

  /**
   * @brief Transform this position to a different reference center (same frame).
   *
   * The FFI center-shift uses VSOP87 ephemeris vectors expressed in
   * EclipticMeanJ2000.  When the position is already in that frame the FFI
   * call is made directly; otherwise the position is first rotated to
   * ecliptic, shifted, and rotated back so the result is frame-correct.
   *
   * @tparam TargetC  Destination center tag.
   * @param  jd       Julian Date (TT) for the ephemeris evaluation.
   */
  template <typename TargetC>
  std::enable_if_t<centers::has_center_transform_v<C, TargetC>, Position<TargetC, F, U>>
  to_center(const JulianDate &jd) const {
    if constexpr (std::is_same_v<C, TargetC>) {
      return *this;
    } else if constexpr (std::is_same_v<F, frames::EclipticMeanJ2000>) {
      // Direct FFI call — shift vectors and position are both in ecliptic.
      siderust_cartesian_pos_t out{};
      check_status(
          siderust_cartesian_pos_transform_center(
              to_c(),
              centers::CenterTraits<TargetC>::ffi_id,
              jd.value(), &out),
          "cartesian::Position::to_center");
      return Position<TargetC, F, U>(out.x, out.y, out.z);
    } else {
      // Route through ecliptic so the shift vectors match the frame.
      auto ecl     = to_frame<frames::EclipticMeanJ2000>(jd);
      auto shifted = ecl.template to_center<TargetC>(jd);
      return shifted.template to_frame<F>(jd);
    }
  }

  /**
   * @brief Combined frame + center transform in one call.
   *
   * Routes through EclipticMeanJ2000 for the center shift so that
   * VSOP87 ephemeris vectors are applied in the correct frame:
   *   1. rotate to EclipticMeanJ2000
   *   2. shift center
   *   3. rotate to target frame
   *
   * @tparam TargetC  Destination center tag.
   * @tparam TargetF  Destination frame tag.
   * @param  jd       Julian Date (TT).
   */
  template <typename TargetC, typename TargetF>
  std::enable_if_t<
      frames::has_frame_transform_v<F, TargetF> &&
      centers::has_center_transform_v<C, TargetC>,
      Position<TargetC, TargetF, U>>
  transform(const JulianDate &jd) const {
    auto ecl     = to_frame<frames::EclipticMeanJ2000>(jd);
    auto shifted = ecl.template to_center<TargetC>(jd);
    return shifted.template to_frame<TargetF>(jd);
  }

  /**
   * @brief Subtract two positions in the same center/frame/unit (vector difference).
   */
  Position operator-(const Position &other) const {
    return Position(U(comp_x.value() - other.comp_x.value()),
                    U(comp_y.value() - other.comp_y.value()),
                    U(comp_z.value() - other.comp_z.value()));
  }

  /**
   * @brief Add two positions in the same center/frame/unit (vector sum).
   */
  Position operator+(const Position &other) const {
    return Position(U(comp_x.value() + other.comp_x.value()),
                    U(comp_y.value() + other.comp_y.value()),
                    U(comp_z.value() + other.comp_z.value()));
  }

  /**
   * @brief Magnitude of the position vector (alias for distance()).
   */
  U magnitude() const { return distance(); }
};

// ============================================================================
// Stream operators
// ============================================================================

/**
 * @brief Stream operator for Position.
 */
template <typename C, typename F, typename U>
inline std::ostream &operator<<(std::ostream &os,
                                const Position<C, F, U> &pos) {
  return os << pos.x() << ", " << pos.y() << ", " << pos.z();
}

} // namespace cartesian
} // namespace siderust
