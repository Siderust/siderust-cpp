#pragma once

/**
 * @file orbit.hpp
 * @brief Orbit model wrappers and propagation helpers.
 */

#include "coordinates.hpp"
#include "ffi_core.hpp"
#include "time.hpp"

#include <cstdint>
#include <type_traits>
#include <utility>

namespace siderust {

namespace detail {

template <typename C> constexpr SiderustOrbitRefCenter orbit_ref_center_id() {
  if constexpr (std::is_same_v<C, centers::Barycentric>) {
    return 0;
  } else if constexpr (std::is_same_v<C, centers::Heliocentric>) {
    return 1;
  } else if constexpr (std::is_same_v<C, centers::Geocentric>) {
    return 2;
  } else {
    static_assert(!sizeof(C), "kepler_position only supports barycentric, heliocentric, or geocentric centers");
    return 1;
  }
}

} // namespace detail

enum class ConicKind : std::uint8_t {
  Elliptic = 0,
  Hyperbolic = 1,
};

struct KeplerianOrbit {
  qtty::AstronomicalUnit semi_major_axis;
  double eccentricity;
  qtty::Degree inclination;
  qtty::Degree lon_ascending_node;
  qtty::Degree arg_perihelion;
  qtty::Degree mean_anomaly;
  double epoch_jd;

  static KeplerianOrbit from_c(const siderust_orbit_t &c) {
    return {qtty::AstronomicalUnit(c.semi_major_axis_au),
            c.eccentricity,
            qtty::Degree(c.inclination_deg),
            qtty::Degree(c.lon_ascending_node_deg),
            qtty::Degree(c.arg_periapsis_deg),
            qtty::Degree(c.mean_anomaly_deg),
            c.epoch_jd};
  }

  siderust_orbit_t to_c() const {
    return {semi_major_axis.value(),
            eccentricity,
            inclination.value(),
            lon_ascending_node.value(),
            arg_perihelion.value(),
            mean_anomaly.value(),
            epoch_jd};
  }
};

using Orbit = KeplerianOrbit;

template <typename C = centers::Heliocentric>
inline cartesian::Position<C, frames::EclipticMeanJ2000, qtty::AstronomicalUnit>
kepler_position(const KeplerianOrbit &orbit, const JulianDate &jd) {
  static_assert(centers::is_center_v<C>,
                "C must be a valid center tag (default: Heliocentric)");
  siderust_cartesian_pos_t c_out{};
  check_status(
      siderust_kepler_position_ex(orbit.to_c(), detail::orbit_ref_center_id<C>(),
                                  jd.value(), &c_out),
      "kepler_position");
  return cartesian::Position<C, frames::EclipticMeanJ2000,
                             qtty::AstronomicalUnit>(
      qtty::AstronomicalUnit(c_out.x), qtty::AstronomicalUnit(c_out.y),
      qtty::AstronomicalUnit(c_out.z));
}

struct MeanMotionOrbit {
  qtty::AstronomicalUnit semi_major_axis;
  double eccentricity;
  qtty::Degree inclination;
  qtty::Degree lon_ascending_node;
  qtty::Degree arg_periapsis;
  double mean_motion_deg_per_day;
  double epoch_jd;

  static MeanMotionOrbit from_c(const siderust_mean_motion_orbit_t &c) {
    return {qtty::AstronomicalUnit(c.semi_major_axis_au),
            c.eccentricity,
            qtty::Degree(c.inclination_deg),
            qtty::Degree(c.lon_ascending_node_deg),
            qtty::Degree(c.arg_periapsis_deg),
            c.mean_motion_deg_per_day,
            c.epoch_jd};
  }

  siderust_mean_motion_orbit_t to_c() const {
    return {semi_major_axis.value(),
            eccentricity,
            inclination.value(),
            lon_ascending_node.value(),
            arg_periapsis.value(),
            mean_motion_deg_per_day,
            epoch_jd};
  }

  cartesian::Position<centers::Heliocentric, frames::EclipticMeanJ2000,
                      qtty::AstronomicalUnit>
  position_at(const JulianDate &jd) const {
    siderust_cartesian_pos_t out{};
    check_status(siderust_mean_motion_position(to_c(), jd.value(), &out),
                 "MeanMotionOrbit::position_at");
    return {qtty::AstronomicalUnit(out.x), qtty::AstronomicalUnit(out.y),
            qtty::AstronomicalUnit(out.z)};
  }
};

struct ConicOrbit {
  qtty::AstronomicalUnit periapsis_distance;
  double eccentricity;
  qtty::Degree inclination;
  qtty::Degree lon_ascending_node;
  qtty::Degree arg_periapsis;
  qtty::Degree mean_anomaly;
  double epoch_jd;

  static ConicOrbit from_c(const siderust_conic_orbit_t &c) {
    return {qtty::AstronomicalUnit(c.periapsis_distance_au),
            c.eccentricity,
            qtty::Degree(c.inclination_deg),
            qtty::Degree(c.lon_ascending_node_deg),
            qtty::Degree(c.arg_periapsis_deg),
            qtty::Degree(c.mean_anomaly_deg),
            c.epoch_jd};
  }

  siderust_conic_orbit_t to_c() const {
    return {periapsis_distance.value(),
            eccentricity,
            inclination.value(),
            lon_ascending_node.value(),
            arg_periapsis.value(),
            mean_anomaly.value(),
            epoch_jd};
  }

  constexpr ConicKind kind() const {
    return eccentricity < 1.0 ? ConicKind::Elliptic : ConicKind::Hyperbolic;
  }

  cartesian::Position<centers::Heliocentric, frames::EclipticMeanJ2000,
                      qtty::AstronomicalUnit>
  position_at(const JulianDate &jd) const {
    siderust_cartesian_pos_t out{};
    check_status(siderust_conic_position(to_c(), jd.value(), &out),
                 "ConicOrbit::position_at");
    return {qtty::AstronomicalUnit(out.x), qtty::AstronomicalUnit(out.y),
            qtty::AstronomicalUnit(out.z)};
  }
};

class PreparedOrbit {
  SiderustPreparedOrbitHandle handle_ = nullptr;

public:
  PreparedOrbit() = default;

  explicit PreparedOrbit(const KeplerianOrbit &orbit) {
    check_status(siderust_prepared_orbit_create(orbit.to_c(), &handle_),
                 "PreparedOrbit");
  }

  ~PreparedOrbit() {
    if (handle_) {
      siderust_prepared_orbit_destroy(handle_);
    }
  }

  PreparedOrbit(const PreparedOrbit &) = delete;
  PreparedOrbit &operator=(const PreparedOrbit &) = delete;

  PreparedOrbit(PreparedOrbit &&other) noexcept : handle_(other.handle_) {
    other.handle_ = nullptr;
  }

  PreparedOrbit &operator=(PreparedOrbit &&other) noexcept {
    if (this != &other) {
      if (handle_) {
        siderust_prepared_orbit_destroy(handle_);
      }
      handle_ = other.handle_;
      other.handle_ = nullptr;
    }
    return *this;
  }

  explicit operator bool() const { return handle_ != nullptr; }

  cartesian::Position<centers::Heliocentric, frames::EclipticMeanJ2000,
                      qtty::AstronomicalUnit>
  position_at(const JulianDate &jd) const {
    siderust_cartesian_pos_t out{};
    check_status(siderust_prepared_orbit_position(handle_, jd.value(), &out),
                 "PreparedOrbit::position_at");
    return {qtty::AstronomicalUnit(out.x), qtty::AstronomicalUnit(out.y),
            qtty::AstronomicalUnit(out.z)};
  }
};

inline std::ostream &operator<<(std::ostream &os, ConicKind kind) {
  switch (kind) {
  case ConicKind::Elliptic:
    return os << "Elliptic";
  case ConicKind::Hyperbolic:
    return os << "Hyperbolic";
  }
  return os << "Unknown";
}

} // namespace siderust
