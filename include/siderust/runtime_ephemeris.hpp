#pragma once

/**
 * @file runtime_ephemeris.hpp
 * @brief C++ wrapper for siderust's runtime-loaded JPL DE4xx ephemeris.
 *
 * Provides an RAII `RuntimeEphemeris` class that loads a BSP file at runtime
 * and exposes the same position queries as the compile-time VSOP87 wrappers
 * in `ephemeris.hpp`.
 *
 * @code
 * #include <siderust/runtime_ephemeris.hpp>
 *
 * siderust::RuntimeEphemeris eph("/path/to/de440.bsp");
 * auto sun = eph.sun_barycentric(jd);
 * @endcode
 */

#include "coordinates.hpp"
#include "ffi_core.hpp"
#include "time.hpp"

#include <cstdint>
#include <memory>
#include <string>

namespace siderust {

/**
 * @brief Cartesian velocity in AU/day, tagged by reference frame.
 *
 * Returned by `RuntimeEphemeris::earth_barycentric_velocity`.
 */
struct CartesianVelocity {
  double vx; ///< X component (AU/day).
  double vy; ///< Y component (AU/day).
  double vz; ///< Z component (AU/day).
  siderust_frame_t frame;

  /// Construct from the raw C struct.
  static CartesianVelocity from_c(const siderust_cartesian_vel_t &v) {
    return {v.vx, v.vy, v.vz, v.frame};
  }
};

/**
 * @brief Runtime-loaded JPL DE4xx ephemeris.
 *
 * This class wraps an opaque Rust `RuntimeEphemeris` handle.  It loads a
 * BSP file once (from a file path or a memory buffer) and then provides the
 * same five fundamental position/velocity queries as the compile-time
 * `ephemeris::*` free functions.
 *
 * The class is **move-only** — use `std::move` to transfer ownership.
 */
class RuntimeEphemeris {
public:
  // -- Constructors ----------------------------------------------------------

  /**
   * @brief Load a runtime ephemeris from a BSP file on disk.
   * @param path  Filesystem path to a JPL DE4xx BSP file.
   * @throws DataLoadError  if the file cannot be read or parsed.
   */
  explicit RuntimeEphemeris(const std::string &path) : handle_(nullptr) {
    siderust_runtime_ephemeris_t *h = nullptr;
    check_status(siderust_runtime_ephemeris_load_bsp(path.c_str(), &h), "RuntimeEphemeris(path)");
    handle_ = h;
  }

  /**
   * @brief Load a runtime ephemeris from raw BSP bytes in memory.
   * @param data  Pointer to BSP data.
   * @param len   Length in bytes.
   * @throws DataLoadError  if the data cannot be parsed.
   */
  RuntimeEphemeris(const uint8_t *data, size_t len) : handle_(nullptr) {
    siderust_runtime_ephemeris_t *h = nullptr;
    check_status(siderust_runtime_ephemeris_load_bytes(data, len, &h), "RuntimeEphemeris(bytes)");
    handle_ = h;
  }

  // -- Move semantics --------------------------------------------------------

  RuntimeEphemeris(RuntimeEphemeris &&other) noexcept : handle_(other.handle_) {
    other.handle_ = nullptr;
  }

  RuntimeEphemeris &operator=(RuntimeEphemeris &&other) noexcept {
    if (this != &other) {
      reset();
      handle_ = other.handle_;
      other.handle_ = nullptr;
    }
    return *this;
  }

  // Non-copyable
  RuntimeEphemeris(const RuntimeEphemeris &) = delete;
  RuntimeEphemeris &operator=(const RuntimeEphemeris &) = delete;

  // -- Destructor ------------------------------------------------------------

  ~RuntimeEphemeris() { reset(); }

  // -- Queries ---------------------------------------------------------------

  /**
   * @brief Sun's barycentric position (EclipticMeanJ2000, AU).
   */
  cartesian::position::HelioBarycentric<qtty::AstronomicalUnit>
  sun_barycentric(const JulianDate &jd) const {
    siderust_cartesian_pos_t out;
    check_status(siderust_runtime_ephemeris_sun_barycentric(handle_, jd.value(), &out),
                 "RuntimeEphemeris::sun_barycentric");
    return cartesian::position::HelioBarycentric<qtty::AstronomicalUnit>::from_c(out);
  }

  /**
   * @brief Earth's barycentric position (EclipticMeanJ2000, AU).
   */
  cartesian::position::GeoBarycentric<qtty::AstronomicalUnit>
  earth_barycentric(const JulianDate &jd) const {
    siderust_cartesian_pos_t out;
    check_status(siderust_runtime_ephemeris_earth_barycentric(handle_, jd.value(), &out),
                 "RuntimeEphemeris::earth_barycentric");
    return cartesian::position::GeoBarycentric<qtty::AstronomicalUnit>::from_c(out);
  }

  /**
   * @brief Earth's heliocentric position (EclipticMeanJ2000, AU).
   */
  cartesian::position::EclipticMeanJ2000<qtty::AstronomicalUnit>
  earth_heliocentric(const JulianDate &jd) const {
    siderust_cartesian_pos_t out;
    check_status(siderust_runtime_ephemeris_earth_heliocentric(handle_, jd.value(), &out),
                 "RuntimeEphemeris::earth_heliocentric");
    return cartesian::position::EclipticMeanJ2000<qtty::AstronomicalUnit>::from_c(out);
  }

  /**
   * @brief Moon's geocentric position (EclipticMeanJ2000, km).
   */
  cartesian::position::MoonGeocentric<qtty::Kilometer> moon_geocentric(const JulianDate &jd) const {
    siderust_cartesian_pos_t out;
    check_status(siderust_runtime_ephemeris_moon_geocentric(handle_, jd.value(), &out),
                 "RuntimeEphemeris::moon_geocentric");
    return cartesian::position::MoonGeocentric<qtty::Kilometer>::from_c(out);
  }

  /**
   * @brief Earth's barycentric velocity (EclipticMeanJ2000, AU/day).
   *
   * Returns the first-order time derivative of the Earth's barycentric
   * position as provided by the loaded JPL DE kernel.
   */
  CartesianVelocity earth_barycentric_velocity(const JulianDate &jd) const {
    siderust_cartesian_vel_t out{};
    check_status(siderust_runtime_ephemeris_earth_barycentric_velocity(handle_, jd.value(), &out),
                 "RuntimeEphemeris::earth_barycentric_velocity");
    return CartesianVelocity::from_c(out);
  }

  // -- Validity --------------------------------------------------------------

  /**
   * @brief Check whether this handle is valid (non-null).
   */
  explicit operator bool() const noexcept { return handle_ != nullptr; }

private:
  siderust_runtime_ephemeris_t *handle_;

  void reset() noexcept {
    if (handle_) {
      siderust_runtime_ephemeris_free(handle_);
      handle_ = nullptr;
    }
  }
};

} // namespace siderust
