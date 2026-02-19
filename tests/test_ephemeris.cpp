#include <gtest/gtest.h>
#include <siderust/siderust.hpp>
#include <cmath>

using namespace siderust;

// ============================================================================
// Legacy API (backward compat)
// ============================================================================

TEST(Ephemeris, SunBarycentric) {
    auto jd  = JulianDate::J2000();
    auto pos = ephemeris::sun_barycentric(jd);
    EXPECT_EQ(pos.frame, Frame::EclipticMeanJ2000);
    EXPECT_EQ(pos.center, Center::Barycentric);
    // Sun is very close to the barycenter: distance << 0.01 AU
    double r = std::sqrt(pos.x * pos.x + pos.y * pos.y + pos.z * pos.z);
    EXPECT_LT(r, 0.02);
}

TEST(Ephemeris, EarthBarycentric) {
    auto jd  = JulianDate::J2000();
    auto pos = ephemeris::earth_barycentric(jd);
    EXPECT_EQ(pos.frame, Frame::EclipticMeanJ2000);
    // Earth should be ~1 AU from barycenter
    double r = std::sqrt(pos.x * pos.x + pos.y * pos.y + pos.z * pos.z);
    EXPECT_NEAR(r, 1.0, 0.02);
}

TEST(Ephemeris, EarthHeliocentric) {
    auto jd  = JulianDate::J2000();
    auto pos = ephemeris::earth_heliocentric(jd);
    EXPECT_EQ(pos.frame, Frame::EclipticMeanJ2000);
    EXPECT_EQ(pos.center, Center::Heliocentric);
    double r = std::sqrt(pos.x * pos.x + pos.y * pos.y + pos.z * pos.z);
    EXPECT_NEAR(r, 1.0, 0.02);
}

TEST(Ephemeris, MoonGeocentric) {
    auto jd  = JulianDate::J2000();
    auto pos = ephemeris::moon_geocentric(jd);
    EXPECT_EQ(pos.frame, Frame::EclipticMeanJ2000);
    EXPECT_EQ(pos.center, Center::Geocentric);
    // Moon distance ~356k-407k km (varies with orbit phase)
    double r = std::sqrt(pos.x * pos.x + pos.y * pos.y + pos.z * pos.z);
    EXPECT_NEAR(r, 384400.0, 25000.0);
}

// ============================================================================
// Typed API
// ============================================================================

TEST(EphemerisTyped, EarthHeliocentricTyped) {
    auto jd  = JulianDate::J2000();
    auto pos = ephemeris::earth_heliocentric_typed(jd);

    // Compile-time type checks
    static_assert(std::is_same_v<decltype(pos), EclipticCartPosAU>);
    static_assert(std::is_same_v<decltype(pos.comp_x), qtty::AstronomicalUnit>);

    // Value check — distance should be ~1 AU
    double r = std::sqrt(pos.x() * pos.x() + pos.y() * pos.y() + pos.z() * pos.z());
    EXPECT_NEAR(r, 1.0, 0.02);

    // Unit conversion: AU -> Kilometer (on individual component)
    qtty::Kilometer x_km = pos.comp_x.to<qtty::Kilometer>();
    // x_km is one component, not the full distance; just verify conversion works
    EXPECT_NEAR(x_km.value(), pos.x() * 1.495978707e8, 1e3);

    // Total distance in km should be ~1 AU ≈ 149.6M km
    double r_km = r * 1.495978707e8;
    EXPECT_NEAR(r_km, 1.496e8, 3e6);
}

TEST(EphemerisTyped, MoonGeocentricTyped) {
    auto jd  = JulianDate::J2000();
    auto pos = ephemeris::moon_geocentric_typed(jd);

    static_assert(std::is_same_v<decltype(pos), MoonGeoCartPosKM>);
    static_assert(std::is_same_v<decltype(pos.comp_x), qtty::Kilometer>);

    double r = std::sqrt(pos.x() * pos.x() + pos.y() * pos.y() + pos.z() * pos.z());
    EXPECT_NEAR(r, 384400.0, 25000.0);
}
