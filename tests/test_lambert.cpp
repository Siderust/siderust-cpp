// SPDX-License-Identifier: AGPL-3.0-or-later
// Copyright (C) 2026 Vallés Puig, Ramon

#include <cmath>
#include <gtest/gtest.h>
#include <siderust/siderust.hpp>

using namespace siderust;

namespace {

static constexpr double AU_KM = 1.495'978'707e8;
static constexpr double MU_SUN = 1.327'124'400'18e11;
static constexpr double SECONDS_PER_DAY = 86'400.0;

TEST(Lambert, EarthMarsPrograde) {
  const double r_earth = AU_KM;
  const double r_mars = 1.524 * AU_KM;
  const double phi = 60.0 * M_PI / 180.0;

  const double r1[3] = {r_earth, 0.0, 0.0};
  const double r2[3] = {r_mars * std::cos(phi), r_mars * std::sin(phi), 0.0};
  const double tof_s = 258.0 * SECONDS_PER_DAY;

  auto sol = lambert::solve(r1, r2, tof_s, MU_SUN);

  // Departure velocity should be small (~29 km/s heliocentric).
  const auto &v1 = sol.v1_kms;
  double v1_mag = std::sqrt(v1[0] * v1[0] + v1[1] * v1[1] + v1[2] * v1[2]);
  EXPECT_GT(v1_mag, 20.0);
  EXPECT_LT(v1_mag, 40.0);

  // Arrival velocity similarly bounded.
  const auto &v2 = sol.v2_kms;
  double v2_mag = std::sqrt(v2[0] * v2[0] + v2[1] * v2[1] + v2[2] * v2[2]);
  EXPECT_GT(v2_mag, 15.0);
  EXPECT_LT(v2_mag, 35.0);

  // Solver should converge quickly.
  EXPECT_LT(sol.diag.iterations, 50u);
  EXPECT_LT(sol.diag.residual, 1e-10);
  EXPECT_EQ(sol.diag.revolutions, 0u);
}

TEST(Lambert, RetrogradeBranchDiffers) {
  const double r1[3] = {AU_KM, 0.0, 0.0};
  const double r2[3] = {0.0, AU_KM, 0.0};
  const double tof_s = 100.0 * SECONDS_PER_DAY;

  auto sol_pro = lambert::solve(r1, r2, tof_s, MU_SUN, lambert::Branch::Prograde);
  auto sol_retro = lambert::solve(r1, r2, tof_s, MU_SUN, lambert::Branch::Retrograde);

  // The two solutions should differ.
  EXPECT_NE(sol_pro.v1_kms[1], sol_retro.v1_kms[1]);
}

TEST(Lambert, CircularOrbit90deg) {
  // Quarter-circle transfer on a circular orbit: r1 = {r, 0, 0} → r2 = {0, r, 0}.
  // Exact solution: departure velocity = circular speed in +y, arrival in -x.
  // This avoids the degenerate anti-parallel (θ = π) case.
  const double r_km = 7'000.0;
  const double MU_EARTH = 398'600.4418;

  // ToF for a quarter orbit.
  double tof = (M_PI / 2.0) * std::sqrt(r_km * r_km * r_km / MU_EARTH);

  const double r1[3] = {r_km, 0.0, 0.0};
  const double r2[3] = {0.0, r_km, 0.0};

  auto sol = lambert::solve(r1, r2, tof, MU_EARTH);

  // v1 should be {0, v_circ, 0} for an exact circular orbit.
  double v_circ = std::sqrt(MU_EARTH / r_km);
  EXPECT_NEAR(sol.v1_kms[0], 0.0, 0.01);
  EXPECT_NEAR(sol.v1_kms[1], v_circ, 0.01);
  EXPECT_NEAR(sol.v1_kms[2], 0.0, 1e-9);

  // v2 should be {-v_circ, 0, 0}.
  EXPECT_NEAR(sol.v2_kms[0], -v_circ, 0.01);
  EXPECT_NEAR(sol.v2_kms[1], 0.0, 0.01);
  EXPECT_NEAR(sol.v2_kms[2], 0.0, 1e-9);
}

} // namespace
