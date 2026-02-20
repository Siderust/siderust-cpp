#include <cmath>
#include <gtest/gtest.h>
#include <siderust/siderust.hpp>

using namespace siderust;

// ============================================================================
// Typed API
// ============================================================================

TEST(Ephemeris, EarthHeliocentric) {
    auto jd  = JulianDate::J2000();
    auto pos = ephemeris::earth_heliocentric(jd);

    // Compile-time type checks
    static_assert(std::is_same_v<
                  decltype(pos),
                  cartesian::position::EclipticMeanJ2000<qtty::AstronomicalUnit>>);
    static_assert(std::is_same_v<decltype(pos.comp_x), qtty::AstronomicalUnit>);

    // Value check — distance should be ~1 AU
    double r = std::sqrt(pos.x().value() * pos.x().value() + pos.y().value() * pos.y().value() + pos.z().value() * pos.z().value());
    EXPECT_NEAR(r, 1.0, 0.02);

    // Unit conversion: AU -> Kilometer (on individual component)
    qtty::Kilometer x_km = pos.comp_x.to<qtty::Kilometer>();
    // x_km is one component, not the full distance; just verify conversion works
    EXPECT_NEAR(x_km.value(), pos.x().value() * 1.495978707e8, 1e3);

    // Total distance in km should be ~1 AU ≈ 149.6M km
    double r_km = r * 1.495978707e8;
    EXPECT_NEAR(r_km, 1.496e8, 3e6);
}

TEST(Ephemeris, MoonGeocentric) {
    auto jd  = JulianDate::J2000();
    auto pos = ephemeris::moon_geocentric(jd);

    static_assert(std::is_same_v<
                  decltype(pos),
                  cartesian::position::MoonGeocentric<qtty::Kilometer>>);
    static_assert(std::is_same_v<decltype(pos.comp_x), qtty::Kilometer>);

    double r = std::sqrt(pos.x().value() * pos.x().value() + pos.y().value() * pos.y().value() + pos.z().value() * pos.z().value());
    EXPECT_NEAR(r, 384400.0, 25000.0);
}
