#include <gtest/gtest.h>
#include <siderust/siderust.hpp>
#include <cmath>

using namespace siderust;

// ============================================================================
// Legacy SphericalDirection (backward compatibility)
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
// New Typed Direction<F> API
// ============================================================================

TEST(TypedCoordinates, IcrsDirToEcliptic) {
    using namespace siderust::frames;

    IcrsDir vega(279.23473, 38.78369);
    auto jd = JulianDate::J2000();

    // Compile-time typed transform: ICRS -> EclipticMeanJ2000
    auto ecl = vega.to_frame<EclipticMeanJ2000>(jd);

    // Result is statically typed as Direction<EclipticMeanJ2000>
    static_assert(std::is_same_v<decltype(ecl), spherical::Direction<EclipticMeanJ2000>>,
                  "to_frame<EclipticMeanJ2000> must return Direction<EclipticMeanJ2000>");

    EXPECT_NEAR(ecl.lat_deg(), 61.7, 0.5);
}

TEST(TypedCoordinates, IcrsDirRoundtrip) {
    using namespace siderust::frames;

    IcrsDir icrs(100.0, 30.0);
    auto jd = JulianDate::J2000();

    auto ecl  = icrs.to_frame<EclipticMeanJ2000>(jd);
    auto back = ecl.to_frame<ICRS>(jd);

    static_assert(std::is_same_v<decltype(back), IcrsDir>);
    EXPECT_NEAR(back.lon_deg(), 100.0, 1e-4);
    EXPECT_NEAR(back.lat_deg(), 30.0, 1e-4);
}

TEST(TypedCoordinates, ToShorthand) {
    using namespace siderust::frames;

    IcrsDir icrs(100.0, 30.0);
    auto jd = JulianDate::J2000();

    // .to<Target>(jd) is a shorthand for .to_frame<Target>(jd)
    auto ecl = icrs.to<EclipticMeanJ2000>(jd);
    static_assert(std::is_same_v<decltype(ecl), spherical::Direction<EclipticMeanJ2000>>);
    EXPECT_NEAR(ecl.lat_deg(), 30.0, 30.0); // sanity check — something was computed
}

TEST(TypedCoordinates, IcrsDirToHorizontal) {
    using namespace siderust::frames;

    IcrsDir vega(279.23473, 38.78369);
    auto jd  = JulianDate::from_utc({2026, 7, 15, 22, 0, 0});
    auto obs = roque_de_los_muchachos();

    auto hor = vega.to_horizontal(jd, obs);

    static_assert(std::is_same_v<decltype(hor), spherical::Direction<Horizontal>>);
    EXPECT_GT(hor.altitude_deg(), -90.0);
    EXPECT_LT(hor.altitude_deg(), 90.0);
}

TEST(TypedCoordinates, EquatorialToIcrs) {
    using namespace siderust::frames;

    EquatorialJ2000Dir eq(100.0, 30.0);
    auto jd = JulianDate::J2000();

    auto icrs = eq.to_frame<ICRS>(jd);
    static_assert(std::is_same_v<decltype(icrs), IcrsDir>);

    // Should be close to input (EquatorialMeanJ2000 ≈ ICRS at J2000)
    EXPECT_NEAR(icrs.lon_deg(), 100.0, 0.1);
    EXPECT_NEAR(icrs.lat_deg(), 30.0, 0.1);
}

TEST(TypedCoordinates, MultiHopTransform) {
    using namespace siderust::frames;

    // EquatorialMeanOfDate -> EquatorialTrueOfDate (through hub)
    spherical::Direction<EquatorialMeanOfDate> mean_od(100.0, 30.0);
    auto jd = JulianDate::J2000();

    auto true_od = mean_od.to_frame<EquatorialTrueOfDate>(jd);
    static_assert(std::is_same_v<decltype(true_od), spherical::Direction<EquatorialTrueOfDate>>);

    // At J2000, nutation is small — should be close
    EXPECT_NEAR(true_od.lon_deg(), 100.0, 0.1);
    EXPECT_NEAR(true_od.lat_deg(), 30.0, 0.1);
}

TEST(TypedCoordinates, SameFrameIdentity) {
    using namespace siderust::frames;

    IcrsDir icrs(123.456, -45.678);
    auto jd = JulianDate::J2000();

    auto same = icrs.to_frame<ICRS>(jd);
    EXPECT_DOUBLE_EQ(same.lon_deg(), 123.456);
    EXPECT_DOUBLE_EQ(same.lat_deg(), -45.678);
}

TEST(TypedCoordinates, QttyDegreeAccessors) {
    IcrsDir d(123.456, -45.678);

    // Access as qtty::Degree
    qtty::Degree lon = d.lon;
    qtty::Degree lat = d.lat;
    EXPECT_DOUBLE_EQ(lon.value(), 123.456);
    EXPECT_DOUBLE_EQ(lat.value(), -45.678);

    // Convert to radians through qtty
    qtty::Radian lon_rad = lon.to<qtty::Radian>();
    EXPECT_NEAR(lon_rad.value(), 123.456 * M_PI / 180.0, 1e-10);
}

// ============================================================================
// Geodetic — new vs legacy interop
// ============================================================================

TEST(TypedCoordinates, GeodeticQttyFields) {
    auto obs = roque_de_los_muchachos();

    // Exercise the qtty::Degree / qtty::Meter fields
    qtty::Degree lon = obs.lon;
    qtty::Degree lat = obs.lat;
    qtty::Meter  h   = obs.height;

    EXPECT_NE(lon.value(), 0.0);
    EXPECT_NE(lat.value(), 0.0);
    EXPECT_GT(h.value(), 0.0);

    // Legacy accessors
    EXPECT_DOUBLE_EQ(obs.lon_deg(), lon.value());
    EXPECT_DOUBLE_EQ(obs.lat_deg(), lat.value());
    EXPECT_DOUBLE_EQ(obs.height_m(), h.value());
}

TEST(TypedCoordinates, GeodeticToCartesianEcef) {
    auto geo = geodetic(0.0, 0.0, 0.0);
    auto cart = geodetic_to_cartesian_ecef(geo);

    // Typed return: cartesian::Position<Geocentric, ECEF, Meter>
    static_assert(std::is_same_v<decltype(cart), EcefCartPos>);

    EXPECT_NEAR(cart.x(), 6378137.0, 1.0);
    EXPECT_NEAR(cart.y(), 0.0, 1.0);
    EXPECT_NEAR(cart.z(), 0.0, 1.0);
}

// ============================================================================
// CartesianPosition — legacy (backward compat)
// ============================================================================

TEST(Coordinates, GeodeticToCartesianEcef) {
    auto geo = geodetic(0.0, 0.0, 0.0);
    auto cart = geodetic_to_cartesian_ecef_legacy(geo);
    EXPECT_EQ(cart.frame, Frame::ECEF);
    // At lon=0, lat=0, h=0: x ≈ Earth radius (~6378 km), y ≈ 0, z ≈ 0
    EXPECT_NEAR(cart.x, 6378137.0, 1.0);  // WGS84 equatorial radius in metres
    EXPECT_NEAR(cart.y, 0.0, 1.0);
    EXPECT_NEAR(cart.z, 0.0, 1.0);
}
