#include <cmath>
#include <gtest/gtest.h>
#include <siderust/siderust.hpp>
#include <type_traits>

using namespace siderust;

// ============================================================================
// New Typed Direction<F> API
// ============================================================================

TEST(TypedCoordinates, AliasNamespaces) {
  static_assert(std::is_same_v<spherical::direction::ICRS,
                               spherical::Direction<frames::ICRS>>);
  static_assert(
      std::is_same_v<spherical::direction::EclipticMeanJ2000,
                     spherical::Direction<frames::EclipticMeanJ2000>>);
  static_assert(std::is_same_v<spherical::position::ICRS<qtty::Meter>,
                               spherical::Position<centers::Barycentric,
                                                   frames::ICRS, qtty::Meter>>);
  static_assert(
      std::is_same_v<
          cartesian::position::ECEF<qtty::Meter>,
          cartesian::Position<centers::Geocentric, frames::ECEF, qtty::Meter>>);
}

TEST(TypedCoordinates, IcrsDirToEcliptic) {
  using namespace siderust::frames;

  spherical::direction::ICRS vega(qtty::Degree(279.23473),
                                  qtty::Degree(38.78369));
  auto jd = JulianDate::J2000();

  // Compile-time typed transform: ICRS -> EclipticMeanJ2000
  auto ecl = vega.to_frame<EclipticMeanJ2000>(jd);

  // Result is statically typed as Direction<EclipticMeanJ2000>
  static_assert(
      std::is_same_v<decltype(ecl), spherical::Direction<EclipticMeanJ2000>>,
      "to_frame<EclipticMeanJ2000> must return Direction<EclipticMeanJ2000>");

  EXPECT_NEAR(ecl.lat().value(), 61.7, 0.5);
}

TEST(TypedCoordinates, IcrsDirRoundtrip) {
  using namespace siderust::frames;

  spherical::direction::ICRS icrs(qtty::Degree(100.0), qtty::Degree(30.0));
  auto jd = JulianDate::J2000();

  auto ecl = icrs.to_frame<EclipticMeanJ2000>(jd);
  auto back = ecl.to_frame<ICRS>(jd);

  static_assert(std::is_same_v<decltype(back), spherical::direction::ICRS>);
  EXPECT_NEAR(back.ra().value(), 100.0, 1e-4);
  EXPECT_NEAR(back.dec().value(), 30.0, 1e-4);
}

TEST(TypedCoordinates, ToShorthand) {
  using namespace siderust::frames;

  spherical::direction::ICRS icrs(qtty::Degree(100.0), qtty::Degree(30.0));
  auto jd = JulianDate::J2000();

  // .to<Target>(jd) is a shorthand for .to_frame<Target>(jd)
  auto ecl = icrs.to<EclipticMeanJ2000>(jd);
  static_assert(
      std::is_same_v<decltype(ecl), spherical::Direction<EclipticMeanJ2000>>);
  EXPECT_NEAR(ecl.lat().value(), 30.0,
              30.0); // sanity check — something was computed
}

TEST(TypedCoordinates, IcrsDirToHorizontal) {
  using namespace siderust::frames;

  spherical::direction::ICRS vega(qtty::Degree(279.23473),
                                  qtty::Degree(38.78369));
  auto jd = JulianDate::from_utc({2026, 7, 15, 22, 0, 0});
  auto obs = ROQUE_DE_LOS_MUCHACHOS();

  auto hor = vega.to_horizontal(jd, obs);

  static_assert(
      std::is_same_v<decltype(hor), spherical::Direction<Horizontal>>);
  EXPECT_GT(hor.altitude().value(), -90.0);
  EXPECT_LT(hor.altitude().value(), 90.0);
}

TEST(TypedCoordinates, EquatorialToIcrs) {
  using namespace siderust::frames;

  spherical::direction::EquatorialMeanJ2000 eq(qtty::Degree(100.0),
                                               qtty::Degree(30.0));
  auto jd = JulianDate::J2000();

  auto icrs = eq.to_frame<ICRS>(jd);
  static_assert(std::is_same_v<decltype(icrs), spherical::direction::ICRS>);

  // Should be close to input (EquatorialMeanJ2000 ≈ ICRS at J2000)
  EXPECT_NEAR(icrs.ra().value(), 100.0, 0.1);
  EXPECT_NEAR(icrs.dec().value(), 30.0, 0.1);
}

TEST(TypedCoordinates, MultiHopTransform) {
  using namespace siderust::frames;

  // EquatorialMeanOfDate -> EquatorialTrueOfDate (through hub)
  spherical::Direction<EquatorialMeanOfDate> mean_od(qtty::Degree(100.0),
                                                     qtty::Degree(30.0));
  auto jd = JulianDate::J2000();

  auto true_od = mean_od.to_frame<EquatorialTrueOfDate>(jd);
  static_assert(std::is_same_v<decltype(true_od),
                               spherical::Direction<EquatorialTrueOfDate>>);

  // At J2000, nutation is small — should be close
  EXPECT_NEAR(true_od.ra().value(), 100.0, 0.1);
  EXPECT_NEAR(true_od.dec().value(), 30.0, 0.1);
}

TEST(TypedCoordinates, SameFrameIdentity) {
  using namespace siderust::frames;

  spherical::direction::ICRS icrs(qtty::Degree(123.456), qtty::Degree(-45.678));
  auto jd = JulianDate::J2000();

  auto same = icrs.to_frame<ICRS>(jd);
  EXPECT_DOUBLE_EQ(same.ra().value(), 123.456);
  EXPECT_DOUBLE_EQ(same.dec().value(), -45.678);
}

TEST(TypedCoordinates, QttyDegreeAccessors) {
  spherical::direction::ICRS d(qtty::Degree(123.456), qtty::Degree(-45.678));

  // Frame-specific getters for ICRS.
  qtty::Degree ra = d.ra();
  qtty::Degree dec = d.dec();
  EXPECT_DOUBLE_EQ(ra.value(), 123.456);
  EXPECT_DOUBLE_EQ(dec.value(), -45.678);

  // Convert to radians through qtty
  qtty::Radian ra_rad = ra.to<qtty::Radian>();
  EXPECT_NEAR(ra_rad.value(), 123.456 * M_PI / 180.0, 1e-10);
}

// ============================================================================
// Geodetic — new vs legacy interop
// ============================================================================

TEST(TypedCoordinates, GeodeticQttyFields) {
  auto obs = ROQUE_DE_LOS_MUCHACHOS();

  // Exercise the qtty::Degree / qtty::Meter fields
  qtty::Degree lon = obs.lon;
  qtty::Degree lat = obs.lat;
  qtty::Meter h = obs.height;

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
  static_assert(
      std::is_same_v<decltype(cart), cartesian::position::ECEF<qtty::Meter>>);

  EXPECT_NEAR(cart.x().value(), 6378137.0, 1.0);
  EXPECT_NEAR(cart.y().value(), 0.0, 1.0);
  EXPECT_NEAR(cart.z().value(), 0.0, 1.0);
}

TEST(TypedCoordinates, GeodeticToCartesianMember) {
  auto geo = geodetic(0.0, 0.0, 0.0);

  auto ecef_m = geo.to_cartesian();
  auto ecef_km = geo.to_cartesian<qtty::Kilometer>();

  static_assert(
      std::is_same_v<decltype(ecef_m), cartesian::position::ECEF<qtty::Meter>>);
  static_assert(
      std::is_same_v<decltype(ecef_km),
                     cartesian::Position<centers::Geocentric, frames::ECEF,
                                         qtty::Kilometer>>);

  EXPECT_NEAR(ecef_m.x().value(), 6378137.0, 1.0);
  EXPECT_NEAR(ecef_km.x().value(), 6378.137, 1e-3);
}

// ============================================================================
// cartesian::Direction::to_frame — new method
// ============================================================================

TEST(TypedCoordinates, CartesianDirToFrameRoundtrip) {
  using namespace siderust::frames;

  // Unit vector along X in ICRS
  cartesian::Direction<ICRS> dir_icrs(1.0, 0.0, 0.0);
  auto jd = JulianDate::J2000();

  auto dir_ecl = dir_icrs.to_frame<EclipticMeanJ2000>(jd);
  static_assert(std::is_same_v<decltype(dir_ecl),
                               cartesian::Direction<EclipticMeanJ2000>>);

  // Round-trip
  auto dir_back = dir_ecl.to_frame<ICRS>(jd);
  EXPECT_NEAR(dir_back.x, 1.0, 1e-8);
  EXPECT_NEAR(dir_back.y, 0.0, 1e-8);
  EXPECT_NEAR(dir_back.z, 0.0, 1e-8);
}

TEST(TypedCoordinates, CartesianDirToFrameIdentity) {
  using namespace siderust::frames;

  cartesian::Direction<ICRS> dir(0.6, 0.8, 0.0);
  auto jd = JulianDate::J2000();

  auto same = dir.to_frame<ICRS>(jd);
  static_assert(std::is_same_v<decltype(same), cartesian::Direction<ICRS>>);
  EXPECT_DOUBLE_EQ(same.x, 0.6);
  EXPECT_DOUBLE_EQ(same.y, 0.8);
  EXPECT_DOUBLE_EQ(same.z, 0.0);
}

TEST(TypedCoordinates, CartesianDirToFramePreservesLength) {
  using namespace siderust::frames;

  // The rotation must preserve vector length
  cartesian::Direction<ICRS> dir(0.6, 0.8, 0.0);
  auto jd = JulianDate::J2000();

  auto ecl = dir.to_frame<EclipticMeanJ2000>(jd);
  double len = std::sqrt(ecl.x * ecl.x + ecl.y * ecl.y + ecl.z * ecl.z);
  EXPECT_NEAR(len, 1.0, 1e-10);
}

// ============================================================================
// cartesian::Position::to_frame — new method
// ============================================================================

TEST(TypedCoordinates, CartesianPosToFrameRoundtrip) {
  using namespace siderust::frames;
  using AU = qtty::AstronomicalUnit;

  cartesian::Position<centers::Heliocentric, EclipticMeanJ2000, AU> pos(1.0, 0.5, 0.2);
  auto jd = JulianDate::J2000();

  auto pos_icrs = pos.to_frame<ICRS>(jd);
  static_assert(std::is_same_v<decltype(pos_icrs),
                               cartesian::Position<centers::Heliocentric, ICRS, AU>>);

  // Round-trip back to EclipticMeanJ2000
  auto pos_back = pos_icrs.to_frame<EclipticMeanJ2000>(jd);
  EXPECT_NEAR(pos_back.x().value(), 1.0, 1e-8);
  EXPECT_NEAR(pos_back.y().value(), 0.5, 1e-8);
  EXPECT_NEAR(pos_back.z().value(), 0.2, 1e-8);
}

TEST(TypedCoordinates, CartesianPosToFrameSameCenterPreserved) {
  using namespace siderust::frames;
  using AU = qtty::AstronomicalUnit;

  // Frame-only transform preserves center
  cartesian::Position<centers::Barycentric, EclipticMeanJ2000, AU> pos(1.0, 0.0, 0.0);
  auto jd = JulianDate::J2000();

  auto transformed = pos.to_frame<EquatorialMeanJ2000>(jd);
  static_assert(
      std::is_same_v<decltype(transformed),
                     cartesian::Position<centers::Barycentric, EquatorialMeanJ2000, AU>>);

  // Distance must be preserved (rotation)
  double r0 = pos.distance().value();
  double r1 = transformed.distance().value();
  EXPECT_NEAR(r0, r1, 1e-10);
}

// ============================================================================
// spherical::Position::to_frame — new method
// ============================================================================

TEST(TypedCoordinates, SphericalPosToFrameRoundtrip) {
  using namespace siderust::frames;
  using AU = qtty::AstronomicalUnit;

  spherical::Position<centers::Heliocentric, EclipticMeanJ2000, AU> sph(
      qtty::Degree(30.0), qtty::Degree(10.0), AU(1.5));
  auto jd = JulianDate::J2000();

  auto sph_icrs = sph.to_frame<ICRS>(jd);
  static_assert(
      std::is_same_v<decltype(sph_icrs),
                     spherical::Position<centers::Heliocentric, ICRS, AU>>);

  // Round-trip back
  auto sph_back = sph_icrs.to_frame<EclipticMeanJ2000>(jd);
  EXPECT_NEAR(sph_back.lon().value(), 30.0, 1e-6);
  EXPECT_NEAR(sph_back.lat().value(), 10.0, 1e-6);
  EXPECT_NEAR(sph_back.distance().value(), 1.5, 1e-10);
}

TEST(TypedCoordinates, SphericalPosToFramePreservesDistance) {
  using namespace siderust::frames;
  using AU = qtty::AstronomicalUnit;

  spherical::Position<centers::Barycentric, ICRS, AU> sph(
      qtty::Degree(100.0), qtty::Degree(45.0), AU(2.3));
  auto jd = JulianDate::J2000();

  auto ecl = sph.to_frame<EclipticMeanJ2000>(jd);
  // Distance must be unchanged by a frame rotation
  EXPECT_NEAR(ecl.distance().value(), 2.3, 1e-10);
}

TEST(TypedCoordinates, SphericalPosToFrameShorthand) {
  using namespace siderust::frames;
  using AU = qtty::AstronomicalUnit;

  spherical::Position<centers::Heliocentric, ICRS, AU> sph(
      qtty::Degree(50.0), qtty::Degree(20.0), AU(1.0));
  auto jd = JulianDate::J2000();

  // .to<Target>(jd) shorthand
  auto ecl = sph.to<EclipticMeanJ2000>(jd);
  static_assert(
      std::is_same_v<decltype(ecl),
                     spherical::Position<centers::Heliocentric, EclipticMeanJ2000, AU>>);
  EXPECT_NEAR(ecl.distance().value(), 1.0, 1e-10);
}
