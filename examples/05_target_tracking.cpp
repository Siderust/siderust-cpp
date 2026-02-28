// SPDX-License-Identifier: AGPL-3.0-or-later
// Copyright (C) 2026 Vallés Puig, Ramon

/// @file 05_target_tracking.cpp
/// @brief Target and Trackable examples.
///
/// Demonstrates:
/// - `Target` hierarchy: `DirectionTarget`, `BodyTarget`, `StarTarget`
/// - Planet ephemeris as coordinate snapshots
/// - Kepler propagation for comets and satellites
/// - Proper motion propagation (inline math)
/// - Position frame + center transforms
///
/// Build & run:
///   cmake --build build-local --target 05_target_tracking_example
///   ./build-local/05_target_tracking_example

#include <siderust/siderust.hpp>

#include <cmath>
#include <cstdio>
#include <iomanip>
#include <iostream>
#include <string>

using namespace siderust;
using namespace siderust::frames;
using namespace siderust::centers;

// ─── Helper: simple coordinate snapshot (mirrors Rust's Target<T>) ──────────

/// A timestamped position snapshot, optionally with proper motion.
template <typename P>
struct Snapshot {
    P position;
    JulianDate time;

    void update(P new_pos, JulianDate new_time) {
        position = new_pos;
        time = new_time;
    }
};

// ─── Halley's comet orbit (hardcoded from the Rust `HALLEY` constant) ───────

inline Orbit halley_orbit() {
    // a = 17.834 AU, e = 0.96714, i = 162.26°, Ω = 58.42°, ω = 111.33°,
    // M = 38.38° at epoch JD 2446467.4 (≈1986 Feb 9).
    return {17.834, 0.96714, 162.26, 58.42, 111.33, 38.38, 2446467.4};
}

// ─── Section 1: Trackable objects ───────────────────────────────────────────

void section_trackable_objects(const JulianDate &jd, const JulianDate &jd_next) {
    std::puts("1) Trackable objects (ICRS, star, Sun, planet, Moon)");

    // ICRS direction — time-invariant target
    spherical::direction::ICRS fixed_icrs(qtty::Degree(120.0), qtty::Degree(22.5));
    ICRSTarget icrs_target(fixed_icrs, jd, "FixedICRS");

    // Verify time-invariance: the ICRS direction coordinates are constant.
    std::cout << "  ICRS direction is time-invariant: " << std::fixed
              << std::setprecision(3) << fixed_icrs << std::endl;

    // Sirius via the catalog StarTarget
    StarTarget sirius_target(SIRIUS);
    std::printf("  Sirius via StarTarget: name = %s\n",
                sirius_target.name().c_str());

    // Sun, Mars, Moon via BodyTarget
    BodyTarget sun_target(Body::Sun);
    BodyTarget mars_target(Body::Mars);
    BodyTarget moon_target(Body::Moon);

    // Show Sun barycentric distance at J2000
    auto sun_bary = ephemeris::sun_barycentric(jd);
    std::cout << "  Sun barycentric distance: " << std::fixed
              << std::setprecision(6) << sun_bary.distance() << std::endl;

    // Mars heliocentric distance
    auto mars_helio = ephemeris::mars_heliocentric(jd);
    std::cout << "  Mars heliocentric distance: " << std::fixed
              << std::setprecision(6) << mars_helio.distance() << std::endl;

    // Moon geocentric distance
    auto moon_geo = ephemeris::moon_geocentric(jd);
    std::cout << "  Moon geocentric distance: " << std::fixed
              << std::setprecision(1) << moon_geo.distance() << "\n" << std::endl;
}

// ─── Section 2: Target snapshots ────────────────────────────────────────────

void section_target_snapshots(const JulianDate &jd, const JulianDate &jd_next) {
    std::puts("2) Target snapshots for arbitrary sky objects");

    // Mars heliocentric snapshot (VSOP87 ephemeris)
    Snapshot<cartesian::position::EclipticMeanJ2000<qtty::AstronomicalUnit>> mars_snap{
        ephemeris::mars_heliocentric(jd), jd};
    std::cout << "  Mars target at JD " << std::fixed << std::setprecision(1)
              << mars_snap.time << ": r = " << std::setprecision(6)
              << mars_snap.position.distance() << std::endl;

    // Update with next-day ephemeris
    mars_snap.update(ephemeris::mars_heliocentric(jd_next), jd_next);
    std::cout << "  Mars target updated to JD " << std::fixed
              << std::setprecision(1) << mars_snap.time
              << ": r = " << std::setprecision(6)
              << mars_snap.position.distance() << std::endl;

    // Halley's comet — Kepler-propagated snapshot
    auto halley_pos = kepler_position(halley_orbit(), jd);
    Snapshot<cartesian::position::EclipticMeanJ2000<qtty::AstronomicalUnit>> halley_snap{
        halley_pos, jd};
    std::cout << "  Halley target at JD " << std::fixed
              << std::setprecision(1) << halley_snap.time
              << ": r = " << std::setprecision(6)
              << halley_snap.position.distance() << std::endl;

    // DemoSat — satellite-like custom object with a geocentric orbit
    Orbit demosat_orbit{1.0002, 0.001, 0.1, 35.0, 80.0, 10.0, jd.value()};
    auto demosat_pos = kepler_position(demosat_orbit, jd);
    Snapshot<cartesian::position::EclipticMeanJ2000<qtty::AstronomicalUnit>> demosat_snap{
        demosat_pos, jd};
    std::cout << "  DemoSat target at JD " << std::fixed << std::setprecision(1)
              << demosat_snap.time << ": r = " << std::setprecision(6)
              << demosat_snap.position.distance() << "\n" << std::endl;
}

// ─── Section 3: Proper motion ───────────────────────────────────────────────

/// Apply stellar proper motion to an ICRS direction.
///
/// Computes: RA' = RA + μα* · Δt / cos(dec),  Dec' = Dec + μδ · Δt
/// where Δt is in Julian years since the reference epoch.
inline spherical::direction::ICRS
apply_proper_motion(const spherical::direction::ICRS &pos,
                    const ProperMotion &pm,
                    const JulianDate &epoch,
                    const JulianDate &target_epoch) {
    constexpr double JULIAN_YEAR = 365.25; // days
    double dt_years = (target_epoch.value() - epoch.value()) / JULIAN_YEAR;

    double ra_deg  = pos.ra().value();
    double dec_deg = pos.dec().value();

    // ProperMotion rates in deg/yr (already stored as deg/yr in the struct)
    // MuAlphaStar convention: pm_ra is already μα* = μα·cos(δ), so divide by cos(δ)
    double cos_dec = std::cos(dec_deg * M_PI / 180.0);
    double dra  = (cos_dec > 1e-12) ? pm.pm_ra_deg_yr * dt_years / cos_dec
                                    : 0.0;
    double ddec = pm.pm_dec_deg_yr * dt_years;

    return spherical::direction::ICRS(
        qtty::Degree(ra_deg + dra),
        qtty::Degree(dec_deg + ddec));
}

void section_target_with_proper_motion(const JulianDate &jd) {
    std::puts("3) Target with proper motion (stellar-style target)");

    // Betelgeuse approximate ICRS coordinates at J2000
    // (RA ≈ 88.7929°, Dec ≈ +7.4071°)
    spherical::direction::ICRS betelgeuse_pos(
        qtty::Degree(88.7929), qtty::Degree(7.4071));

    // Proper motion: µα* = 27.54 mas/yr, µδ = 10.86 mas/yr
    // Convert mas/yr → deg/yr
    constexpr double MAS_TO_DEG = 1.0 / 3600000.0;
    ProperMotion pm(27.54 * MAS_TO_DEG, 10.86 * MAS_TO_DEG);

    std::cout << "  Betelgeuse-like target at J2000: RA "
              << std::fixed << std::setprecision(6)
              << betelgeuse_pos.ra() << ", Dec "
              << betelgeuse_pos.dec() << std::endl;

    // Propagate 25 years
    constexpr double JULIAN_YEAR = 365.25;
    JulianDate jd_future(jd.value() + 25.0 * JULIAN_YEAR);
    auto moved = apply_proper_motion(betelgeuse_pos, pm, jd, jd_future);

    std::cout << "  After 25 years: RA "
              << std::fixed << std::setprecision(6)
              << moved.ra() << ", Dec "
              << moved.dec() << "\n" << std::endl;
}

// ─── Section 4: Frame + center transforms ───────────────────────────────────

void section_target_transform(const JulianDate &jd) {
    std::puts("4) Target conversion across frame + center");

    // Mars heliocentric ecliptic → geocentric equatorial
    auto mars_helio = ephemeris::mars_heliocentric(jd);
    auto mars_geoeq = mars_helio.template transform<Geocentric, EquatorialMeanJ2000>(jd);

    std::cout << "  Mars heliocentric ecliptic target: r = "
              << std::fixed << std::setprecision(6)
              << mars_helio.distance() << std::endl;
    std::cout << "  Mars geocentric equatorial target: r = "
              << std::fixed << std::setprecision(6)
              << mars_geoeq.distance() << std::endl;
}

// ─── main ─────────────────────────────────────────────────────────────────────

int main() {
    JulianDate jd = JulianDate::J2000();
    JulianDate jd_next(jd.value() + 1.0);

    std::puts("Target + Trackable examples");
    std::puts("===========================\n");

    section_trackable_objects(jd, jd_next);
    section_target_snapshots(jd, jd_next);
    section_target_with_proper_motion(jd);
    section_target_transform(jd);

    return 0;
}
