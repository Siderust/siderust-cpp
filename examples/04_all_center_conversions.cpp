/**
 * @file 04_all_center_conversions.cpp
 * @brief C++ port of siderust/examples/22_all_center_conversions.rs
 *
 * Demonstrates all supported center-shift pairs with round-trip error metric:
 * - Barycentric <-> Heliocentric
 * - Barycentric <-> Geocentric
 * - Heliocentric <-> Geocentric
 *
 * NOTE: The Rust library exposes an automatic `to_center()` trait API backed
 * by a `CenterShiftProvider` pattern. In C++ this is not yet fully bound;
 * center shifts are performed here manually using VSOP87 ephemeris offsets,
 * which is the same underlying calculation.
 *
 * TODO: When `CenterShiftProvider` is bound in C++, replace the manual
 *       arithmetic below with the typed transform calls.
 *
 * Run with: cmake --build build --target all_center_conversions_example
 */

#include <cmath>
#include <iomanip>
#include <iostream>

#include <siderust/siderust.hpp>

using namespace siderust;
using namespace qtty::literals;

using AU = qtty::AstronomicalUnit;

// ---------------------------------------------------------------------------
// Compute Euclidean error between two cartesian positions (same type erased)
// ---------------------------------------------------------------------------
static double cart_error(double ax, double ay, double az,
                         double bx, double by, double bz) {
    double dx = ax - bx, dy = ay - by, dz = az - bz;
    return std::sqrt(dx * dx + dy * dy + dz * dz);
}

int main() {
    const JulianDate jd = JulianDate(2'460'000.5);
    std::cout << "Center conversion demo at JD(TT) = " << std::fixed
              << std::setprecision(1) << jd.value() << "\n\n";

    // Fetch SSB offsets via ephemeris (same data the Rust provider uses)
    auto sun_bary  = ephemeris::sun_barycentric(jd);   // Sun relative to SSB
    auto earth_bary = ephemeris::earth_barycentric(jd); // Earth relative to SSB

    // A sample physical point defined in barycentric coords
    cartesian::Position<centers::Barycentric, frames::EclipticMeanJ2000, AU>
        p_bary(AU(0.40), AU(-0.10), AU(1.20));

    // -------------------------------------------------------------------------
    // Derive heliocentric and geocentric equivalents
    //   helio  = bary  - sun_bary
    //   geo    = bary  - earth_bary
    // -------------------------------------------------------------------------
    cartesian::Position<centers::Heliocentric, frames::EclipticMeanJ2000, AU>
        p_helio(p_bary.x() - sun_bary.x(),
                p_bary.y() - sun_bary.y(),
                p_bary.z() - sun_bary.z());

    cartesian::Position<centers::Geocentric, frames::EclipticMeanJ2000, AU>
        p_geo(p_bary.x() - earth_bary.x(),
              p_bary.y() - earth_bary.y(),
              p_bary.z() - earth_bary.z());

    const auto fmt = [](double val) {
        return val; // just pass through for cout
    };

    auto print_row = [&](const char *from, const char *to,
                         double ox, double oy, double oz,
                         double err) {
        std::cout << std::left  << std::setw(14) << from
                  << " -> " << std::setw(14) << to
                  << "  out=(" << std::fixed << std::setprecision(9)
                  << std::setw(13) << ox << ", "
                  << std::setw(13) << oy << ", "
                  << std::setw(13) << oz << ")  "
                  << "roundtrip=" << std::scientific << std::setprecision(3)
                  << err << "\n";
    };

    // -------------------------------------------------------------------------
    // Barycentric source
    // -------------------------------------------------------------------------

    // Bary -> Bary (identity)
    print_row("Barycentric", "Barycentric",
              p_bary.x().value(), p_bary.y().value(), p_bary.z().value(), 0.0);

    // Bary -> Helio
    {
        // back: helio -> bary = helio + sun_bary
        cartesian::Position<centers::Barycentric, frames::EclipticMeanJ2000, AU>
            back(p_helio.x() + sun_bary.x(),
                 p_helio.y() + sun_bary.y(),
                 p_helio.z() + sun_bary.z());
        double err = cart_error(p_bary.x().value(), p_bary.y().value(), p_bary.z().value(),
                                back.x().value(),   back.y().value(),   back.z().value());
        print_row("Barycentric", "Heliocentric",
                  p_helio.x().value(), p_helio.y().value(), p_helio.z().value(), err);
    }

    // Bary -> Geo
    {
        cartesian::Position<centers::Barycentric, frames::EclipticMeanJ2000, AU>
            back(p_geo.x() + earth_bary.x(),
                 p_geo.y() + earth_bary.y(),
                 p_geo.z() + earth_bary.z());
        double err = cart_error(p_bary.x().value(), p_bary.y().value(), p_bary.z().value(),
                                back.x().value(),   back.y().value(),   back.z().value());
        print_row("Barycentric", "Geocentric",
                  p_geo.x().value(), p_geo.y().value(), p_geo.z().value(), err);
    }

    // -------------------------------------------------------------------------
    // Heliocentric source
    // -------------------------------------------------------------------------
    print_row("Heliocentric", "Heliocentric",
              p_helio.x().value(), p_helio.y().value(), p_helio.z().value(), 0.0);

    // Helio -> Bary
    {
        cartesian::Position<centers::Barycentric, frames::EclipticMeanJ2000, AU>
            out(p_helio.x() + sun_bary.x(),
                p_helio.y() + sun_bary.y(),
                p_helio.z() + sun_bary.z());
        cartesian::Position<centers::Heliocentric, frames::EclipticMeanJ2000, AU>
            back(out.x() - sun_bary.x(),
                 out.y() - sun_bary.y(),
                 out.z() - sun_bary.z());
        double err = cart_error(p_helio.x().value(), p_helio.y().value(), p_helio.z().value(),
                                back.x().value(),    back.y().value(),    back.z().value());
        print_row("Heliocentric", "Barycentric",
                  out.x().value(), out.y().value(), out.z().value(), err);
    }

    // Helio -> Geo
    {
        cartesian::Position<centers::Geocentric, frames::EclipticMeanJ2000, AU>
            out(p_helio.x() - (earth_bary.x() - sun_bary.x()),
                p_helio.y() - (earth_bary.y() - sun_bary.y()),
                p_helio.z() - (earth_bary.z() - sun_bary.z()));
        cartesian::Position<centers::Heliocentric, frames::EclipticMeanJ2000, AU>
            back(out.x() + (earth_bary.x() - sun_bary.x()),
                 out.y() + (earth_bary.y() - sun_bary.y()),
                 out.z() + (earth_bary.z() - sun_bary.z()));
        double err = cart_error(p_helio.x().value(), p_helio.y().value(), p_helio.z().value(),
                                back.x().value(),    back.y().value(),    back.z().value());
        print_row("Heliocentric", "Geocentric",
                  out.x().value(), out.y().value(), out.z().value(), err);
    }

    // -------------------------------------------------------------------------
    // Geocentric source
    // -------------------------------------------------------------------------
    print_row("Geocentric", "Geocentric",
              p_geo.x().value(), p_geo.y().value(), p_geo.z().value(), 0.0);

    // Geo -> Bary
    {
        cartesian::Position<centers::Barycentric, frames::EclipticMeanJ2000, AU>
            out(p_geo.x() + earth_bary.x(),
                p_geo.y() + earth_bary.y(),
                p_geo.z() + earth_bary.z());
        cartesian::Position<centers::Geocentric, frames::EclipticMeanJ2000, AU>
            back(out.x() - earth_bary.x(),
                 out.y() - earth_bary.y(),
                 out.z() - earth_bary.z());
        double err = cart_error(p_geo.x().value(), p_geo.y().value(), p_geo.z().value(),
                                back.x().value(),  back.y().value(),  back.z().value());
        print_row("Geocentric", "Barycentric",
                  out.x().value(), out.y().value(), out.z().value(), err);
    }

    // Geo -> Helio
    {
        cartesian::Position<centers::Heliocentric, frames::EclipticMeanJ2000, AU>
            out(p_geo.x() + (earth_bary.x() - sun_bary.x()),
                p_geo.y() + (earth_bary.y() - sun_bary.y()),
                p_geo.z() + (earth_bary.z() - sun_bary.z()));
        cartesian::Position<centers::Geocentric, frames::EclipticMeanJ2000, AU>
            back(out.x() - (earth_bary.x() - sun_bary.x()),
                 out.y() - (earth_bary.y() - sun_bary.y()),
                 out.z() - (earth_bary.z() - sun_bary.z()));
        double err = cart_error(p_geo.x().value(), p_geo.y().value(), p_geo.z().value(),
                                back.x().value(),  back.y().value(),  back.z().value());
        print_row("Geocentric", "Heliocentric",
                  out.x().value(), out.y().value(), out.z().value(), err);
    }

    std::cout << "\n=== Example Complete ===\n";
    return 0;
}
