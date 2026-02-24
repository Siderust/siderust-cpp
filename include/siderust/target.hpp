#pragma once

/**
 * @file target.hpp
 * @brief RAII C++ wrapper for an siderust Target (fixed ICRS pointing).
 *
 * A `Target` represents a fixed celestial direction (RA, Dec at a given epoch)
 * and exposes altitude and azimuth computations via the same observer/window
 * API as the sun/moon/star helpers in altitude.hpp and azimuth.hpp.
 *
 * Proper motion is stored for future use but presently not applied during
 * altitude/azimuth queries (epoch-fixed direction is used throughout).
 */

#include "altitude.hpp"
#include "azimuth.hpp"
#include "coordinates.hpp"
#include "ffi_core.hpp"
#include "time.hpp"
#include "trackable.hpp"
#include <utility>
#include <vector>

namespace siderust {

/**
 * @brief Move-only RAII handle for a siderust target direction.
 *
 * ### Example
 * @code
 * siderust::Target vega(279.2348, +38.7836, 2451545.0); // Vega at J2000
 * auto alt = vega.altitude_at(obs, now);
 * @endcode
 */
class Target : public Trackable {
  public:
    // ------------------------------------------------------------------
    // Construction / destruction
    // ------------------------------------------------------------------

    /**
     * @brief Create a target from ICRS [RA, Dec] and an epoch.
     *
     * @param ra_deg   Right ascension, degrees [0, 360).
     * @param dec_deg  Declination, degrees [−90, +90].
     * @param epoch_jd Julian Date of the coordinate epoch (default J2000.0).
     */
    explicit Target(double ra_deg, double dec_deg, double epoch_jd = 2451545.0) {
        SiderustTarget* h = nullptr;
        check_status(siderust_target_create(ra_deg, dec_deg, epoch_jd, &h),
                     "Target::Target");
        handle_ = h;
    }

    ~Target() {
        if (handle_) {
            siderust_target_free(handle_);
            handle_ = nullptr;
        }
    }

    /// Move constructor.
    Target(Target&& other) noexcept : handle_(other.handle_) {
        other.handle_ = nullptr;
    }

    /// Move assignment.
    Target& operator=(Target&& other) noexcept {
        if (this != &other) {
            if (handle_) {
                siderust_target_free(handle_);
            }
            handle_       = other.handle_;
            other.handle_ = nullptr;
        }
        return *this;
    }

    // Prevent copying (the handle has unique ownership).
    Target(const Target&)            = delete;
    Target& operator=(const Target&) = delete;

    // ------------------------------------------------------------------
    // Coordinate accessors
    // ------------------------------------------------------------------

    /// Right ascension of the target (degrees).
    double ra_deg() const {
        double out{};
        check_status(siderust_target_ra_deg(handle_, &out), "Target::ra_deg");
        return out;
    }

    /// Declination of the target (degrees).
    double dec_deg() const {
        double out{};
        check_status(siderust_target_dec_deg(handle_, &out), "Target::dec_deg");
        return out;
    }

    /// Epoch of the coordinates (Julian Date).
    double epoch_jd() const {
        double out{};
        check_status(siderust_target_epoch_jd(handle_, &out), "Target::epoch_jd");
        return out;
    }

    // ------------------------------------------------------------------
    // Altitude queries
    // ------------------------------------------------------------------

    /**
     * @brief Compute altitude (degrees) at a given MJD instant.
     */
    qtty::Degree altitude_at(const Geodetic& obs, const MJD& mjd) const override {
        double out{};
        check_status(siderust_target_altitude_at(
                         handle_, obs.to_c(), mjd.value(), &out),
                     "Target::altitude_at");
        return qtty::Degree(out);
    }

    /**
     * @brief Find periods when the target is above a threshold altitude.
     */
    std::vector<Period> above_threshold(
        const Geodetic& obs, const Period& window,
        qtty::Degree threshold, const SearchOptions& opts = {}) const override {
        tempoch_period_mjd_t* ptr   = nullptr;
        uintptr_t              count = 0;
        check_status(siderust_target_above_threshold(
                         handle_, obs.to_c(), window.c_inner(),
                         threshold.value(), opts.to_c(), &ptr, &count),
                     "Target::above_threshold");
        return detail_periods_from_c(ptr, count);
    }

    /**
     * @brief Backward-compatible [start, end] overload.
     */
    std::vector<Period> above_threshold(
        const Geodetic& obs, const MJD& start, const MJD& end,
        qtty::Degree threshold, const SearchOptions& opts = {}) const {
        return above_threshold(obs, Period(start, end), threshold, opts);
    }

    /**
     * @brief Find periods when the target is below a threshold altitude.
     */
    std::vector<Period> below_threshold(
        const Geodetic& obs, const Period& window,
        qtty::Degree threshold, const SearchOptions& opts = {}) const override {
        // Target wraps an ICRS direction; use icrs_below_threshold FFI.
        siderust_spherical_dir_t dir_c{};
        dir_c.polar_deg  = dec_deg();
        dir_c.azimuth_deg = ra_deg();
        dir_c.frame      = SIDERUST_FRAME_T_ICRS;
        tempoch_period_mjd_t* ptr   = nullptr;
        uintptr_t             count = 0;
        check_status(siderust_icrs_below_threshold(
                         dir_c, obs.to_c(), window.c_inner(),
                         threshold.value(), opts.to_c(), &ptr, &count),
                     "Target::below_threshold");
        return detail_periods_from_c(ptr, count);
    }

    /**
     * @brief Backward-compatible [start, end] overload.
     */
    std::vector<Period> below_threshold(
        const Geodetic& obs, const MJD& start, const MJD& end,
        qtty::Degree threshold, const SearchOptions& opts = {}) const {
        return below_threshold(obs, Period(start, end), threshold, opts);
    }

    /**
     * @brief Find threshold-crossing events (rising / setting).
     */
    std::vector<CrossingEvent> crossings(
        const Geodetic& obs, const Period& window,
        qtty::Degree threshold, const SearchOptions& opts = {}) const override {
        siderust_crossing_event_t* ptr   = nullptr;
        uintptr_t                  count = 0;
        check_status(siderust_target_crossings(
                         handle_, obs.to_c(), window.c_inner(),
                         threshold.value(), opts.to_c(), &ptr, &count),
                     "Target::crossings");
        return detail::crossings_from_c(ptr, count);
    }

    /**
     * @brief Backward-compatible [start, end] overload.
     */
    std::vector<CrossingEvent> crossings(
        const Geodetic& obs, const MJD& start, const MJD& end,
        qtty::Degree threshold, const SearchOptions& opts = {}) const {
        return crossings(obs, Period(start, end), threshold, opts);
    }

    /**
     * @brief Find culmination (local altitude extremum) events.
     */
    std::vector<CulminationEvent> culminations(
        const Geodetic& obs, const Period& window,
        const SearchOptions& opts = {}) const override {
        siderust_culmination_event_t* ptr   = nullptr;
        uintptr_t                     count = 0;
        check_status(siderust_target_culminations(
                         handle_, obs.to_c(), window.c_inner(),
                         opts.to_c(), &ptr, &count),
                     "Target::culminations");
        return detail::culminations_from_c(ptr, count);
    }

    /**
     * @brief Backward-compatible [start, end] overload.
     */
    std::vector<CulminationEvent> culminations(
        const Geodetic& obs, const MJD& start, const MJD& end,
        const SearchOptions& opts = {}) const {
        return culminations(obs, Period(start, end), opts);
    }

    // ------------------------------------------------------------------
    // Azimuth queries
    // ------------------------------------------------------------------

    /**
     * @brief Compute azimuth (degrees, N-clockwise) at a given MJD instant.
     */
    qtty::Degree azimuth_at(const Geodetic& obs, const MJD& mjd) const override {
        double out{};
        check_status(siderust_target_azimuth_at(
                         handle_, obs.to_c(), mjd.value(), &out),
                     "Target::azimuth_at");
        return qtty::Degree(out);
    }

    /**
     * @brief Find epochs when the target crosses a given azimuth bearing.
     */
    std::vector<AzimuthCrossingEvent> azimuth_crossings(
        const Geodetic& obs, const Period& window,
        qtty::Degree bearing, const SearchOptions& opts = {}) const override {
        siderust_azimuth_crossing_event_t* ptr   = nullptr;
        uintptr_t                          count = 0;
        check_status(siderust_target_azimuth_crossings(
                         handle_, obs.to_c(), window.c_inner(),
                         bearing.value(), opts.to_c(), &ptr, &count),
                     "Target::azimuth_crossings");
        return detail::az_crossings_from_c(ptr, count);
    }

    /**
     * @brief Backward-compatible [start, end] overload.
     */
    std::vector<AzimuthCrossingEvent> azimuth_crossings(
        const Geodetic& obs, const MJD& start, const MJD& end,
        qtty::Degree bearing, const SearchOptions& opts = {}) const {
        return azimuth_crossings(obs, Period(start, end), bearing, opts);
    }

    /// Access the underlying C handle (advanced use).
    const SiderustTarget* c_handle() const { return handle_; }

  private:
    SiderustTarget* handle_ = nullptr;

    /// Build a Period vector from a tempoch_period_mjd_t* array.
    static std::vector<Period> detail_periods_from_c(
        tempoch_period_mjd_t* ptr, uintptr_t count) {
        std::vector<Period> result;
        result.reserve(count);
        for (uintptr_t i = 0; i < count; ++i) {
            result.push_back(Period(MJD(ptr[i].start_mjd), MJD(ptr[i].end_mjd)));
        }
        siderust_periods_free(ptr, count);
        return result;
    }
};

} // namespace siderust
