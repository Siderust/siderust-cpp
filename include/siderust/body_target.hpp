#pragma once

/**
 * @file body_target.hpp
 * @brief Trackable wrapper for solar-system bodies.
 *
 * `BodyTarget` implements the `Trackable` interface for any solar-system
 * body identified by the `Body` enum.  It dispatches altitude and azimuth
 * computations through the siderust-ffi `siderust_body_*` functions, which
 * in turn use VSOP87 (planets), specialised engines (Sun/Moon), or
 * Meeus/Williams series (Pluto) for ephemeris.
 *
 * ### Example
 * @code
 * using namespace siderust;
 * BodyTarget mars(Body::Mars);
 * qtty::Degree alt = mars.altitude_at(obs, now);
 *
 * // Polymorphic usage
 * std::vector<std::unique_ptr<Trackable>> targets;
 * targets.push_back(std::make_unique<BodyTarget>(Body::Sun));
 * targets.push_back(std::make_unique<BodyTarget>(Body::Jupiter));
 * for (const auto& t : targets) {
 *     std::cout << t->altitude_at(obs, now).value() << "\n";
 * }
 * @endcode
 */

#include "altitude.hpp"
#include "azimuth.hpp"
#include "ffi_core.hpp"
#include "trackable.hpp"

namespace siderust {

// ============================================================================
// Body enum
// ============================================================================

/**
 * @brief Identifies a solar-system body for generic altitude/azimuth dispatch.
 *
 * Maps 1:1 to the FFI `SiderustBody` discriminant.
 */
enum class Body : int32_t {
  Sun = SIDERUST_BODY_SUN,
  Moon = SIDERUST_BODY_MOON,
  Mercury = SIDERUST_BODY_MERCURY,
  Venus = SIDERUST_BODY_VENUS,
  Mars = SIDERUST_BODY_MARS,
  Jupiter = SIDERUST_BODY_JUPITER,
  Saturn = SIDERUST_BODY_SATURN,
  Uranus = SIDERUST_BODY_URANUS,
  Neptune = SIDERUST_BODY_NEPTUNE,
};

// ============================================================================
// Free functions in body:: namespace
// ============================================================================

namespace body {

/**
 * @brief Compute a body's altitude (radians) at a given MJD instant.
 */
inline qtty::Radian altitude_at(Body b, const Geodetic &obs, const MJD &mjd) {
  double out;
  check_status(siderust_body_altitude_at(static_cast<SiderustBody>(b),
                                         obs.to_c(), mjd.value(), &out),
               "body::altitude_at");
  return qtty::Radian(out);
}

/**
 * @brief Find periods when a body is above a threshold altitude.
 */
inline std::vector<Period> above_threshold(Body b, const Geodetic &obs,
                                           const Period &window,
                                           qtty::Degree threshold,
                                           const SearchOptions &opts = {}) {
  tempoch_period_mjd_t *ptr = nullptr;
  uintptr_t count = 0;
  check_status(siderust_body_above_threshold(
                   static_cast<SiderustBody>(b), obs.to_c(), window.c_inner(),
                   threshold.value(), opts.to_c(), &ptr, &count),
               "body::above_threshold");
  return detail::periods_from_c(ptr, count);
}

/**
 * @brief Find periods when a body is below a threshold altitude.
 */
inline std::vector<Period> below_threshold(Body b, const Geodetic &obs,
                                           const Period &window,
                                           qtty::Degree threshold,
                                           const SearchOptions &opts = {}) {
  tempoch_period_mjd_t *ptr = nullptr;
  uintptr_t count = 0;
  check_status(siderust_body_below_threshold(
                   static_cast<SiderustBody>(b), obs.to_c(), window.c_inner(),
                   threshold.value(), opts.to_c(), &ptr, &count),
               "body::below_threshold");
  return detail::periods_from_c(ptr, count);
}

/**
 * @brief Find threshold-crossing events for a body.
 */
inline std::vector<CrossingEvent> crossings(Body b, const Geodetic &obs,
                                            const Period &window,
                                            qtty::Degree threshold,
                                            const SearchOptions &opts = {}) {
  siderust_crossing_event_t *ptr = nullptr;
  uintptr_t count = 0;
  check_status(siderust_body_crossings(static_cast<SiderustBody>(b), obs.to_c(),
                                       window.c_inner(), threshold.value(),
                                       opts.to_c(), &ptr, &count),
               "body::crossings");
  return detail::crossings_from_c(ptr, count);
}

/**
 * @brief Find culmination events for a body.
 */
inline std::vector<CulminationEvent>
culminations(Body b, const Geodetic &obs, const Period &window,
             const SearchOptions &opts = {}) {
  siderust_culmination_event_t *ptr = nullptr;
  uintptr_t count = 0;
  check_status(siderust_body_culminations(static_cast<SiderustBody>(b),
                                          obs.to_c(), window.c_inner(),
                                          opts.to_c(), &ptr, &count),
               "body::culminations");
  return detail::culminations_from_c(ptr, count);
}

/**
 * @brief Find periods when a body's altitude is within [min, max].
 */
inline std::vector<Period> altitude_periods(Body b, const Geodetic &obs,
                                            const Period &window,
                                            qtty::Degree min_alt,
                                            qtty::Degree max_alt) {
  siderust_altitude_query_t q = {obs.to_c(), window.start().value(),
                                 window.end().value(), min_alt.value(),
                                 max_alt.value()};
  tempoch_period_mjd_t *ptr = nullptr;
  uintptr_t count = 0;
  check_status(siderust_body_altitude_periods(static_cast<SiderustBody>(b), q,
                                              &ptr, &count),
               "body::altitude_periods");
  return detail::periods_from_c(ptr, count);
}

} // namespace body

namespace body {

// ── Azimuth free functions ──────────────────────────────────────────────

/**
 * @brief Compute a body's azimuth (radians) at a given MJD instant.
 */
inline qtty::Radian azimuth_at(Body b, const Geodetic &obs, const MJD &mjd) {
  double out;
  check_status(siderust_body_azimuth_at(static_cast<SiderustBody>(b),
                                        obs.to_c(), mjd.value(), &out),
               "body::azimuth_at");
  return qtty::Radian(out);
}

/**
 * @brief Find azimuth-bearing crossing events for a body.
 */
inline std::vector<AzimuthCrossingEvent>
azimuth_crossings(Body b, const Geodetic &obs, const Period &window,
                  qtty::Degree bearing, const SearchOptions &opts = {}) {
  siderust_azimuth_crossing_event_t *ptr = nullptr;
  uintptr_t count = 0;
  check_status(siderust_body_azimuth_crossings(
                   static_cast<SiderustBody>(b), obs.to_c(), window.c_inner(),
                   bearing.value(), opts.to_c(), &ptr, &count),
               "body::azimuth_crossings");
  return detail::az_crossings_from_c(ptr, count);
}

/**
 * @brief Find azimuth extrema (northernmost/southernmost bearing) for a body.
 */
inline std::vector<AzimuthExtremum>
azimuth_extrema(Body b, const Geodetic &obs, const Period &window,
                const SearchOptions &opts = {}) {
  siderust_azimuth_extremum_t *ptr = nullptr;
  uintptr_t count = 0;
  check_status(siderust_body_azimuth_extrema(static_cast<SiderustBody>(b),
                                             obs.to_c(), window.c_inner(),
                                             opts.to_c(), &ptr, &count),
               "body::azimuth_extrema");
  return detail::az_extrema_from_c(ptr, count);
}

/**
 * @brief Find periods when a body's azimuth is within [min, max].
 */
inline std::vector<Period> in_azimuth_range(Body b, const Geodetic &obs,
                                            const Period &window,
                                            qtty::Degree min, qtty::Degree max,
                                            const SearchOptions &opts = {}) {
  tempoch_period_mjd_t *ptr = nullptr;
  uintptr_t count = 0;
  check_status(siderust_body_in_azimuth_range(
                   static_cast<SiderustBody>(b), obs.to_c(), window.c_inner(),
                   min.value(), max.value(), opts.to_c(), &ptr, &count),
               "body::in_azimuth_range");
  return detail::periods_from_c(ptr, count);
}

} // namespace body

// ============================================================================
// BodyTarget — Trackable adapter for solar-system bodies
// ============================================================================

/**
 * @brief Trackable adapter for solar-system bodies.
 *
 * Wraps a `Body` enum value and dispatches all altitude/azimuth queries
 * through the FFI `siderust_body_*` functions.
 *
 * `BodyTarget` is lightweight (holds a single enum value), copyable, and
 * can be used directly or stored as `std::unique_ptr<Trackable>` for
 * polymorphic dispatch.
 */
class BodyTarget : public Trackable {
public:
  /**
   * @brief Construct a BodyTarget for a given solar-system body.
   * @param body The body to track.
   */
  explicit BodyTarget(Body body) : body_(body) {}

  // ------------------------------------------------------------------
  // Altitude queries
  // ------------------------------------------------------------------

  qtty::Degree altitude_at(const Geodetic &obs, const MJD &mjd) const override {
    auto rad = body::altitude_at(body_, obs, mjd);
    return qtty::Degree(rad.value() * 180.0 / 3.14159265358979323846);
  }

  std::vector<Period>
  above_threshold(const Geodetic &obs, const Period &window,
                  qtty::Degree threshold,
                  const SearchOptions &opts = {}) const override {
    return body::above_threshold(body_, obs, window, threshold, opts);
  }

  std::vector<Period>
  below_threshold(const Geodetic &obs, const Period &window,
                  qtty::Degree threshold,
                  const SearchOptions &opts = {}) const override {
    return body::below_threshold(body_, obs, window, threshold, opts);
  }

  std::vector<CrossingEvent>
  crossings(const Geodetic &obs, const Period &window, qtty::Degree threshold,
            const SearchOptions &opts = {}) const override {
    return body::crossings(body_, obs, window, threshold, opts);
  }

  std::vector<CulminationEvent>
  culminations(const Geodetic &obs, const Period &window,
               const SearchOptions &opts = {}) const override {
    return body::culminations(body_, obs, window, opts);
  }

  // ------------------------------------------------------------------
  // Azimuth queries
  // ------------------------------------------------------------------

  qtty::Degree azimuth_at(const Geodetic &obs, const MJD &mjd) const override {
    auto rad = body::azimuth_at(body_, obs, mjd);
    return qtty::Degree(rad.value() * 180.0 / 3.14159265358979323846);
  }

  std::vector<AzimuthCrossingEvent>
  azimuth_crossings(const Geodetic &obs, const Period &window,
                    qtty::Degree bearing,
                    const SearchOptions &opts = {}) const override {
    return body::azimuth_crossings(body_, obs, window, bearing, opts);
  }

  /// Access the underlying Body enum value.
  Body body() const { return body_; }

private:
  Body body_;
};

} // namespace siderust
