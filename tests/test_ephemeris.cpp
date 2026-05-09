#include <cmath>
#include <gtest/gtest.h>
#include <siderust/siderust.hpp>

using namespace siderust;

// ============================================================================
// Typed API
// ============================================================================

TEST(Ephemeris, EarthHeliocentric) {
  auto jd = JulianDate::J2000();
  auto pos = ephemeris::earth_heliocentric(jd);

  // Compile-time type checks
  static_assert(std::is_same_v<decltype(pos),
                               cartesian::position::EclipticMeanJ2000<qtty::AstronomicalUnit>>);
  static_assert(std::is_same_v<decltype(pos.comp_x), qtty::AstronomicalUnit>);

  // Value check — distance should be ~1 AU
  double r = std::sqrt(pos.x().value() * pos.x().value() + pos.y().value() * pos.y().value() +
                       pos.z().value() * pos.z().value());
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
  auto jd = JulianDate::J2000();
  auto pos = ephemeris::moon_geocentric(jd);

  static_assert(
      std::is_same_v<decltype(pos), cartesian::position::MoonGeocentric<qtty::Kilometer>>);
  static_assert(std::is_same_v<decltype(pos.comp_x), qtty::Kilometer>);

  double r = std::sqrt(pos.x().value() * pos.x().value() + pos.y().value() * pos.y().value() +
                       pos.z().value() * pos.z().value());
  EXPECT_NEAR(r, 384400.0, 25000.0);
}

// ============================================================================
// RuntimeEphemeris — type correctness and error handling
// ============================================================================

TEST(RuntimeEphemeris, InvalidBspThrows) {
  // Passing garbage bytes must throw DataLoadError, not crash.
  const uint8_t bad[] = {0x00, 0x01, 0x02, 0x03};
  EXPECT_THROW(RuntimeEphemeris(bad, sizeof(bad)), DataLoadError);
}

TEST(RuntimeEphemeris, InvalidPathThrows) {
  EXPECT_THROW(RuntimeEphemeris("/nonexistent/path/de440.bsp"), DataLoadError);
}

TEST(RuntimeEphemeris, CartesianVelocityFieldsExist) {
  // Structural check: CartesianVelocity must expose vx, vy, vz, frame.
  CartesianVelocity v{1.0, 2.0, 3.0, SIDERUST_FRAME_T_ECLIPTIC_MEAN_J2000};
  EXPECT_DOUBLE_EQ(v.vx, 1.0);
  EXPECT_DOUBLE_EQ(v.vy, 2.0);
  EXPECT_DOUBLE_EQ(v.vz, 3.0);
}
