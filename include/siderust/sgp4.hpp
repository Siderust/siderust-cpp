#pragma once

/**
 * @file sgp4.hpp
 * @brief TLE parsing and SGP4 propagator C++ wrappers.
 *
 * RAII wrappers around the `siderust_tle_*` and `siderust_sgp4_*` FFI functions.
 *
 * ### Typical usage
 * @code
 * #include <siderust/sgp4.hpp>
 *
 * auto tle = siderust::tle::Tle::parse(line1, line2);
 * auto prop = siderust::sgp4::Propagator(tle);
 * double epoch_jd = prop.epoch_jd_utc();
 * auto state = prop.propagate_at(epoch_jd + 360.0 / 1440.0);
 * // state.pos_km[3]  — TEME position, km
 * // state.vel_kms[3] — TEME velocity, km/s
 * @endcode
 */

#include "ffi_core.hpp"

#include <cstdint>
#include <string_view>
#include <utility>

namespace siderust {

/// @defgroup sgp4 TLE / SGP4
/// @{

// ============================================================================
// TLE handle
// ============================================================================

namespace tle {

/**
 * @brief Move-only RAII wrapper around a parsed Two-Line Element set.
 *
 * Construct via `Tle::parse(line1, line2)`.
 */
class Tle {
public:
  /// Parse a two-line element set.
  ///
  /// @param line1  TLE line 1 (null termination added internally).
  /// @param line2  TLE line 2 (null termination added internally).
  /// @throws siderust::InvalidArgumentError on parse failure.
  static Tle parse(std::string_view line1, std::string_view line2) {
    const std::string l1{line1};
    const std::string l2{line2};
    SiderustTle      *handle = nullptr;
    check_status(siderust_tle_parse(l1.c_str(), l2.c_str(), &handle), "tle::Tle::parse");
    return Tle{handle};
  }

  Tle(Tle &&other) noexcept : handle_(std::exchange(other.handle_, nullptr)) {}
  Tle &operator=(Tle &&other) noexcept {
    if (this != &other) {
      siderust_tle_free(handle_);
      handle_ = std::exchange(other.handle_, nullptr);
    }
    return *this;
  }

  Tle(const Tle &)            = delete;
  Tle &operator=(const Tle &) = delete;

  ~Tle() { siderust_tle_free(handle_); }

  /// Return the NORAD catalog number.
  std::uint32_t norad_id() const {
    std::uint32_t id = 0;
    check_status(siderust_tle_norad_id(handle_, &id), "tle::Tle::norad_id");
    return id;
  }

  /// Expose the raw opaque handle (for use by Propagator constructor).
  const SiderustTle *raw() const noexcept { return handle_; }

private:
  explicit Tle(SiderustTle *h) : handle_(h) {}
  SiderustTle *handle_;
};

} // namespace tle

// ============================================================================
// SGP4 propagator
// ============================================================================

namespace sgp4 {

/// Gravity model selector.
enum class GravityModel : int {
  Wgs72    = 0, ///< WGS-72 / AFSPC (default; matches TLE conventions).
  Wgs72Iau = 1, ///< WGS-72 / IAU sidereal time.
  Wgs84    = 2, ///< WGS-84 / IAU sidereal time.
};

/// Propagated TEME state vector.
struct State {
  double pos_km[3];  ///< Position [x, y, z] in km (TEME frame).
  double vel_kms[3]; ///< Velocity [vx, vy, vz] in km/s (TEME frame).
};

/**
 * @brief Move-only RAII wrapper around an initialised SGP4 propagator.
 *
 * Construct from a `Tle` handle; the TLE is not consumed.
 */
class Propagator {
public:
  /// Create an SGP4 propagator from a parsed TLE.
  ///
  /// @param tle    Parsed TLE handle.
  /// @param model  Gravity model (default: WGS-72).
  /// @throws siderust::InvalidArgumentError on initialisation failure.
  explicit Propagator(const tle::Tle &tle, GravityModel model = GravityModel::Wgs72) {
    check_status(siderust_sgp4_new(tle.raw(), static_cast<int>(model), &handle_),
                 "sgp4::Propagator");
  }

  Propagator(Propagator &&other) noexcept : handle_(std::exchange(other.handle_, nullptr)) {}
  Propagator &operator=(Propagator &&other) noexcept {
    if (this != &other) {
      siderust_sgp4_free(handle_);
      handle_ = std::exchange(other.handle_, nullptr);
    }
    return *this;
  }

  Propagator(const Propagator &)            = delete;
  Propagator &operator=(const Propagator &) = delete;

  ~Propagator() { siderust_sgp4_free(handle_); }

  /// Return the UTC Julian date of the TLE epoch.
  double epoch_jd_utc() const {
    double jd = 0.0;
    check_status(siderust_sgp4_epoch_jd_utc(handle_, &jd), "sgp4::Propagator::epoch_jd_utc");
    return jd;
  }

  /// Return the gravity model used by this propagator (0/1/2).
  int gravity_model() const {
    int m = 0;
    check_status(siderust_sgp4_gravity_model(handle_, &m), "sgp4::Propagator::gravity_model");
    return m;
  }

  /// Propagate to a UTC Julian date and return the TEME state.
  ///
  /// @param jd_utc  Target epoch as a UTC Julian date (days).
  /// @throws siderust::InvalidArgumentError on propagation failure.
  State propagate_at(double jd_utc) const {
    State s{};
    check_status(siderust_sgp4_propagate_at(handle_, jd_utc, s.pos_km, s.vel_kms),
                 "sgp4::Propagator::propagate_at");
    return s;
  }

private:
  SiderustSgp4 *handle_ = nullptr;
};

} // namespace sgp4

/// @}

} // namespace siderust
