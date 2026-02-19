#pragma once

/**
 * @file bodies.hpp
 * @brief RAII Star handle, Planet value type, and catalog helpers.
 */

#include "ffi_core.hpp"
#include "coordinates.hpp"
#include <string>
#include <optional>
#include <utility>

namespace siderust {

// ============================================================================
// ProperMotion
// ============================================================================

/**
 * @brief Proper motion for a star (equatorial).
 */
struct ProperMotion {
    double       pm_ra_deg_yr;   ///< RA proper motion (deg/yr).
    double       pm_dec_deg_yr;  ///< Dec proper motion (deg/yr).
    RaConvention convention;     ///< RA rate convention.

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
    double semi_major_axis_au;
    double eccentricity;
    double inclination_deg;
    double lon_ascending_node_deg;
    double arg_perihelion_deg;
    double mean_anomaly_deg;
    double epoch_jd;

    static Orbit from_c(const siderust_orbit_t& c) {
        return {c.semi_major_axis_au, c.eccentricity, c.inclination_deg,
                c.lon_ascending_node_deg, c.arg_perihelion_deg,
                c.mean_anomaly_deg, c.epoch_jd};
    }
};

// ============================================================================
// Planet
// ============================================================================

/**
 * @brief Planet data (value type, copyable).
 */
struct Planet {
    double mass_kg;
    double radius_km;
    Orbit  orbit;

    static Planet from_c(const siderust_planet_t& c) {
        return {c.mass_kg, c.radius_km, Orbit::from_c(c.orbit)};
    }
};

/**
 * @brief Planet catalog entry for Mercury.
 */
inline Planet mercury() {
    siderust_planet_t out;
    check_status(siderust_planet_mercury(&out), "mercury");
    return Planet::from_c(out);
}

/**
 * @brief Planet catalog entry for Venus.
 */
inline Planet venus() {
    siderust_planet_t out;
    check_status(siderust_planet_venus(&out), "venus");
    return Planet::from_c(out);
}

/**
 * @brief Planet catalog entry for Earth.
 */
inline Planet earth() {
    siderust_planet_t out;
    check_status(siderust_planet_earth(&out), "earth");
    return Planet::from_c(out);
}

/**
 * @brief Planet catalog entry for Mars.
 */
inline Planet mars() {
    siderust_planet_t out;
    check_status(siderust_planet_mars(&out), "mars");
    return Planet::from_c(out);
}

/**
 * @brief Planet catalog entry for Jupiter.
 */
inline Planet jupiter() {
    siderust_planet_t out;
    check_status(siderust_planet_jupiter(&out), "jupiter");
    return Planet::from_c(out);
}

/**
 * @brief Planet catalog entry for Saturn.
 */
inline Planet saturn() {
    siderust_planet_t out;
    check_status(siderust_planet_saturn(&out), "saturn");
    return Planet::from_c(out);
}

/**
 * @brief Planet catalog entry for Uranus.
 */
inline Planet uranus() {
    siderust_planet_t out;
    check_status(siderust_planet_uranus(&out), "uranus");
    return Planet::from_c(out);
}

/**
 * @brief Planet catalog entry for Neptune.
 */
inline Planet neptune() {
    siderust_planet_t out;
    check_status(siderust_planet_neptune(&out), "neptune");
    return Planet::from_c(out);
}

// ============================================================================
// Star (RAII)
// ============================================================================

/**
 * @brief RAII handle to a Star (opaque Rust object).
 *
 * Non-copyable; move-only. Released on destruction.
 */
class Star {
    SiderustStar* m_handle = nullptr;

    explicit Star(SiderustStar* h) : m_handle(h) {}

public:
    Star() = default;
    ~Star() { if (m_handle) siderust_star_free(m_handle); }

    // Move-only
    Star(Star&& o) noexcept : m_handle(o.m_handle) { o.m_handle = nullptr; }
    Star& operator=(Star&& o) noexcept {
        if (this != &o) {
            if (m_handle) siderust_star_free(m_handle);
            m_handle = o.m_handle;
            o.m_handle = nullptr;
        }
        return *this;
    }
    Star(const Star&) = delete;
    Star& operator=(const Star&) = delete;

    /// Whether the handle is valid.
    explicit operator bool() const { return m_handle != nullptr; }

    /// Access the raw C handle (for passing to altitude functions).
    const SiderustStar* c_handle() const { return m_handle; }

    // -- Factory methods --

    /**
     * @brief Look up a star from the built-in catalog.
     *
     * Supported: "VEGA", "SIRIUS", "POLARIS", "CANOPUS", "ARCTURUS",
     * "RIGEL", "BETELGEUSE", "PROCYON", "ALDEBARAN", "ALTAIR".
     */
    static Star catalog(const std::string& name) {
        SiderustStar* h = nullptr;
        check_status(siderust_star_catalog(name.c_str(), &h),
                     "Star::catalog");
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
    static Star create(const std::string& name,
                       double distance_ly,
                       double mass_solar,
                       double radius_solar,
                       double luminosity_solar,
                       double ra_deg,
                       double dec_deg,
                       double epoch_jd,
                       const std::optional<ProperMotion>& pm = std::nullopt)
    {
        SiderustStar* h = nullptr;
        const siderust_proper_motion_t* pm_ptr = nullptr;
        siderust_proper_motion_t pm_c{};
        if (pm.has_value()) {
            pm_c = pm->to_c();
            pm_ptr = &pm_c;
        }
        check_status(siderust_star_create(
            name.c_str(), distance_ly, mass_solar, radius_solar,
            luminosity_solar, ra_deg, dec_deg, epoch_jd, pm_ptr, &h),
            "Star::create"
        );
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

    double distance_ly()       const { return siderust_star_distance_ly(m_handle); }
    double mass_solar()        const { return siderust_star_mass_solar(m_handle); }
    double radius_solar()      const { return siderust_star_radius_solar(m_handle); }
    double luminosity_solar()  const { return siderust_star_luminosity_solar(m_handle); }
};

} // namespace siderust
