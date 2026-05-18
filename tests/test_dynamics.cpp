// SPDX-License-Identifier: AGPL-3.0-or-later
// Copyright (C) 2026 Vallés Puig, Ramon

#include <cmath>
#include <gtest/gtest.h>
#include <siderust/siderust.hpp>

using namespace siderust;
using namespace siderust::dynamics;

namespace {

constexpr double J2000 = 2451545.0;

/// ISS-like LEO orbit: 7000 km circular, equatorial.
OrbitState iss_like_state() {
  return OrbitState(J2000, 7000.0, 0.0, 0.0, 0.0, 7.545831, 0.0);
}

/// One-hour propagation step.
constexpr double ONE_HOUR_S = 3600.0;

} // namespace

// ============================================================================
// DynamicsContext lifecycle
// ============================================================================

TEST(DynamicsContext, DefaultConstructionSucceeds) {
  EXPECT_NO_THROW(DynamicsContext ctx);
}

// ============================================================================
// OrbitState lifecycle and accessors
// ============================================================================

TEST(OrbitState, ConstructionAndPositionRoundTrip) {
  OrbitState s(J2000, 7000.0, 0.0, 0.0, 0.0, 7.5, 0.0);
  auto pos = s.position();

  EXPECT_NEAR(pos[0], 7000.0, 1e-9);
  EXPECT_NEAR(pos[1], 0.0, 1e-9);
  EXPECT_NEAR(pos[2], 0.0, 1e-9);
}

TEST(OrbitState, VelocityRoundTrip) {
  OrbitState s(J2000, 0.0, 7000.0, 0.0, 0.0, 0.0, 7.5);
  auto vel = s.velocity();

  EXPECT_NEAR(vel[0], 0.0, 1e-9);
  EXPECT_NEAR(vel[1], 0.0, 1e-9);
  EXPECT_NEAR(vel[2], 7.5, 1e-9);
}

TEST(OrbitState, EpochRoundTrip) {
  OrbitState s(J2000 + 10.5, 7000.0, 0.0, 0.0, 0.0, 7.5, 0.0);
  EXPECT_NEAR(s.epoch_jd(), J2000 + 10.5, 1e-9);
}

TEST(OrbitState, StateVectorConstructorRoundTrip) {
  StateVector sv;
  sv.epoch_jd = J2000;
  sv.x_km = 6800.0;
  sv.y_km = 100.0;
  sv.z_km = -200.0;
  sv.vx_km_s = 0.1;
  sv.vy_km_s = 7.6;
  sv.vz_km_s = -0.3;

  OrbitState s(sv);
  auto got = s.to_vector();

  EXPECT_NEAR(got.epoch_jd, sv.epoch_jd, 1e-12);
  EXPECT_NEAR(got.x_km, sv.x_km, 1e-9);
  EXPECT_NEAR(got.y_km, sv.y_km, 1e-9);
  EXPECT_NEAR(got.z_km, sv.z_km, 1e-9);
  EXPECT_NEAR(got.vx_km_s, sv.vx_km_s, 1e-9);
  EXPECT_NEAR(got.vy_km_s, sv.vy_km_s, 1e-9);
  EXPECT_NEAR(got.vz_km_s, sv.vz_km_s, 1e-9);
}

// ============================================================================
// TwoBodyPropagator lifecycle
// ============================================================================

TEST(TwoBodyPropagator, DefaultEarthConstructionSucceeds) {
  EXPECT_NO_THROW(TwoBodyPropagator p);
}

TEST(TwoBodyPropagator, CustomGmConstructionSucceeds) {
  EXPECT_NO_THROW(TwoBodyPropagator p(398600.4418));
}

// ============================================================================
// Two-body propagation correctness
// ============================================================================

TEST(TwoBodyPropagator, PropagatedStateHasFiniteComponents) {
  TwoBodyPropagator prop;
  auto final_s = prop.propagate(iss_like_state(), ONE_HOUR_S);

  const auto pos = final_s.position();
  const auto vel = final_s.velocity();

  EXPECT_TRUE(std::isfinite(pos[0]));
  EXPECT_TRUE(std::isfinite(pos[1]));
  EXPECT_TRUE(std::isfinite(pos[2]));
  EXPECT_TRUE(std::isfinite(vel[0]));
  EXPECT_TRUE(std::isfinite(vel[1]));
  EXPECT_TRUE(std::isfinite(vel[2]));
}

TEST(TwoBodyPropagator, RadiusConservation) {
  TwoBodyPropagator prop;
  auto s0 = iss_like_state();
  auto s1 = prop.propagate(s0, ONE_HOUR_S);

  auto p0 = s0.position();
  auto p1 = s1.position();

  double r0 = std::sqrt(p0[0]*p0[0] + p0[1]*p0[1] + p0[2]*p0[2]);
  double r1 = std::sqrt(p1[0]*p1[0] + p1[1]*p1[1] + p1[2]*p1[2]);

  // For a circular orbit the radius stays within 1 km over one period.
  EXPECT_NEAR(r0, r1, 1.0);
}

TEST(TwoBodyPropagator, EpochAdvancesByDt) {
  TwoBodyPropagator prop;
  auto s0 = iss_like_state();
  auto s1 = prop.propagate(s0, ONE_HOUR_S);

  constexpr double one_hour_jd = ONE_HOUR_S / 86400.0;
  EXPECT_NEAR(s1.epoch_jd() - s0.epoch_jd(), one_hour_jd, 1e-9);
}

TEST(TwoBodyPropagator, BackwardPropagationRoundTrip) {
  TwoBodyPropagator prop;
  auto s0 = iss_like_state();
  auto s1 = prop.propagate(s0, ONE_HOUR_S);
  auto s_back = prop.propagate(s1, -ONE_HOUR_S);

  auto p0 = s0.position();
  auto p2 = s_back.position();

  EXPECT_NEAR(p2[0], p0[0], 1e-6);
  EXPECT_NEAR(p2[1], p0[1], 1e-6);
  EXPECT_NEAR(p2[2], p0[2], 1e-6);
}

TEST(TwoBodyPropagator, PropagationWithEmptyContextMatchesNullContext) {
  TwoBodyPropagator prop;
  DynamicsContext ctx;

  auto s_null = prop.propagate(iss_like_state(), ONE_HOUR_S, nullptr);
  auto s_ctx  = prop.propagate(iss_like_state(), ONE_HOUR_S, &ctx);

  auto p_null = s_null.position();
  auto p_ctx  = s_ctx.position();

  EXPECT_NEAR(p_ctx[0], p_null[0], 1e-9);
  EXPECT_NEAR(p_ctx[1], p_null[1], 1e-9);
  EXPECT_NEAR(p_ctx[2], p_null[2], 1e-9);
}

TEST(TwoBodyPropagator, CustomGmGivesConsistentRadius) {
  // Sun GM (km³/s²): Earth-like orbit but around the Sun.
  constexpr double GM_SUN = 1.327124400e11;
  TwoBodyPropagator solar_prop(GM_SUN);

  OrbitState earth_like(J2000, 149597870.7, 0.0, 0.0, 0.0, 29.783, 0.0);
  auto final_s = solar_prop.propagate(earth_like, 86400.0 * 30.0);

  auto p = final_s.position();
  double r = std::sqrt(p[0]*p[0] + p[1]*p[1] + p[2]*p[2]);

  // Radius stays within 1% of 1 AU for a near-circular solar orbit.
  EXPECT_NEAR(r, 149597870.7, 1.5e6);
}

TEST(TwoBodyPropagator, MoveSemantics) {
  TwoBodyPropagator p1;
  TwoBodyPropagator p2 = std::move(p1);
  EXPECT_FALSE(static_cast<bool>(p1)); // NOLINT: intentional moved-from check
  EXPECT_TRUE(static_cast<bool>(p2));
}
