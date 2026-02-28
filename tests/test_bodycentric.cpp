// SPDX-License-Identifier: AGPL-3.0-or-later
// Copyright (C) 2026 Vallés Puig, Ramon

#include <cmath>
#include <gtest/gtest.h>
#include <siderust/siderust.hpp>

using namespace siderust;
using namespace siderust::frames;
using namespace siderust::centers;
using qtty::AstronomicalUnit;
using namespace qtty::literals;

namespace {

constexpr double J2000 = 2451545.0;
constexpr double KM_PER_AU = 149597870.7;

// Satellite orbit at 0.0001 AU (~14 960 km) geocentric
Orbit satellite_orbit() {
  return {0.0001_au, 0.0, 0.0_deg,
          0.0_deg, 0.0_deg, 0.0_deg, J2000};
}

// Approximate Mars heliocentric orbit
Orbit mars_orbit() {
  return {1.524_au, 0.0934, 1.85_deg,
          49.56_deg, 286.5_deg, 19.41_deg, J2000};
}

double vec_magnitude(double x, double y, double z) {
  return std::sqrt(x * x + y * y + z * z);
}

} // namespace

// ============================================================================
// Kepler position
// ============================================================================

TEST(BodycentricTransforms, KeplerPositionGeocentricOrbit) {
  const JulianDate jd(J2000);
  auto pos = kepler_position<Geocentric>(satellite_orbit(), jd);

  // Satellite at 0.0001 AU — distance should be close to 0.0001 AU
  double r = std::sqrt(pos.x().value() * pos.x().value() +
                       pos.y().value() * pos.y().value() +
                       pos.z().value() * pos.z().value());
  EXPECT_NEAR(r, 0.0001, 1e-5);
  EXPECT_TRUE(std::isfinite(pos.x().value()));
  EXPECT_TRUE(std::isfinite(pos.y().value()));
  EXPECT_TRUE(std::isfinite(pos.z().value()));
}

TEST(BodycentricTransforms, KeplerPositionHeliocentricOrbit) {
  const JulianDate jd(J2000);
  auto pos = kepler_position(mars_orbit(), jd); // default C = Heliocentric

  double r = std::sqrt(pos.x().value() * pos.x().value() +
                       pos.y().value() * pos.y().value() +
                       pos.z().value() * pos.z().value());
  // Mars at ~1.52 AU from Sun
  EXPECT_NEAR(r, 1.524, 0.15);
  EXPECT_TRUE(std::isfinite(pos.x().value()));
}

// ============================================================================
// Geocentric source → Bodycentric (geocentric orbit)
// ============================================================================

TEST(BodycentricTransforms, GeocentricToBodycentricGeoOrbit) {
  const JulianDate jd(J2000);
  BodycentricParams params = BodycentricParams::geocentric(satellite_orbit());

  // Target at 0.001 AU from Earth
  cartesian::Position<Geocentric, EclipticMeanJ2000, AstronomicalUnit>
      target(0.001, 0.0, 0.0);

  auto result = to_bodycentric(target, params, jd);

  // Satellite is at ~0.0001 AU; relative position should be positive and < 0.001 AU
  EXPECT_GT(result.x().value(), 0.0);
  EXPECT_LT(result.x().value(), 0.001);
  EXPECT_TRUE(std::isfinite(result.x().value()));
  EXPECT_TRUE(std::isfinite(result.y().value()));
  EXPECT_TRUE(std::isfinite(result.z().value()));

  // center_params round-trips correctly
  EXPECT_NEAR(result.center_params().orbit.semi_major_axis.value(), 0.0001, 1e-10);
}

// ============================================================================
// Heliocentric source → Bodycentric (heliocentric orbit)
// ============================================================================

TEST(BodycentricTransforms, HeliocentricToBodycentricHelioOrbit) {
  const JulianDate jd(J2000);
  BodycentricParams params = BodycentricParams::heliocentric(mars_orbit());

  auto earth_helio = ephemeris::earth_heliocentric(jd);
  auto result = to_bodycentric(earth_helio, params, jd);

  // Earth–Mars distance at J2000 ≈ 0.5–2.5 AU
  double r = vec_magnitude(result.x().value(), result.y().value(),
                           result.z().value());
  EXPECT_GT(r, 0.3);
  EXPECT_LT(r, 3.0);
  EXPECT_TRUE(std::isfinite(r));
}

// ============================================================================
// Round-Trip: Geocentric ↔ Bodycentric
// ============================================================================

TEST(BodycentricTransforms, RoundTripGeocentricBodycentric) {
  const JulianDate jd(J2000);
  BodycentricParams params =
      BodycentricParams::geocentric(satellite_orbit());

  cartesian::Position<Geocentric, EclipticMeanJ2000, AstronomicalUnit>
      original(0.001, 0.002, 0.003);

  auto bodycentric = to_bodycentric(original, params, jd);
  auto recovered   = bodycentric.to_geocentric(jd);

  EXPECT_NEAR(recovered.x().value(), original.x().value(), 1e-9);
  EXPECT_NEAR(recovered.y().value(), original.y().value(), 1e-9);
  EXPECT_NEAR(recovered.z().value(), original.z().value(), 1e-9);
}

TEST(BodycentricTransforms, RoundTripHeliocentricBodycentric) {
  const JulianDate jd(J2000);
  BodycentricParams params = BodycentricParams::heliocentric(mars_orbit());

  cartesian::Position<Geocentric, EclipticMeanJ2000, AstronomicalUnit>
      original(0.005, 0.003, 0.001);

  auto bodycentric = to_bodycentric(original, params, jd);
  auto recovered   = bodycentric.to_geocentric(jd);

  EXPECT_NEAR(recovered.x().value(), original.x().value(), 1e-9);
  EXPECT_NEAR(recovered.y().value(), original.y().value(), 1e-9);
  EXPECT_NEAR(recovered.z().value(), original.z().value(), 1e-9);
}

// ============================================================================
// Body at its own position → should be at origin
// ============================================================================

TEST(BodycentricTransforms, BodyOwnPositionAtOrigin) {
  const JulianDate jd(J2000);
  Orbit orbit      = satellite_orbit();
  BodycentricParams params = BodycentricParams::geocentric(orbit);

  // Get the satellite's own geocentric position
  auto body_geo = kepler_position<Geocentric>(orbit, jd);

  // Transform to body-centric
  auto body_from_body = to_bodycentric(body_geo, params, jd);

  double r = vec_magnitude(body_from_body.x().value(),
                           body_from_body.y().value(),
                           body_from_body.z().value());
  // The body's own position should be at (or very near) the origin
  EXPECT_LT(r, 1e-10);
}

// ============================================================================
// Different source centers
// ============================================================================

TEST(BodycentricTransforms, HeliocentricOrbitWithGeocentricOrbit) {
  const JulianDate jd(J2000);
  // Geocentric orbit for the body, but heliocentric position for the target
  BodycentricParams params =
      BodycentricParams::geocentric(satellite_orbit());

  // Moon geocentric at ~0.00257 AU
  cartesian::Position<Geocentric, EclipticMeanJ2000, AstronomicalUnit>
      moon_geo(0.00257, 0.0, 0.0);

  auto moon_from_sat = to_bodycentric(moon_geo, params, jd);

  double r = vec_magnitude(moon_from_sat.x().value(), moon_from_sat.y().value(),
                           moon_from_sat.z().value());
  // Moon at ~384400 km, ISS at ~6378 km → distance should be close to moon distance
  EXPECT_NEAR(r, 0.00257, 0.0002);
}

// ============================================================================
// FFI direct: null pointer guard + invalid center
// ============================================================================

TEST(BodycentricFFI, NullOutputPointer_ToBodycentric) {
  siderust_cartesian_pos_t pos{1.0, 0.0, 0.0, SIDERUST_FRAME_T_ECLIPTIC_MEAN_J2000,
                               SIDERUST_CENTER_T_HELIOCENTRIC};
  SiderustBodycentricParams params{};
  params.orbit = {1.0, 0.0, 0.0, 0.0, 0.0, 0.0, J2000};
  params.orbit_center = 1; // Heliocentric

  auto s = siderust_to_bodycentric(pos, params, J2000, nullptr);
  EXPECT_EQ(s, SIDERUST_STATUS_T_NULL_POINTER);
}

TEST(BodycentricFFI, NullOutputPointer_FromBodycentric) {
  siderust_cartesian_pos_t pos{0.5, 0.0, 0.0, SIDERUST_FRAME_T_ECLIPTIC_MEAN_J2000,
                               SIDERUST_CENTER_T_BODYCENTRIC};
  SiderustBodycentricParams params{};
  params.orbit = {1.0, 0.0, 0.0, 0.0, 0.0, 0.0, J2000};
  params.orbit_center = 1;

  auto s = siderust_from_bodycentric(pos, params, J2000, nullptr);
  EXPECT_EQ(s, SIDERUST_STATUS_T_NULL_POINTER);
}

TEST(BodycentricFFI, InvalidCenterInput) {
  siderust_cartesian_pos_t pos{1.0, 0.0, 0.0, SIDERUST_FRAME_T_ECLIPTIC_MEAN_J2000,
                               SIDERUST_CENTER_T_TOPOCENTRIC}; // unsupported center
  siderust_cartesian_pos_t out{};
  SiderustBodycentricParams params{};
  params.orbit = {1.0, 0.0, 0.0, 0.0, 0.0, 0.0, J2000};
  params.orbit_center = 1;

  auto s = siderust_to_bodycentric(pos, params, J2000, &out);
  EXPECT_EQ(s, SIDERUST_STATUS_T_INVALID_CENTER);
}

TEST(BodycentricFFI, NullOutputPointer_KeplerPosition) {
  siderust_orbit_t orbit{1.0, 0.0, 0.0, 0.0, 0.0, 0.0, J2000};
  auto s = siderust_kepler_position(orbit, J2000, nullptr);
  EXPECT_EQ(s, SIDERUST_STATUS_T_NULL_POINTER);
}

TEST(BodycentricFFI, KeplerPositionReturnsFinite) {
  siderust_orbit_t orbit{1.524, 0.0934, 1.85, 49.56, 286.5, 19.41, J2000};
  siderust_cartesian_pos_t out{};
  auto s = siderust_kepler_position(orbit, J2000, &out);
  EXPECT_EQ(s, SIDERUST_STATUS_T_OK);
  EXPECT_TRUE(std::isfinite(out.x));
  EXPECT_TRUE(std::isfinite(out.y));
  EXPECT_TRUE(std::isfinite(out.z));
  EXPECT_EQ(out.frame, SIDERUST_FRAME_T_ECLIPTIC_MEAN_J2000);
}

// ============================================================================
// Venus VSOP87 (using new ephemeris function)
// ============================================================================

TEST(BodycentricTransforms, VenusHeliocentricIsFinite) {
  const JulianDate jd(J2000);
  auto venus = ephemeris::venus_heliocentric(jd);
  EXPECT_TRUE(std::isfinite(venus.x().value()));
  EXPECT_TRUE(std::isfinite(venus.y().value()));
  EXPECT_TRUE(std::isfinite(venus.z().value()));

  double r = vec_magnitude(venus.x().value(), venus.y().value(), venus.z().value());
  // Venus at ~0.72 AU from Sun
  EXPECT_NEAR(r, 0.72, 0.05);
}
