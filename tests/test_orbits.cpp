// SPDX-License-Identifier: AGPL-3.0-or-later
// Copyright (C) 2026 Vallés Puig, Ramon

#include <cmath>
#include <gtest/gtest.h>
#include <siderust/siderust.hpp>
#include <type_traits>

using namespace siderust;
using namespace qtty::literals;

namespace {

constexpr double J2000 = 2451545.0;

KeplerianOrbit earth_like_keplerian() {
  return {1.0_au, 0.0167, 0.0_deg, 0.0_deg, 102.9_deg, 100.0_deg, J2000};
}

MeanMotionOrbit earth_like_mean_motion() {
  return {1.0_au, 0.0167, 0.0_deg, 0.0_deg, 102.9_deg, 0.9856, J2000};
}

ConicOrbit halley_like_conic() {
  return {0.586_au, 0.967, 162.3_deg, 58.4_deg, 111.3_deg, 38.4_deg, J2000};
}

ConicOrbit hyperbolic_conic() {
  return {0.255_au, 1.2, 122.7_deg, 24.6_deg, 241.8_deg, 12.0_deg, J2000};
}

} // namespace

TEST(Orbits, OrbitAliasMatchesKeplerianOrbit) {
  static_assert(std::is_same_v<Orbit, KeplerianOrbit>);
}

TEST(Orbits, OrbitAliasPropagatesWithKeplerHelper) {
  Orbit orbit = earth_like_keplerian();
  auto pos = kepler_position(orbit, JulianDate(J2000));

  EXPECT_TRUE(std::isfinite(pos.x().value()));
  EXPECT_TRUE(std::isfinite(pos.y().value()));
  EXPECT_TRUE(std::isfinite(pos.z().value()));
  EXPECT_GT(pos.distance().value(), 0.9);
  EXPECT_LT(pos.distance().value(), 1.1);
}

TEST(Orbits, MeanMotionOrbitPropagates) {
  auto pos = earth_like_mean_motion().position_at(JulianDate(J2000 + 42.0));

  EXPECT_TRUE(std::isfinite(pos.x().value()));
  EXPECT_TRUE(std::isfinite(pos.y().value()));
  EXPECT_TRUE(std::isfinite(pos.z().value()));
  EXPECT_GT(pos.distance().value(), 0.9);
  EXPECT_LT(pos.distance().value(), 1.1);
}

TEST(Orbits, ConicOrbitClassifiesKinds) {
  EXPECT_EQ(halley_like_conic().kind(), ConicKind::Elliptic);
  EXPECT_EQ(hyperbolic_conic().kind(), ConicKind::Hyperbolic);
}

TEST(Orbits, ConicOrbitHyperbolicPropagationProducesFinitePosition) {
  auto pos = hyperbolic_conic().position_at(JulianDate(J2000 + 20.0));

  EXPECT_TRUE(std::isfinite(pos.x().value()));
  EXPECT_TRUE(std::isfinite(pos.y().value()));
  EXPECT_TRUE(std::isfinite(pos.z().value()));
}

TEST(Orbits, PreparedOrbitMatchesDirectKeplerianPropagation) {
  auto orbit = earth_like_keplerian();
  PreparedOrbit prepared(orbit);
  auto jd = JulianDate(J2000 + 17.25);

  auto direct = kepler_position(orbit, jd);
  auto cached = prepared.position_at(jd);

  EXPECT_NEAR(cached.x().value(), direct.x().value(), 1e-12);
  EXPECT_NEAR(cached.y().value(), direct.y().value(), 1e-12);
  EXPECT_NEAR(cached.z().value(), direct.z().value(), 1e-12);
}

TEST(Orbits, PreparedOrbitRejectsNonEllipticElements) {
  KeplerianOrbit invalid = earth_like_keplerian();
  invalid.eccentricity = 1.1;

  EXPECT_THROW({ PreparedOrbit prepared(invalid); }, InvalidArgumentError);
}

TEST(Orbits, ConicOrbitRejectsParabolicPropagation) {
  ConicOrbit invalid = hyperbolic_conic();
  invalid.eccentricity = 1.0;

  EXPECT_THROW(invalid.position_at(JulianDate(J2000)), InvalidArgumentError);
}
