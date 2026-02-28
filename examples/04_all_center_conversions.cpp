// SPDX-License-Identifier: AGPL-3.0-or-later
// Copyright (C) 2026 Vallés Puig, Ramon

/// @file 04_all_center_conversions.cpp
/// @brief All currently supported center conversions.
///
/// Demonstrates:
/// - Standard center shifts: Barycentric ↔ Heliocentric ↔ Geocentric
/// - Identity shifts for each center
/// - Bodycentric conversions with round-trip (Mars helio + ISS geo)
///
/// Build & run:
///   cmake --build build-local --target 04_all_center_conversions_example
///   ./build-local/04_all_center_conversions_example

#include <siderust/siderust.hpp>

#include <cstdio>
#include <iomanip>
#include <iostream>

using namespace siderust;
using namespace siderust::frames;
using namespace siderust::centers;
using namespace qtty::literals;

using F = EclipticMeanJ2000;
using U = qtty::AstronomicalUnit;

// ─── Standard center shifts ──────────────────────────────────────────────────

/// Show a center conversion C1→C2, round-trip C1→C2→C1, and the error.
template <typename C1, typename C2>
void show_center_conversion(const JulianDate &jd,
                            const cartesian::Position<C1, F, U> &src) {
    auto out  = src.template to_center<C2>(jd);
    auto back = out.template to_center<C1>(jd);
    auto err  = (src - back).magnitude();

    std::cout << std::left << std::setw(12) << CenterTraits<C1>::name()
              << " -> " << std::setw(12) << CenterTraits<C2>::name()
              << " out=(" << std::showpos << std::fixed << std::setprecision(9)
              << out
              << std::noshowpos << ")  roundtrip=" << std::scientific
              << std::setprecision(3) << err << std::fixed
              << std::endl;
}

// ─── Bodycentric ─────────────────────────────────────────────────────────────

/// Transform `src` (in center `C`) to body-centric coordinates and back.
///
/// Round-trip: C → Bodycentric → Geocentric → C.
template <typename C>
void show_bodycentric_conversion(const char *label,
                                 const JulianDate &jd,
                                 const cartesian::Position<C, F, U> &src,
                                 const BodycentricParams &params) {
    auto bary = to_bodycentric(src, params, jd);
    auto recovered_geo = bary.to_geocentric(jd);
    auto recovered = recovered_geo.template to_center<C>(jd);
    auto err = (src - recovered).magnitude();

    std::cout << std::left << std::setw(12) << label
              << " -> " << std::setw(12) << "Bodycentric"
              << " dist=" << std::fixed << std::setprecision(6)
              << bary.distance()
              << "  roundtrip=" << std::scientific << std::setprecision(3)
              << err << std::fixed
              << std::endl;
}

// ─── main ─────────────────────────────────────────────────────────────────────

int main() {
    JulianDate jd(2460000.5);
    std::cout << "Center conversion demo at JD(TT) = " << std::fixed
              << std::setprecision(1) << jd << "\n" << std::endl;

    cartesian::Position<Barycentric, F, U> p_bary(0.40, -0.10, 1.20);
    auto p_helio = p_bary.to_center<Heliocentric>(jd);
    auto p_geo   = p_bary.to_center<Geocentric>(jd);

    // ── Standard center shifts ────────────────────────────────────────────────
    std::puts("── Standard center shifts ─────────────────────────────────────────────");

    // Barycentric source
    show_center_conversion<Barycentric, Barycentric>(jd, p_bary);
    show_center_conversion<Barycentric, Heliocentric>(jd, p_bary);
    show_center_conversion<Barycentric, Geocentric>(jd, p_bary);

    // Heliocentric source
    show_center_conversion<Heliocentric, Heliocentric>(jd, p_helio);
    show_center_conversion<Heliocentric, Barycentric>(jd, p_helio);
    show_center_conversion<Heliocentric, Geocentric>(jd, p_helio);

    // Geocentric source
    show_center_conversion<Geocentric, Geocentric>(jd, p_geo);
    show_center_conversion<Geocentric, Barycentric>(jd, p_geo);
    show_center_conversion<Geocentric, Heliocentric>(jd, p_geo);

    // ── Bodycentric: Mars-like orbit (heliocentric reference) ──────────────────
    std::puts("\n── Bodycentric – Mars-like orbit (heliocentric ref) ───────────────────");
    Orbit mars_orbit{
        1.524_au,                        // semi_major_axis
        0.0934,                          // eccentricity
        1.85_deg,                        // inclination
        49.56_deg,                       // lon_ascending_node
        286.5_deg,                       // arg_perihelion
        19.41_deg,                       // mean_anomaly
        jd.value()
    };
    auto mars_params = BodycentricParams::heliocentric(mars_orbit);

    show_bodycentric_conversion("Heliocentric", jd, p_helio, mars_params);
    show_bodycentric_conversion("Barycentric",  jd, p_bary,  mars_params);
    show_bodycentric_conversion("Geocentric",   jd, p_geo,   mars_params);

    // ── Bodycentric: ISS-like orbit (geocentric reference) ────────────────────
    std::puts("\n── Bodycentric – ISS-like orbit (geocentric ref) ──────────────────────");
    Orbit iss_orbit{
        0.0000426_au,                      // ~6 378 km in AU
        0.001,                             // eccentricity
        51.6_deg,                          // inclination
        0.0_deg,                           // lon_ascending_node
        0.0_deg,                           // arg_perihelion
        0.0_deg,                           // mean_anomaly
        jd.value()
    };
    auto iss_params = BodycentricParams::geocentric(iss_orbit);

    show_bodycentric_conversion("Heliocentric", jd, p_helio, iss_params);
    show_bodycentric_conversion("Barycentric",  jd, p_bary,  iss_params);
    show_bodycentric_conversion("Geocentric",   jd, p_geo,   iss_params);


    // ── Topocentric ───────────────────────────────────────────────────────────
    // NOTE: Topocentric position transforms are not yet available in the C FFI;
    // Topocentric phase/altitude/azimuth queries are available via the altitude
    // and lunar_phase modules.  This section will be enabled once
    // siderust_to_topocentric / siderust_from_topocentric FFI functions are added.

    return 0;
}
