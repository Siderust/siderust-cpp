#pragma once

/**
 * @file target.hpp
 * @brief Strongly-typed fixed-direction Target for any supported frame.
 *
 * `DirectionTarget<C>` represents a fixed celestial direction (star, galaxy,
 * or any user-defined sky coordinate) in any supported reference frame and
 * exposes altitude/azimuth computations via the same observer/window API as
 * the body helpers in altitude.hpp and azimuth.hpp.
 *
 * It is one concrete implementation of `Target` (the common abstract base).
 * For moving solar-system bodies use `BodyTarget`; for catalog stars use
 * `StarTarget`.
 *
 * The template parameter `C` must be an instantiation of
 * `spherical::Direction<F>` for a frame `F` that can be transformed to ICRS
 * (i.e., `frames::has_frame_transform_v<F, frames::ICRS>` must be true).
 * Non-ICRS directions are silently converted to ICRS at construction; the
 * original typed direction is retained as C++ state.
 *
 * Supported frames:
 *   - `frames::ICRS`, `frames::ICRF`
 *   - `frames::EquatorialMeanJ2000`, `frames::EquatorialMeanOfDate`,
 *     `frames::EquatorialTrueOfDate`
 *   - `frames::EclipticMeanJ2000`
 *
 * Convenience aliases:
 *   - `ICRSTarget`, `ICRFTarget`
 *   - `EquatorialMeanJ2000Target`, `EquatorialMeanOfDateTarget`,
 *     `EquatorialTrueOfDateTarget`
 *   - `EclipticMeanJ2000Target`
 */

#include "altitude.hpp"
#include "azimuth.hpp"
#include "coordinates.hpp"
#include "ffi_core.hpp"
#include "time.hpp"
#include "trackable.hpp"
#include <sstream>
#include <string>
#include <type_traits>
#include <utility>
#include <vector>

namespace siderust {

// ============================================================================
// Internal type traits
// ============================================================================

namespace detail {

/// @cond INTERNAL

/// True iff T is an instantiation of spherical::Direction<F>.
template <typename T> struct is_spherical_direction : std::false_type {};

template <typename F>
struct is_spherical_direction<spherical::Direction<F>> : std::true_type {};

template <typename T>
inline constexpr bool is_spherical_direction_v =
    is_spherical_direction<T>::value;

/// Extract the frame tag F from spherical::Direction<F>.
template <typename T> struct spherical_direction_frame; // undefined primary

template <typename F>
struct spherical_direction_frame<spherical::Direction<F>> {
  using type = F;
};

template <typename T>
using spherical_direction_frame_t = typename spherical_direction_frame<T>::type;

/// @endcond

} // namespace detail

// ============================================================================
// Target<C>
// ============================================================================

/**
 * @brief Fixed celestial direction target — a `Target` for a specific sky
 * position.
 *
 * @tparam C  Spherical direction type (e.g. `spherical::direction::ICRS`).
 *
 * ### Example — ICRS target (Vega at J2000)
 * @code
 * using namespace siderust;
 * ICRSTarget vega{ spherical::direction::ICRS{ 279.2348_deg, +38.7836_deg } };
 * auto alt = vega.altitude_at(obs, now);   // → qtty::Degree
 * std::cout << vega.name() << "\n";        // "ICRS(279.23°, 38.78°)"
 * std::cout << vega.ra() << "\n";          // qtty::Degree (equatorial frames)
 * @endcode
 *
 * ### Example — named ICRS target
 * @code
 * ICRSTarget vega{ spherical::direction::ICRS{ 279.2348_deg, +38.7836_deg },
 *                  JulianDate::J2000(), "Vega" };
 * std::cout << vega.name() << "\n";  // "Vega"
 * @endcode
 *
 * ### Example — Ecliptic target (auto-converted to ICRS internally)
 * @code
 * EclipticMeanJ2000Target ec{
 *     spherical::direction::EclipticMeanJ2000{ 246.2_deg, 59.2_deg } };
 * auto alt = ec.altitude_at(obs, now);
 * @endcode
 */
template <typename C> class DirectionTarget : public Target {

  static_assert(detail::is_spherical_direction_v<C>,
                "Target<C>: C must be a specialisation of "
                "siderust::spherical::Direction<F>");

  using Frame = detail::spherical_direction_frame_t<C>;

  static_assert(
      frames::has_frame_transform_v<Frame, frames::ICRS>,
      "Target<C>: frame F must support a transform to ICRS "
      "(frames::has_frame_transform_v<F, frames::ICRS> must be true). "
      "Supported frames: ICRS, ICRF, EquatorialMeanJ2000, "
      "EquatorialMeanOfDate, EquatorialTrueOfDate, EclipticMeanJ2000.");

public:
  // ------------------------------------------------------------------
  // Construction / destruction
  // ------------------------------------------------------------------

  /**
   * @brief Construct from a strongly-typed spherical direction.
   *
   * For frames other than ICRS, the direction is converted to ICRS before
   * being registered with the Rust FFI.  The original `C` direction is
   * retained for C++-side accessors.
   *
   * @param dir    Spherical direction (any supported frame).
   * @param epoch  Coordinate epoch (default J2000.0).
   * @param label  Optional human-readable name.  If empty, a default
   *               "Frame(lon°, lat°)" string is generated from the direction.
   */
  explicit DirectionTarget(C dir, JulianDate epoch = JulianDate::J2000(),
                           std::string label = "")
      : m_dir_(dir), m_epoch_(epoch), label_(std::move(label)) {
    // Convert to ICRS for the FFI; identity transform when already ICRS.
    if constexpr (std::is_same_v<Frame, frames::ICRS>) {
      m_icrs_ = dir;
    } else {
      m_icrs_ = dir.template to_frame<frames::ICRS>(epoch);
    }
    SiderustTarget *h = nullptr;
    check_status(siderust_target_create(m_icrs_.ra().value(),
                                        m_icrs_.dec().value(), epoch.value(),
                                        &h),
                 "Target::Target");
    handle_ = h;
  }

  ~DirectionTarget() {
    if (handle_) {
      siderust_target_free(handle_);
      handle_ = nullptr;
    }
  }

  /// Move constructor.
  DirectionTarget(DirectionTarget &&other) noexcept
      : m_dir_(std::move(other.m_dir_)), m_epoch_(other.m_epoch_),
        m_icrs_(other.m_icrs_), label_(std::move(other.label_)),
        handle_(other.handle_) {
    other.handle_ = nullptr;
  }

  /// Move assignment.
  DirectionTarget &operator=(DirectionTarget &&other) noexcept {
    if (this != &other) {
      if (handle_) {
        siderust_target_free(handle_);
      }
      m_dir_ = std::move(other.m_dir_);
      m_epoch_ = other.m_epoch_;
      m_icrs_ = other.m_icrs_;
      label_ = std::move(other.label_);
      handle_ = other.handle_;
      other.handle_ = nullptr;
    }
    return *this;
  }

  // Prevent copying (the handle has unique ownership).
  DirectionTarget(const DirectionTarget &) = delete;
  DirectionTarget &operator=(const DirectionTarget &) = delete;

  // ------------------------------------------------------------------
  // Identity (implements Target)
  // ------------------------------------------------------------------

  /**
   * @brief Human-readable name for this direction target.
   *
   * Returns the label passed at construction if one was provided; otherwise
   * generates a "Frame(lon°, lat°)" string from the ICRS direction.
   */
  std::string name() const override {
    if (!label_.empty())
      return label_;
    std::ostringstream ss;
    ss << "Direction(" << m_icrs_.ra().value() << "\xc2\xb0, "
       << m_icrs_.dec().value() << "\xc2\xb0)";
    return ss.str();
  }

  // ------------------------------------------------------------------
  // Coordinate accessors
  // ------------------------------------------------------------------

  /// The original typed direction as supplied at construction.
  const C &direction() const { return m_dir_; }

  /// Epoch of the coordinate.
  JulianDate epoch() const { return m_epoch_; }

  /// The ICRS direction used for FFI calls (equals `direction()` when C is
  /// already `spherical::direction::ICRS`).
  const spherical::direction::ICRS &icrs_direction() const { return m_icrs_; }

  /// Right ascension — only available for equatorial frames (RA/Dec).
  template <typename F_ = Frame,
            std::enable_if_t<frames::has_ra_dec_v<F_>, int> = 0>
  qtty::Degree ra() const {
    return m_dir_.ra();
  }

  /// Declination — only available for equatorial frames (RA/Dec).
  template <typename F_ = Frame,
            std::enable_if_t<frames::has_ra_dec_v<F_>, int> = 0>
  qtty::Degree dec() const {
    return m_dir_.dec();
  }

  // ------------------------------------------------------------------
  // Altitude queries (implements Trackable)
  // ------------------------------------------------------------------

  /**
   * @brief Compute altitude (degrees) at a given MJD instant.
   *
   * @note The Rust FFI returns radians; this method converts to degrees.
   */
  qtty::Degree altitude_at(const Geodetic &obs, const MJD &mjd) const override {
    double out{};
    check_status(
        siderust_target_altitude_at(handle_, obs.to_c(), mjd.value(), &out),
        "Target::altitude_at");
    return qtty::Radian(out).to<qtty::Degree>();
  }

  /**
   * @brief Find periods when the target is above a threshold altitude.
   */
  std::vector<Period>
  above_threshold(const Geodetic &obs, const Period &window,
                  qtty::Degree threshold,
                  const SearchOptions &opts = {}) const override {
    tempoch_period_mjd_t *ptr = nullptr;
    uintptr_t count = 0;
    check_status(siderust_target_above_threshold(
                     handle_, obs.to_c(), window.c_inner(), threshold.value(),
                     opts.to_c(), &ptr, &count),
                 "Target::above_threshold");
    return detail_periods_from_c(ptr, count);
  }

  /// Backward-compatible [start, end] overload.
  std::vector<Period> above_threshold(const Geodetic &obs, const MJD &start,
                                      const MJD &end, qtty::Degree threshold,
                                      const SearchOptions &opts = {}) const {
    return above_threshold(obs, Period(start, end), threshold, opts);
  }

  /**
   * @brief Find periods when the target is below a threshold altitude.
   */
  std::vector<Period>
  below_threshold(const Geodetic &obs, const Period &window,
                  qtty::Degree threshold,
                  const SearchOptions &opts = {}) const override {
    tempoch_period_mjd_t *ptr = nullptr;
    uintptr_t count = 0;
    check_status(siderust_target_below_threshold(
                     handle_, obs.to_c(), window.c_inner(), threshold.value(),
                     opts.to_c(), &ptr, &count),
                 "Target::below_threshold");
    return detail_periods_from_c(ptr, count);
  }

  /// Backward-compatible [start, end] overload.
  std::vector<Period> below_threshold(const Geodetic &obs, const MJD &start,
                                      const MJD &end, qtty::Degree threshold,
                                      const SearchOptions &opts = {}) const {
    return below_threshold(obs, Period(start, end), threshold, opts);
  }

  /**
   * @brief Find threshold-crossing events (rising / setting).
   */
  std::vector<CrossingEvent>
  crossings(const Geodetic &obs, const Period &window, qtty::Degree threshold,
            const SearchOptions &opts = {}) const override {
    siderust_crossing_event_t *ptr = nullptr;
    uintptr_t count = 0;
    check_status(siderust_target_crossings(handle_, obs.to_c(),
                                           window.c_inner(), threshold.value(),
                                           opts.to_c(), &ptr, &count),
                 "Target::crossings");
    return detail::crossings_from_c(ptr, count);
  }

  /// Backward-compatible [start, end] overload.
  std::vector<CrossingEvent> crossings(const Geodetic &obs, const MJD &start,
                                       const MJD &end, qtty::Degree threshold,
                                       const SearchOptions &opts = {}) const {
    return crossings(obs, Period(start, end), threshold, opts);
  }

  /**
   * @brief Find culmination (local altitude extremum) events.
   */
  std::vector<CulminationEvent>
  culminations(const Geodetic &obs, const Period &window,
               const SearchOptions &opts = {}) const override {
    siderust_culmination_event_t *ptr = nullptr;
    uintptr_t count = 0;
    check_status(siderust_target_culminations(handle_, obs.to_c(),
                                              window.c_inner(), opts.to_c(),
                                              &ptr, &count),
                 "Target::culminations");
    return detail::culminations_from_c(ptr, count);
  }

  /// Backward-compatible [start, end] overload.
  std::vector<CulminationEvent>
  culminations(const Geodetic &obs, const MJD &start, const MJD &end,
               const SearchOptions &opts = {}) const {
    return culminations(obs, Period(start, end), opts);
  }

  // ------------------------------------------------------------------
  // Azimuth queries (implements Trackable)
  // ------------------------------------------------------------------

  /**
   * @brief Compute azimuth (degrees, N-clockwise) at a given MJD instant.
   */
  qtty::Degree azimuth_at(const Geodetic &obs, const MJD &mjd) const override {
    double out{};
    check_status(
        siderust_target_azimuth_at(handle_, obs.to_c(), mjd.value(), &out),
        "Target::azimuth_at");
    return qtty::Degree(out);
  }

  /**
   * @brief Find epochs when the target crosses a given azimuth bearing.
   */
  std::vector<AzimuthCrossingEvent>
  azimuth_crossings(const Geodetic &obs, const Period &window,
                    qtty::Degree bearing,
                    const SearchOptions &opts = {}) const override {
    siderust_azimuth_crossing_event_t *ptr = nullptr;
    uintptr_t count = 0;
    check_status(siderust_target_azimuth_crossings(
                     handle_, obs.to_c(), window.c_inner(), bearing.value(),
                     opts.to_c(), &ptr, &count),
                 "Target::azimuth_crossings");
    return detail::az_crossings_from_c(ptr, count);
  }

  /// Backward-compatible [start, end] overload.
  std::vector<AzimuthCrossingEvent>
  azimuth_crossings(const Geodetic &obs, const MJD &start, const MJD &end,
                    qtty::Degree bearing,
                    const SearchOptions &opts = {}) const {
    return azimuth_crossings(obs, Period(start, end), bearing, opts);
  }

  /// Access the underlying C handle (advanced use).
  const SiderustTarget *c_handle() const { return handle_; }

private:
  C m_dir_;
  JulianDate m_epoch_;
  spherical::direction::ICRS m_icrs_;
  std::string label_;
  SiderustTarget *handle_ = nullptr;

  /// Build a Period vector from a tempoch_period_mjd_t* array.
  static std::vector<Period> detail_periods_from_c(tempoch_period_mjd_t *ptr,
                                                   uintptr_t count) {
    std::vector<Period> result;
    result.reserve(count);
    for (uintptr_t i = 0; i < count; ++i) {
      result.push_back(Period(MJD(ptr[i].start_mjd), MJD(ptr[i].end_mjd)));
    }
    siderust_periods_free(ptr, count);
    return result;
  }
};

// ============================================================================
// Convenience type aliases
// ============================================================================

/// Fixed direction in ICRS (most common use-case).
using ICRSTarget = DirectionTarget<spherical::direction::ICRS>;

/// Fixed direction in ICRF (treated identically to ICRS in Siderust).
using ICRFTarget = DirectionTarget<spherical::direction::ICRF>;

/// Fixed direction in mean equatorial coordinates of J2000.0 (FK5).
using EquatorialMeanJ2000Target =
    DirectionTarget<spherical::direction::EquatorialMeanJ2000>;

/// Fixed direction in mean equatorial coordinates of date (precessed only).
using EquatorialMeanOfDateTarget =
    DirectionTarget<spherical::direction::EquatorialMeanOfDate>;

/// Fixed direction in true equatorial coordinates of date (precessed +
/// nutated).
using EquatorialTrueOfDateTarget =
    DirectionTarget<spherical::direction::EquatorialTrueOfDate>;

/// Fixed direction in mean ecliptic coordinates of J2000.0.
using EclipticMeanJ2000Target =
    DirectionTarget<spherical::direction::EclipticMeanJ2000>;

} // namespace siderust
