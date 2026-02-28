#pragma once

/**
 * @file subject.hpp
 * @brief Unified Subject type — one value to represent any celestial entity.
 *
 * `Subject` is a lightweight tagged value (akin to `std::variant`) that
 * wraps the FFI `siderust_subject_t` struct.  It can carry:
 *
 * | Kind        | Data                                         |
 * |-------------|----------------------------------------------|
 * | `Body`      | `siderust::Body` discriminant                |
 * | `Star`      | borrows an existing `siderust::Star`         |
 * | `Icrs`      | inline `spherical::Direction<frames::ICRS>`  |
 * | `Target`    | borrows an existing `DirectionTarget<…>`     |
 *
 * All unified functions (`altitude_at`, `above_threshold`, …) accept a
 * `Subject` so the caller no longer needs separate `sun::`, `moon::`,
 * `body::`, `star::`, `icrs::`, and target-specific calls.
 *
 * **Lifetime**: when constructing from `Star` or `DirectionTarget`, the
 * `Subject` *borrows* the handle — the original object must outlive it.
 *
 * ### Example
 * @code
 * using namespace siderust;
 *
 * // From a solar-system body
 * Subject sun = Subject::body(Body::Sun);
 * qtty::Degree alt = altitude_at(sun, obs, now);
 *
 * // From a catalog star
 * Star vega = Star::catalog("VEGA");
 * Subject s = Subject::star(vega);
 * auto periods = above_threshold(s, obs, window, qtty::Degree(10));
 *
 * // From an ICRS direction
 * Subject d = Subject::icrs(spherical::Direction<frames::ICRS>(
 *     qtty::Degree(279.23), qtty::Degree(38.78)));
 * @endcode
 */

#include "altitude.hpp"
#include "azimuth.hpp"
#include "bodies.hpp"
#include "body_target.hpp"
#include "coordinates.hpp"
#include "ffi_core.hpp"
#include "target.hpp"
#include "time.hpp"
#include <vector>

namespace siderust {

// ============================================================================
// SubjectKind enum
// ============================================================================

/**
 * @brief Discriminant for the active field in a Subject.
 */
enum class SubjectKind : int32_t {
  Body = SIDERUST_SUBJECT_KIND_T_BODY,
  Star = SIDERUST_SUBJECT_KIND_T_STAR,
  Icrs = SIDERUST_SUBJECT_KIND_T_ICRS,
  Target = SIDERUST_SUBJECT_KIND_T_TARGET,
};

// ============================================================================
// Subject value type
// ============================================================================

/**
 * @brief Unified, lightweight handle representing any celestial subject.
 *
 * Subject is a small copyable value type that stores a discriminant and
 * either an inline body enum, an inline ICRS direction, or a borrowed
 * pointer to a `Star` or `DirectionTarget` handle.
 *
 * Use the static factory methods to construct instances.
 */
class Subject {
public:
  // -- Factories --------------------------------------------------------

  /** @brief Create a subject for a solar-system body. */
  static Subject body(Body b) {
    siderust_subject_t s{};
    s.kind = SIDERUST_SUBJECT_KIND_T_BODY;
    s.body = static_cast<SiderustBody>(b);
    return Subject(s);
  }

  /**
   * @brief Create a subject borrowing a `Star` handle.
   * @warning The `Star` must outlive this `Subject`.
   */
  static Subject star(const siderust::Star &star) {
    siderust_subject_t s{};
    s.kind = SIDERUST_SUBJECT_KIND_T_STAR;
    s.star_handle = star.c_handle();
    return Subject(s);
  }

  /**
   * @brief Create a subject for an inline ICRS direction.
   *
   * The direction is stored by value inside the Subject; no external handle
   * is borrowed.
   */
  static Subject icrs(const spherical::Direction<frames::ICRS> &dir) {
    siderust_subject_t s{};
    s.kind = SIDERUST_SUBJECT_KIND_T_ICRS;
    s.icrs_dir = dir.to_c();
    return Subject(s);
  }

  /**
   * @brief Create a subject borrowing an opaque `SiderustTarget` handle.
   *
   * Works with any `DirectionTarget<C>` via its `c_handle()` accessor.
   * @warning The target must outlive this `Subject`.
   */
  template <typename C>
  static Subject target(const DirectionTarget<C> &tgt) {
    siderust_subject_t s{};
    s.kind = SIDERUST_SUBJECT_KIND_T_TARGET;
    s.target_handle = tgt.c_handle();
    return Subject(s);
  }

  // -- Accessors --------------------------------------------------------

  SubjectKind kind() const { return static_cast<SubjectKind>(inner_.kind); }
  const siderust_subject_t &c_inner() const { return inner_; }

private:
  siderust_subject_t inner_{};
  explicit Subject(siderust_subject_t s) : inner_(s) {}
};

// ============================================================================
// Unified free functions
// ============================================================================

/**
 * @brief Altitude at an instant (radians) for any subject.
 */
inline qtty::Radian altitude_at(const Subject &subj, const Geodetic &obs,
                                const MJD &mjd) {
  double out;
  check_status(siderust_altitude_at(subj.c_inner(), obs.to_c(), mjd.value(),
                                    &out),
               "altitude_at(Subject)");
  return qtty::Radian(out);
}

/**
 * @brief Periods when a subject is above a threshold altitude.
 */
inline std::vector<Period> above_threshold(const Subject &subj,
                                           const Geodetic &obs,
                                           const Period &window,
                                           qtty::Degree threshold,
                                           const SearchOptions &opts = {}) {
  tempoch_period_mjd_t *ptr = nullptr;
  uintptr_t count = 0;
  check_status(
      siderust_above_threshold(subj.c_inner(), obs.to_c(), window.c_inner(),
                               threshold.value(), opts.to_c(), &ptr, &count),
      "above_threshold(Subject)");
  return detail::periods_from_c(ptr, count);
}

/**
 * @brief Periods when a subject is below a threshold altitude.
 */
inline std::vector<Period> below_threshold(const Subject &subj,
                                           const Geodetic &obs,
                                           const Period &window,
                                           qtty::Degree threshold,
                                           const SearchOptions &opts = {}) {
  tempoch_period_mjd_t *ptr = nullptr;
  uintptr_t count = 0;
  check_status(
      siderust_below_threshold(subj.c_inner(), obs.to_c(), window.c_inner(),
                               threshold.value(), opts.to_c(), &ptr, &count),
      "below_threshold(Subject)");
  return detail::periods_from_c(ptr, count);
}

/**
 * @brief Threshold-crossing events for a subject.
 */
inline std::vector<CrossingEvent>
crossings(const Subject &subj, const Geodetic &obs, const Period &window,
          qtty::Degree threshold, const SearchOptions &opts = {}) {
  siderust_crossing_event_t *ptr = nullptr;
  uintptr_t count = 0;
  check_status(
      siderust_crossings(subj.c_inner(), obs.to_c(), window.c_inner(),
                         threshold.value(), opts.to_c(), &ptr, &count),
      "crossings(Subject)");
  return detail::crossings_from_c(ptr, count);
}

/**
 * @brief Culmination (local extrema) events for a subject.
 */
inline std::vector<CulminationEvent>
culminations(const Subject &subj, const Geodetic &obs, const Period &window,
             const SearchOptions &opts = {}) {
  siderust_culmination_event_t *ptr = nullptr;
  uintptr_t count = 0;
  check_status(siderust_culminations(subj.c_inner(), obs.to_c(),
                                     window.c_inner(), opts.to_c(), &ptr,
                                     &count),
               "culminations(Subject)");
  return detail::culminations_from_c(ptr, count);
}

/**
 * @brief Periods when a body's altitude is within [min, max].
 *
 * Only valid for `Body` subjects.  Will throw for `Star`/`Icrs`/`Target`.
 */
inline std::vector<Period> altitude_periods(const Subject &subj,
                                            const Geodetic &obs,
                                            const Period &window,
                                            qtty::Degree min_alt,
                                            qtty::Degree max_alt) {
  siderust_altitude_query_t q = {obs.to_c(), window.start().value(),
                                 window.end().value(), min_alt.value(),
                                 max_alt.value()};
  tempoch_period_mjd_t *ptr = nullptr;
  uintptr_t count = 0;
  check_status(siderust_altitude_periods(subj.c_inner(), q, &ptr, &count),
               "altitude_periods(Subject)");
  return detail::periods_from_c(ptr, count);
}

/**
 * @brief Azimuth at an instant (degrees, N-clockwise) for any subject.
 */
inline qtty::Degree azimuth_at(const Subject &subj, const Geodetic &obs,
                               const MJD &mjd) {
  double out;
  check_status(siderust_azimuth_at(subj.c_inner(), obs.to_c(), mjd.value(),
                                   &out),
               "azimuth_at(Subject)");
  return qtty::Degree(out);
}

/**
 * @brief Azimuth bearing-crossing events for a subject.
 */
inline std::vector<AzimuthCrossingEvent>
azimuth_crossings(const Subject &subj, const Geodetic &obs,
                  const Period &window, qtty::Degree bearing,
                  const SearchOptions &opts = {}) {
  siderust_azimuth_crossing_event_t *ptr = nullptr;
  uintptr_t count = 0;
  check_status(siderust_azimuth_crossings(subj.c_inner(), obs.to_c(),
                                          window.c_inner(), bearing.value(),
                                          opts.to_c(), &ptr, &count),
               "azimuth_crossings(Subject)");
  return detail::az_crossings_from_c(ptr, count);
}

/**
 * @brief Azimuth extrema (northernmost / southernmost) for a subject.
 */
inline std::vector<AzimuthExtremum>
azimuth_extrema(const Subject &subj, const Geodetic &obs,
                const Period &window, const SearchOptions &opts = {}) {
  siderust_azimuth_extremum_t *ptr = nullptr;
  uintptr_t count = 0;
  check_status(siderust_azimuth_extrema(subj.c_inner(), obs.to_c(),
                                        window.c_inner(), opts.to_c(), &ptr,
                                        &count),
               "azimuth_extrema(Subject)");
  return detail::az_extrema_from_c(ptr, count);
}

/**
 * @brief Periods when a subject's azimuth is within [min_deg, max_deg].
 */
inline std::vector<Period> in_azimuth_range(const Subject &subj,
                                            const Geodetic &obs,
                                            const Period &window,
                                            qtty::Degree min_deg,
                                            qtty::Degree max_deg,
                                            const SearchOptions &opts = {}) {
  tempoch_period_mjd_t *ptr = nullptr;
  uintptr_t count = 0;
  check_status(siderust_in_azimuth_range(subj.c_inner(), obs.to_c(),
                                         window.c_inner(), min_deg.value(),
                                         max_deg.value(), opts.to_c(), &ptr,
                                         &count),
               "in_azimuth_range(Subject)");
  return detail::periods_from_c(ptr, count);
}

} // namespace siderust
