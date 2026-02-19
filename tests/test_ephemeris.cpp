#include <gtest/gtest.h>
#include <siderust/siderust.hpp>
#include <cmath>

using namespace siderust;

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
