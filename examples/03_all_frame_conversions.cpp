/**
 * @file 03_all_frame_conversions.cpp
 * @brief C++ port of siderust/examples/23_all_frame_conversions.rs
 *
 * Demonstrates all supported direction frame-rotation pairs using the
 * `direction.to_frame<F>(jd)` API.
 *
 * Supported frame pairs (via ICRS hub):
 *   ICRS <-> EclipticMeanJ2000
 *   ICRS <-> EquatorialMeanJ2000
 *   ICRS <-> EquatorialMeanOfDate
 *   ICRS <-> EquatorialTrueOfDate
 *   EclipticMeanJ2000 <-> EquatorialMeanJ2000/OfDate/TrueOfDate
 *   EquatorialMean* <-> EquatorialTrue*
 *   Any -> Horizontal via `.to_horizontal(jd, observer)`
 *
 * Run with:
 *   cmake --build build --target 03_all_frame_conversions_example
 */

#include <cmath>
#include <iomanip>
#include <iostream>

#include <siderust/siderust.hpp>

using namespace siderust;
using namespace siderust::frames;
using namespace qtty::literals;

int main() {
    std::cout << "=== All Frame Conversions ===\n\n";

    const JulianDate jd = JulianDate::J2000();
    std::cout << "Epoch: J2000.0 (JD " << std::fixed << std::setprecision(1)
              << jd.value() << ")\n\n";

    // Vega in ICRS (RA=279.2348 deg, Dec=38.7837 deg)
    const spherical::direction::ICRS vega_icrs(279.2348_deg, 38.7837_deg);
    std::cout << "Reference: Vega\n";
    std::cout << "  ICRS  RA=" << std::fixed << std::setprecision(4)
              << vega_icrs.ra().value()
              << "  Dec=" << vega_icrs.dec().value() << " deg\n\n";

    // -------------------------------------------------------------------------
    // ICRS -> EclipticMeanJ2000
    // -------------------------------------------------------------------------
    std::cout << "--- ICRS -> EclipticMeanJ2000 ---\n";
    auto d_ecl = vega_icrs.to_frame<EclipticMeanJ2000>(jd);
    std::cout << "  lon=" << std::setprecision(4) << d_ecl.longitude().value()
              << "  lat=" << d_ecl.latitude().value() << " deg\n";
    auto rt_ecl = d_ecl.to_frame<ICRS>(jd);
    std::cout << "  round-trip RA err: " << std::scientific
              << std::abs(rt_ecl.ra().value() - vega_icrs.ra().value()) << "\n\n";

    // -------------------------------------------------------------------------
    // ICRS -> EquatorialMeanJ2000
    // -------------------------------------------------------------------------
    std::cout << "--- ICRS -> EquatorialMeanJ2000 ---\n";
    auto d_eqj = vega_icrs.to_frame<EquatorialMeanJ2000>(jd);
    std::cout << "  RA=" << std::fixed << std::setprecision(4) << d_eqj.ra().value()
              << "  Dec=" << d_eqj.dec().value() << " deg\n";
    auto rt_eqj = d_eqj.to_frame<ICRS>(jd);
    std::cout << "  round-trip RA err: " << std::scientific
              << std::abs(rt_eqj.ra().value() - vega_icrs.ra().value()) << "\n\n";

    // -------------------------------------------------------------------------
    // ICRS -> EquatorialMeanOfDate
    // -------------------------------------------------------------------------
    std::cout << "--- ICRS -> EquatorialMeanOfDate ---\n";
    auto d_eqmod = vega_icrs.to_frame<EquatorialMeanOfDate>(jd);
    std::cout << "  RA=" << std::fixed << std::setprecision(4) << d_eqmod.ra().value()
              << "  Dec=" << d_eqmod.dec().value() << " deg\n";
    auto rt_eqmod = d_eqmod.to_frame<ICRS>(jd);
    std::cout << "  round-trip RA err: " << std::scientific
              << std::abs(rt_eqmod.ra().value() - vega_icrs.ra().value()) << "\n\n";

    // -------------------------------------------------------------------------
    // ICRS -> EquatorialTrueOfDate
    // -------------------------------------------------------------------------
    std::cout << "--- ICRS -> EquatorialTrueOfDate ---\n";
    auto d_eqtod = vega_icrs.to_frame<EquatorialTrueOfDate>(jd);
    std::cout << "  RA=" << std::fixed << std::setprecision(4) << d_eqtod.ra().value()
              << "  Dec=" << d_eqtod.dec().value() << " deg\n";
    auto rt_eqtod = d_eqtod.to_frame<ICRS>(jd);
    std::cout << "  round-trip RA err: " << std::scientific
              << std::abs(rt_eqtod.ra().value() - vega_icrs.ra().value()) << "\n\n";

    // -------------------------------------------------------------------------
    // EclipticMeanJ2000 -> EquatorialMeanJ2000
    // -------------------------------------------------------------------------
    std::cout << "--- EclipticMeanJ2000 -> EquatorialMeanJ2000 ---\n";
    auto d_ecl_to_eqj = d_ecl.to_frame<EquatorialMeanJ2000>(jd);
    std::cout << "  RA=" << std::fixed << std::setprecision(4) << d_ecl_to_eqj.ra().value()
              << "  Dec=" << d_ecl_to_eqj.dec().value() << " deg\n\n";

    // -------------------------------------------------------------------------
    // EclipticMeanJ2000 -> EquatorialMeanOfDate
    // -------------------------------------------------------------------------
    std::cout << "--- EclipticMeanJ2000 -> EquatorialMeanOfDate ---\n";
    auto d_ecl_to_mod = d_ecl.to_frame<EquatorialMeanOfDate>(jd);
    std::cout << "  RA=" << std::fixed << std::setprecision(4) << d_ecl_to_mod.ra().value()
              << "  Dec=" << d_ecl_to_mod.dec().value() << " deg\n\n";

    // -------------------------------------------------------------------------
    // EclipticMeanJ2000 -> EquatorialTrueOfDate
    // -------------------------------------------------------------------------
    std::cout << "--- EclipticMeanJ2000 -> EquatorialTrueOfDate ---\n";
    auto d_ecl_to_tod = d_ecl.to_frame<EquatorialTrueOfDate>(jd);
    std::cout << "  RA=" << std::fixed << std::setprecision(4) << d_ecl_to_tod.ra().value()
              << "  Dec=" << d_ecl_to_tod.dec().value() << " deg\n\n";

    // -------------------------------------------------------------------------
    // EquatorialMeanJ2000 -> EquatorialMeanOfDate
    // -------------------------------------------------------------------------
    std::cout << "--- EquatorialMeanJ2000 -> EquatorialMeanOfDate ---\n";
    auto d_eqj_to_mod = d_eqj.to_frame<EquatorialMeanOfDate>(jd);
    std::cout << "  RA=" << std::fixed << std::setprecision(4) << d_eqj_to_mod.ra().value()
              << "  Dec=" << d_eqj_to_mod.dec().value() << " deg\n\n";

    // -------------------------------------------------------------------------
    // EquatorialMeanJ2000 -> EquatorialTrueOfDate
    // -------------------------------------------------------------------------
    std::cout << "--- EquatorialMeanJ2000 -> EquatorialTrueOfDate ---\n";
    auto d_eqj_to_tod = d_eqj.to_frame<EquatorialTrueOfDate>(jd);
    std::cout << "  RA=" << std::fixed << std::setprecision(4) << d_eqj_to_tod.ra().value()
              << "  Dec=" << d_eqj_to_tod.dec().value() << " deg\n\n";

    // -------------------------------------------------------------------------
    // EquatorialMeanOfDate -> EquatorialTrueOfDate
    // -------------------------------------------------------------------------
    std::cout << "--- EquatorialMeanOfDate -> EquatorialTrueOfDate ---\n";
    auto d_mod_to_tod = d_eqmod.to_frame<EquatorialTrueOfDate>(jd);
    std::cout << "  RA=" << std::fixed << std::setprecision(4) << d_mod_to_tod.ra().value()
              << "  Dec=" << d_mod_to_tod.dec().value() << " deg\n\n";

    // -------------------------------------------------------------------------
    // ICRS -> Horizontal (alt-az) — requires observer location
    // -------------------------------------------------------------------------
    std::cout << "--- ICRS -> Horizontal (Roque de los Muchachos) ---\n";
    const Geodetic obs(-17.8947_deg, 28.7606_deg, 2396.0_m);
    auto d_horiz = vega_icrs.to_horizontal(jd, obs);
    std::cout << "  Az=" << std::fixed << std::setprecision(4) << d_horiz.az().value()
              << "  Alt=" << d_horiz.altitude().value() << " deg\n\n";

    // -------------------------------------------------------------------------
    // EclipticMeanJ2000 -> Horizontal
    // -------------------------------------------------------------------------
    std::cout << "--- EclipticMeanJ2000 -> Horizontal ---\n";
    auto d_ecl_horiz = d_ecl.to_horizontal(jd, obs);
    std::cout << "  Az=" << std::fixed << std::setprecision(4) << d_ecl_horiz.az().value()
              << "  Alt=" << d_ecl_horiz.altitude().value() << " deg\n\n";

    std::cout << "=== Done ===\n";
    return 0;
}
