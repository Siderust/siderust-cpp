#include <gtest/gtest.h>
#include <siderust/siderust.hpp>
#include <cmath>

using namespace siderust;

// ============================================================================
// SphericalDirection
// ============================================================================

TEST(Coordinates, SphericalDirectionTransformIcrsToEcliptic) {
    // Vega approx RA=279.23, Dec=+38.78 (ICRS)
    SphericalDirection icrs(279.23473, 38.78369, Frame::ICRS);
    auto jd = JulianDate::J2000();
    auto ecl = icrs.transform(Frame::EclipticMeanJ2000, jd.value());

    EXPECT_EQ(ecl.frame, Frame::EclipticMeanJ2000);
    // Ecliptic latitude of Vega is ~61.7°
    EXPECT_NEAR(ecl.lat_deg, 61.7, 0.5);
}

TEST(Coordinates, SphericalDirectionRoundtrip) {
    SphericalDirection icrs(100.0, 30.0, Frame::ICRS);
    auto jd  = JulianDate::J2000();
    auto ecl = icrs.transform(Frame::EclipticMeanJ2000, jd.value());
    auto back = ecl.transform(Frame::ICRS, jd.value());

    EXPECT_NEAR(back.lon_deg, 100.0, 1e-4);
    EXPECT_NEAR(back.lat_deg, 30.0, 1e-4);
}

TEST(Coordinates, SphericalDirectionToHorizontal) {
    auto obs = roque_de_los_muchachos();
    SphericalDirection icrs(279.23473, 38.78369, Frame::ICRS);
    auto jd = JulianDate::from_utc({2026, 7, 15, 22, 0, 0});

    auto hor = icrs.to_horizontal(obs, jd.value());
    EXPECT_EQ(hor.frame, Frame::Horizontal);
    // Altitude should be a reasonable value (-90..90)
    EXPECT_GT(hor.altitude_deg(), -90.0);
    EXPECT_LT(hor.altitude_deg(), 90.0);
}

// ============================================================================
// CartesianPosition
// ============================================================================

TEST(Coordinates, GeodeticToCartesianEcef) {
    auto geo = geodetic(0.0, 0.0, 0.0);
    auto cart = geodetic_to_cartesian_ecef(geo);
    EXPECT_EQ(cart.frame, Frame::ECEF);
    // At lon=0, lat=0, h=0: x ≈ Earth radius (~6378 km), y ≈ 0, z ≈ 0
    EXPECT_NEAR(cart.x, 6378137.0, 1.0);  // WGS84 equatorial radius in metres
    EXPECT_NEAR(cart.y, 0.0, 1.0);
    EXPECT_NEAR(cart.z, 0.0, 1.0);
}
