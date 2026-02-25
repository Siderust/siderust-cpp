#pragma once

/**
 * @file bodycentric_transforms.hpp
 * @brief Body-centric coordinate transformations.
 *
 * Mirrors Rust's `ToBodycentricExt` and `FromBodycentricExt` traits:
 * - `to_bodycentric(pos, params, jd)` — free function transforming a
 *   Geocentric/Heliocentric/Barycentric position to one centered on the
 *   orbiting body described by `params`.
 * - `BodycentricPos<F,U>::to_geocentric(jd)` — inverse transform back to
 *   geocentric.
 *
 * The transform algorithm (mirroring Rust):
 * 1. Propagate the body's Keplerian orbit to JD → position in the orbit's
 *    reference center.
 * 2. Convert that position to match the source center (via VSOP87 offsets).
 * 3. `bodycentric = input - body_in_source_center`
 *
 * # Usage
 * ```cpp
 * #include <siderust/siderust.hpp>   // or just this file
 * using namespace siderust;
 * using namespace siderust::frames;
 * using namespace siderust::centers;
 * using qtty::AstronomicalUnit;
 *
 * auto jd = JulianDate::J2000;
 *
 * // ISS-like geocentric orbit
 * Orbit iss_orbit{0.0000426, 0.001, 51.6, 0.0, 0.0, 0.0, jd.value()};
 * BodycentricParams iss_params = BodycentricParams::geocentric(iss_orbit);
 *
 * // Moon's approximate geocentric position
 * cartesian::Position<Geocentric, EclipticMeanJ2000, AstronomicalUnit> moon_geo(
 *     0.00257, 0.0, 0.0);
 *
 * // Moon as seen from ISS
 * auto moon_from_iss = to_bodycentric(moon_geo, iss_params, jd);
 *
 * // Round-trip back to geocentric
 * auto recovered = moon_from_iss.to_geocentric(jd);
 * ```
 */

#include "../ffi_core.hpp"
#include "../orbital_center.hpp"
#include "../time.hpp"
#include "cartesian.hpp"

#include <cstdint>

namespace siderust {

// ============================================================================
// BodycentricPos<F, U>
// ============================================================================

/**
 * @brief Result of a body-centric coordinate transformation.
 *
 * Carries the relative position (target – body) and the embedded
 * `BodycentricParams` needed for the inverse transform (`to_geocentric`).
 * Mirrors Rust's `Position<Bodycentric, F, U>` which stores
 * `BodycentricParams` at runtime.
 *
 * @tparam F  Reference frame tag (e.g. `frames::EclipticMeanJ2000`).
 * @tparam U  Length unit (default: `qtty::AstronomicalUnit`).
 *
 * @ingroup coordinates_cartesian
 */
template <typename F, typename U = qtty::AstronomicalUnit>
struct BodycentricPos {
  static_assert(frames::is_frame_v<F>, "F must be a valid frame tag");

  /// Raw Cartesian position tagged with the Bodycentric center.
  cartesian::Position<centers::Bodycentric, F, U> pos;

  /// Orbital parameters of the body used as the coordinate origin.
  BodycentricParams params;

  // -- Accessors --

  U x() const { return pos.x(); }
  U y() const { return pos.y(); }
  U z() const { return pos.z(); }

  /// Distance from the body (norm of the embedded `pos`).
  U distance() const { return pos.distance(); }

  /// Distance to another body-centric position.
  U distance_to(const BodycentricPos &other) const { return pos.distance_to(other.pos); }

  /// Access the embedded orbital parameters of the body.
  const BodycentricParams &center_params() const { return params; }

  // ── Inverse transform ─────────────────────────────────────────────────────

  /**
   * @brief Transform back to geocentric coordinates.
   *
   * Mirrors Rust's `FromBodycentricExt::to_geocentric(jd)`.
   * Uses the same `params` and `jd` as the original `to_bodycentric()` call.
   *
   * @param jd  Julian Date (same as the forward transform).
   * @return Geocentric position in the same frame and unit.
   */
  cartesian::Position<centers::Geocentric, F, U>
  to_geocentric(const JulianDate &jd) const;
};

// ============================================================================
// to_bodycentric() — free function template
// ============================================================================

/**
 * @brief Transform a position to body-centric coordinates.
 *
 * Mirrors Rust's `position.to_bodycentric(params, jd)`.
 *
 * The source center must be `Geocentric`, `Heliocentric`, or `Barycentric`.
 * Calling this with `Bodycentric` or `Topocentric` as the source center will
 * throw `InvalidCenterError` at runtime.
 *
 * The result frame `F` and unit `U` are preserved from the source position.
 *
 * @tparam C  Source center (Geocentric, Heliocentric, or Barycentric).
 * @tparam F  Reference frame.
 * @tparam U  Length unit.
 * @param pos     Source position.
 * @param params  Orbital parameters of the body to use as the new center.
 * @param jd      Julian Date for Keplerian propagation and center shifts.
 * @return `BodycentricPos<F, U>` — relative position plus embedded params.
 *
 * @throws InvalidCenterError if the source center is not supported.
 */
template <typename C, typename F, typename U>
inline BodycentricPos<F, U>
to_bodycentric(const cartesian::Position<C, F, U> &pos,
               const BodycentricParams &params, const JulianDate &jd) {
  static_assert(centers::is_center_v<C>, "C must be a valid center tag");

  siderust_cartesian_pos_t c_pos = pos.to_c();
  SiderustBodycentricParams c_params = params.to_c();
  siderust_cartesian_pos_t c_out{};

  check_status(siderust_to_bodycentric(c_pos, c_params, jd.value(), &c_out),
               "to_bodycentric");

  cartesian::Position<centers::Bodycentric, F, U> result_pos(
      U(c_out.x), U(c_out.y), U(c_out.z));
  return BodycentricPos<F, U>{result_pos, params};
}

// ============================================================================
// BodycentricPos::to_geocentric() — out-of-line member implementation
// ============================================================================

template <typename F, typename U>
inline cartesian::Position<centers::Geocentric, F, U>
BodycentricPos<F, U>::to_geocentric(const JulianDate &jd) const {
  siderust_cartesian_pos_t c_pos = pos.to_c();
  SiderustBodycentricParams c_params = params.to_c();
  siderust_cartesian_pos_t c_out{};

  check_status(siderust_from_bodycentric(c_pos, c_params, jd.value(), &c_out),
               "from_bodycentric");

  return cartesian::Position<centers::Geocentric, F, U>(U(c_out.x), U(c_out.y),
                                                        U(c_out.z));
}

// ============================================================================
// kepler_position() — Keplerian orbital propagation
// ============================================================================

/**
 * @brief Compute an orbital position at a given Julian Date via Kepler's laws.
 *
 * Returns the body's position in the EclipticMeanJ2000 frame in AU.
 * The reference center of the returned position equals the orbit's own
 * reference center (e.g. heliocentric for a planet's orbit).
 *
 * @tparam C  Desired center tag for the result (caller must know from context,
 *            e.g. `centers::Geocentric` for a satellite orbit).
 * @param orbit  Keplerian orbital elements.
 * @param jd     Julian Date.
 * @return Position in EclipticMeanJ2000/AU with center C.
 */
template <typename C = centers::Heliocentric>
inline cartesian::Position<C, frames::EclipticMeanJ2000, qtty::AstronomicalUnit>
kepler_position(const Orbit &orbit, const JulianDate &jd) {
  static_assert(centers::is_center_v<C>,
                "C must be a valid center tag (default: Heliocentric)");
  siderust_cartesian_pos_t c_out{};
  check_status(siderust_kepler_position(orbit.to_c(), jd.value(), &c_out),
               "kepler_position");
  return cartesian::Position<C, frames::EclipticMeanJ2000,
                             qtty::AstronomicalUnit>(
      qtty::AstronomicalUnit(c_out.x), qtty::AstronomicalUnit(c_out.y),
      qtty::AstronomicalUnit(c_out.z));
}

} // namespace siderust
