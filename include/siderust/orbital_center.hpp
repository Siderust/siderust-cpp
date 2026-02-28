#pragma once

/**
 * @file orbital_center.hpp
 * @brief C++ wrapper for body-centric coordinates using orbital elements.
 *
 * Provides `OrbitReferenceCenter` enum and `BodycentricParams` struct to define
 * and work with coordinate systems centered on orbiting bodies (planets, moons,
 * satellites, etc.). Use these with `Position<centers::Bodycentric, F, U>` to
 * express positions relative to an orbiting body.
 *
 * @example
 * ```cpp
 * using namespace siderust;
 *
 * // Define Mars's heliocentric orbit
 * Orbit mars_orbit = {
 *     1.524,    // semi_major_axis_au
 *     0.0934,   // eccentricity
 *     1.85,     // inclination_deg
 *     49.56,    // lon_ascending_node_deg
 *     286.5,    // arg_perihelion_deg
 *     19.41,    // mean_anomaly_deg
 *     2451545.0 // epoch_jd (J2000)
 * };
 *
 * // Create parameters: Mars as center of a heliocentric orbit
 * BodycentricParams mars_center = BodycentricParams::heliocentric(mars_orbit);
 *
 * // Later, compute Mars position and use it as reference center
 * // (Integration with transform functions coming in future release)
 * ```
 */

#include "bodies.hpp"
#include "centers.hpp"
#include "ffi_core.hpp"
#include <cstdint>
#include <ostream>

namespace siderust {

/**
 * @brief Specifies the reference center for an orbit.
 *
 * Indicates which standard center the orbital elements are defined relative to.
 * This is needed when transforming positions to/from a body-centric frame,
 * as the orbit must be converted to match the coordinate system.
 */
enum class OrbitReferenceCenter : std::uint8_t {
  /// Orbit defined relative to the solar system barycenter.
  Barycentric = 0,
  /// Orbit defined relative to the Sun (planets, asteroids, comets).
  Heliocentric = 1,
  /// Orbit defined relative to Earth (artificial satellites, Moon).
  Geocentric = 2,
};

/**
 * @brief Parameters for a body-centric coordinate system.
 *
 * Specifies the orbital elements of a celestial body and the reference center
 * for those elements. This allows computing a body's position at any Julian date
 * using Keplerian propagation, then using that position as the origin of a
 * coordinate system.
 *
 * # Use Cases
 *
 * - **Satellites**: Define L1, L2, L3, L4, L5 positions relative to their
 *   parent body (e.g., a halo orbit at the Sun-Earth L2).
 * - **Planets**: Compute stellar positions as seen from another planet.
 * - **Moons**: Express coordinates relative to a moon's center (e.g., Phobos
 *   relative to Mars).
 *
 * # Example: L2 Satellite
 *
 * ```cpp
 * // Approximate L2 orbit (1.5M km from Earth on opposite side of Sun)
 * // In practice, L2 is a quasi-periodic Halo orbit, but we approximate here
 * Orbit l2_approx = {
 *     1.0,      // semi_major_axis_au (~1 AU from Sun, like Earth)
 *     0.01,     // eccentricity (small: stable near L2)
 *     0.0,      // inclination_deg
 *     0.0,      // lon_ascending_node_deg
 *     0.0,      // arg_perihelion_deg
 *     0.0,      // mean_anomaly_deg
 *     2451545.0 // epoch_jd (J2000)
 * };
 * BodycentricParams l2_center = BodycentricParams::heliocentric(l2_approx);
 *
 * // Now use l2_center as the reference for body-centric coordinates
 * // to express Mars's position relative to L2.
 * ```
 */
struct BodycentricParams {
  /// Keplerian orbital elements of the body.
  Orbit orbit;

  /// Which standard center the orbit is defined relative to.
  OrbitReferenceCenter orbit_center;

  /**
   * @brief Creates parameters for a body with the given orbit.
   *
   * @param orb The Keplerian orbital elements.
   * @param center The reference center for the orbit.
   */
  BodycentricParams(const Orbit &orb, OrbitReferenceCenter center)
      : orbit(orb), orbit_center(center) {}

  /**
   * @brief Creates parameters for a body orbiting the Sun.
   *
   * Most common: planets, asteroids, comets.
   *
   * @param orb Heliocentric orbital elements.
   * @return BodycentricParams with Heliocentric reference.
   */
  static BodycentricParams heliocentric(const Orbit &orb) {
    return BodycentricParams(orb, OrbitReferenceCenter::Heliocentric);
  }

  /**
   * @brief Creates parameters for a body orbiting Earth.
   *
   * Use for artificial satellites, the Moon, etc.
   *
   * @param orb Geocentric orbital elements.
   * @return BodycentricParams with Geocentric reference.
   */
  static BodycentricParams geocentric(const Orbit &orb) {
    return BodycentricParams(orb, OrbitReferenceCenter::Geocentric);
  }

  /**
   * @brief Creates parameters for a body orbiting the barycenter.
   *
   * @param orb Barycentric orbital elements.
   * @return BodycentricParams with Barycentric reference.
   */
  static BodycentricParams barycentric(const Orbit &orb) {
    return BodycentricParams(orb, OrbitReferenceCenter::Barycentric);
  }

  /// Default: circular 1 AU heliocentric orbit (placeholder).
  BodycentricParams()
      : orbit{qtty::AstronomicalUnit(1.0), 0.0, qtty::Degree(0.0),
              qtty::Degree(0.0), qtty::Degree(0.0), qtty::Degree(0.0),
              2451545.0},
        orbit_center(OrbitReferenceCenter::Heliocentric) {}

  /// Convert to C FFI struct for passing to siderust_to_bodycentric /
  /// siderust_from_bodycentric.
  SiderustBodycentricParams to_c() const {
    SiderustBodycentricParams c{};
    c.orbit = orbit.to_c();
    c.orbit_center = static_cast<uint8_t>(orbit_center);
    return c;
  }
};

// Stream operator for OrbitReferenceCenter
inline std::ostream &operator<<(std::ostream &os,
                                OrbitReferenceCenter center) {
  switch (center) {
  case OrbitReferenceCenter::Barycentric:
    return os << "Barycentric";
  case OrbitReferenceCenter::Heliocentric:
    return os << "Heliocentric";
  case OrbitReferenceCenter::Geocentric:
    return os << "Geocentric";
  }
  return os << "Unknown";
}

} // namespace siderust
