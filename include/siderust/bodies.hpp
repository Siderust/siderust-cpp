#pragma once

/**
 * @file bodies.hpp
 * @brief RAII Star handle, Planet value type, and catalog helpers.
 */

#include "coordinates.hpp"
#include "ffi_core.hpp"
#include <optional>
#include <string>
#include <utility>

namespace siderust {

// ============================================================================
// ProperMotion
// ============================================================================

/**
 * @brief Proper motion for a star (equatorial).
 */
struct ProperMotion {
  double pm_ra_deg_yr;     ///< RA proper motion (deg/yr).
  double pm_dec_deg_yr;    ///< Dec proper motion (deg/yr).
  RaConvention convention; ///< RA rate convention.

  ProperMotion(double ra, double dec,
               RaConvention conv = RaConvention::MuAlphaStar)
      : pm_ra_deg_yr(ra), pm_dec_deg_yr(dec), convention(conv) {}

  siderust_proper_motion_t to_c() const {
    return {pm_ra_deg_yr, pm_dec_deg_yr,
            static_cast<siderust_ra_convention_t>(convention)};
  }
};

// ============================================================================
// Orbit
// ============================================================================

/**
 * @brief Keplerian orbital elements.
 */
struct Orbit {
  qtty::AstronomicalUnit semi_major_axis; ///< Semi-major axis.
  double eccentricity;                    ///< Orbital eccentricity [0, 1).
  qtty::Degree inclination;              ///< Orbital inclination.
  qtty::Degree lon_ascending_node;       ///< Longitude of ascending node.
  qtty::Degree arg_perihelion;           ///< Argument of perihelion.
  qtty::Degree mean_anomaly;             ///< Mean anomaly at epoch.
  double epoch_jd;                        ///< Reference epoch (Julian Date).

  static Orbit from_c(const siderust_orbit_t &c) {
    return {qtty::AstronomicalUnit(c.semi_major_axis_au),
            c.eccentricity,
            qtty::Degree(c.inclination_deg),
            qtty::Degree(c.lon_ascending_node_deg),
            qtty::Degree(c.arg_perihelion_deg),
            qtty::Degree(c.mean_anomaly_deg),
            c.epoch_jd};
  }

  /// Convert to C FFI struct.
  siderust_orbit_t to_c() const {
    return {semi_major_axis.value(), eccentricity, inclination.value(),
            lon_ascending_node.value(), arg_perihelion.value(),
            mean_anomaly.value(), epoch_jd};
  }
};

// ============================================================================
// Planet
// ============================================================================

/**
 * @brief Planet data (value type, copyable).
 */
struct Planet {
  qtty::Kilogram mass;    ///< Planet mass.
  qtty::Kilometer radius; ///< Mean equatorial radius.
  Orbit orbit;

  static Planet from_c(const siderust_planet_t &c) {
    return {qtty::Kilogram(c.mass_kg), qtty::Kilometer(c.radius_km),
            Orbit::from_c(c.orbit)};
  }
};

namespace detail {

inline Planet make_planet_mercury() {
  siderust_planet_t out;
  check_status(siderust_planet_mercury(&out), "MERCURY");
  return Planet::from_c(out);
}

inline Planet make_planet_venus() {
  siderust_planet_t out;
  check_status(siderust_planet_venus(&out), "VENUS");
  return Planet::from_c(out);
}

inline Planet make_planet_earth() {
  siderust_planet_t out;
  check_status(siderust_planet_earth(&out), "EARTH");
  return Planet::from_c(out);
}

inline Planet make_planet_mars() {
  siderust_planet_t out;
  check_status(siderust_planet_mars(&out), "MARS");
  return Planet::from_c(out);
}

inline Planet make_planet_jupiter() {
  siderust_planet_t out;
  check_status(siderust_planet_jupiter(&out), "JUPITER");
  return Planet::from_c(out);
}

inline Planet make_planet_saturn() {
  siderust_planet_t out;
  check_status(siderust_planet_saturn(&out), "SATURN");
  return Planet::from_c(out);
}

inline Planet make_planet_uranus() {
  siderust_planet_t out;
  check_status(siderust_planet_uranus(&out), "URANUS");
  return Planet::from_c(out);
}

inline Planet make_planet_neptune() {
  siderust_planet_t out;
  check_status(siderust_planet_neptune(&out), "NEPTUNE");
  return Planet::from_c(out);
}

} // namespace detail

inline const Planet &MERCURY() {
  static const Planet s = detail::make_planet_mercury();
  return s;
}
inline const Planet &VENUS() {
  static const Planet s = detail::make_planet_venus();
  return s;
}
inline const Planet &EARTH() {
  static const Planet s = detail::make_planet_earth();
  return s;
}
inline const Planet &MARS() {
  static const Planet s = detail::make_planet_mars();
  return s;
}
inline const Planet &JUPITER() {
  static const Planet s = detail::make_planet_jupiter();
  return s;
}
inline const Planet &SATURN() {
  static const Planet s = detail::make_planet_saturn();
  return s;
}
inline const Planet &URANUS() {
  static const Planet s = detail::make_planet_uranus();
  return s;
}
inline const Planet &NEPTUNE() {
  static const Planet s = detail::make_planet_neptune();
  return s;
}

// Backward-compatible function aliases.
inline Planet mercury() { return MERCURY(); }
inline Planet venus() { return VENUS(); }
inline Planet earth() { return EARTH(); }
inline Planet mars() { return MARS(); }
inline Planet jupiter() { return JUPITER(); }
inline Planet saturn() { return SATURN(); }
inline Planet uranus() { return URANUS(); }
inline Planet neptune() { return NEPTUNE(); }

// ============================================================================
// Star (RAII)
// ============================================================================

/**
 * @brief RAII handle to a Star (opaque Rust object).
 *
 * Non-copyable; move-only. Released on destruction.
 */
class Star {
  SiderustStar *m_handle = nullptr;

  explicit Star(SiderustStar *h) : m_handle(h) {}

public:
  Star() = default;
  ~Star() {
    if (m_handle)
      siderust_star_free(m_handle);
  }

  // Move-only
  Star(Star &&o) noexcept : m_handle(o.m_handle) { o.m_handle = nullptr; }
  Star &operator=(Star &&o) noexcept {
    if (this != &o) {
      if (m_handle)
        siderust_star_free(m_handle);
      m_handle = o.m_handle;
      o.m_handle = nullptr;
    }
    return *this;
  }
  Star(const Star &) = delete;
  Star &operator=(const Star &) = delete;

  /// Whether the handle is valid.
  explicit operator bool() const { return m_handle != nullptr; }

  /// Access the raw C handle (for passing to altitude functions).
  const SiderustStar *c_handle() const { return m_handle; }

  // -- Factory methods --

  /**
   * @brief Look up a star from the built-in catalog.
   *
   * Supported: "VEGA", "SIRIUS", "POLARIS", "CANOPUS", "ARCTURUS",
   * "RIGEL", "BETELGEUSE", "PROCYON", "ALDEBARAN", "ALTAIR".
   */
  static Star catalog(const std::string &name) {
    SiderustStar *h = nullptr;
    check_status(siderust_star_catalog(name.c_str(), &h), "Star::catalog");
    return Star(h);
  }

  /**
   * @brief Create a custom star.
   *
   * @param name           Star name.
   * @param distance_ly    Distance in light-years.
   * @param mass_solar     Mass in solar masses.
   * @param radius_solar   Radius in solar radii.
   * @param luminosity_solar Luminosity in solar luminosities.
   * @param ra_deg         Right ascension (J2000) in degrees.
   * @param dec_deg        Declination (J2000) in degrees.
   * @param epoch_jd       Epoch of coordinates (Julian Date).
   * @param pm             Optional proper motion.
   */
  static Star create(const std::string &name, double distance_ly,
                     double mass_solar, double radius_solar,
                     double luminosity_solar, double ra_deg, double dec_deg,
                     double epoch_jd,
                     const std::optional<ProperMotion> &pm = std::nullopt) {
    SiderustStar *h = nullptr;
    const siderust_proper_motion_t *pm_ptr = nullptr;
    siderust_proper_motion_t pm_c{};
    if (pm.has_value()) {
      pm_c = pm->to_c();
      pm_ptr = &pm_c;
    }
    check_status(siderust_star_create(name.c_str(), distance_ly, mass_solar,
                                      radius_solar, luminosity_solar, ra_deg,
                                      dec_deg, epoch_jd, pm_ptr, &h),
                 "Star::create");
    return Star(h);
  }

  // -- Accessors --

  std::string name() const {
    char buf[256];
    uintptr_t written = 0;
    check_status(siderust_star_name(m_handle, buf, sizeof(buf), &written),
                 "Star::name");
    return std::string(buf, written);
  }

  double distance_ly() const { return siderust_star_distance_ly(m_handle); }
  double mass_solar() const { return siderust_star_mass_solar(m_handle); }
  double radius_solar() const { return siderust_star_radius_solar(m_handle); }
  double luminosity_solar() const {
    return siderust_star_luminosity_solar(m_handle);
  }
};

inline const Star &VEGA() {
  static const Star s = Star::catalog("VEGA");
  return s;
}
inline const Star &SIRIUS() {
  static const Star s = Star::catalog("SIRIUS");
  return s;
}
inline const Star &POLARIS() {
  static const Star s = Star::catalog("POLARIS");
  return s;
}
inline const Star &CANOPUS() {
  static const Star s = Star::catalog("CANOPUS");
  return s;
}
inline const Star &ARCTURUS() {
  static const Star s = Star::catalog("ARCTURUS");
  return s;
}
inline const Star &RIGEL() {
  static const Star s = Star::catalog("RIGEL");
  return s;
}
inline const Star &BETELGEUSE() {
  static const Star s = Star::catalog("BETELGEUSE");
  return s;
}
inline const Star &PROCYON() {
  static const Star s = Star::catalog("PROCYON");
  return s;
}
inline const Star &ALDEBARAN() {
  static const Star s = Star::catalog("ALDEBARAN");
  return s;
}
inline const Star &ALTAIR() {
  static const Star s = Star::catalog("ALTAIR");
  return s;
}

} // namespace siderust
