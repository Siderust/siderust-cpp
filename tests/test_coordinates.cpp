#include <gtest/gtest.h>
#include <siderust/siderust.hpp>
#include <cmath>

using namespace siderust;

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

    EXPECT_NEAR(ecl.lat.value(), 61.7, 0.5);
}

TEST(TypedCoordinates, IcrsDirRoundtrip) {
    using namespace siderust::frames;

    IcrsDir icrs(100.0, 30.0);
    auto jd = JulianDate::J2000();

    auto ecl  = icrs.to_frame<EclipticMeanJ2000>(jd);
    auto back = ecl.to_frame<ICRS>(jd);

    static_assert(std::is_same_v<decltype(back), IcrsDir>);
    EXPECT_NEAR(back.lon.value(), 100.0, 1e-4);
    EXPECT_NEAR(back.lat.value(), 30.0, 1e-4);
}

TEST(TypedCoordinates, ToShorthand) {
    using namespace siderust::frames;

    IcrsDir icrs(100.0, 30.0);
    auto jd = JulianDate::J2000();

    // .to<Target>(jd) is a shorthand for .to_frame<Target>(jd)
    auto ecl = icrs.to<EclipticMeanJ2000>(jd);
    static_assert(std::is_same_v<decltype(ecl), spherical::Direction<EclipticMeanJ2000>>);
    EXPECT_NEAR(ecl.lat.value(), 30.0, 30.0); // sanity check — something was computed
}

TEST(TypedCoordinates, IcrsDirToHorizontal) {
    using namespace siderust::frames;

    IcrsDir vega(279.23473, 38.78369);
    auto jd  = JulianDate::from_utc({2026, 7, 15, 22, 0, 0});
    auto obs = roque_de_los_muchachos();

    auto hor = vega.to_horizontal(jd, obs);

    static_assert(std::is_same_v<decltype(hor), spherical::Direction<Horizontal>>);
    EXPECT_GT(hor.altitude().value(), -90.0);
    EXPECT_LT(hor.altitude().value(), 90.0);
}

TEST(TypedCoordinates, EquatorialToIcrs) {
    using namespace siderust::frames;

    EquatorialJ2000Dir eq(100.0, 30.0);
    auto jd = JulianDate::J2000();

    auto icrs = eq.to_frame<ICRS>(jd);
    static_assert(std::is_same_v<decltype(icrs), IcrsDir>);

    // Should be close to input (EquatorialMeanJ2000 ≈ ICRS at J2000)
    EXPECT_NEAR(icrs.lon.value(), 100.0, 0.1);
    EXPECT_NEAR(icrs.lat.value(), 30.0, 0.1);
}

TEST(TypedCoordinates, MultiHopTransform) {
    using namespace siderust::frames;

    // EquatorialMeanOfDate -> EquatorialTrueOfDate (through hub)
    spherical::Direction<EquatorialMeanOfDate> mean_od(100.0, 30.0);
    auto jd = JulianDate::J2000();

    auto true_od = mean_od.to_frame<EquatorialTrueOfDate>(jd);
    static_assert(std::is_same_v<decltype(true_od), spherical::Direction<EquatorialTrueOfDate>>);

    // At J2000, nutation is small — should be close
    EXPECT_NEAR(true_od.lon.value(), 100.0, 0.1);
    EXPECT_NEAR(true_od.lat.value(), 30.0, 0.1);
}

TEST(TypedCoordinates, SameFrameIdentity) {
    using namespace siderust::frames;

    IcrsDir icrs(123.456, -45.678);
    auto jd = JulianDate::J2000();

    auto same = icrs.to_frame<ICRS>(jd);
    EXPECT_DOUBLE_EQ(same.lon.value(), 123.456);
    EXPECT_DOUBLE_EQ(same.lat.value(), -45.678);
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

    // Accessors are the fields themselves
    EXPECT_EQ(obs.lon, lon);
    EXPECT_EQ(obs.lat, lat);
    EXPECT_EQ(obs.height, h);
}

// ============================================================================
// Geodetic
// ============================================================================

TEST(TypedCoordinates, GeodeticToCartesianEcef) {
    auto geo = geodetic(0.0, 0.0, 0.0);
    auto cart = geodetic_to_cartesian_ecef(geo);

    // Typed return: cartesian::Position<Geocentric, ECEF, Meter>
    static_assert(std::is_same_v<decltype(cart), EcefCartPos>);

    EXPECT_NEAR(cart.x().value(), 6378137.0, 1.0);
    EXPECT_NEAR(cart.y().value(), 0.0, 1.0);
    EXPECT_NEAR(cart.z().value(), 0.0, 1.0);
}
