// SPDX-License-Identifier: AGPL-3.0-or-later
// Copyright (C) 2026 Vallés Puig, Ramon

/// @file 15_orbit_models.cpp
/// @brief Orbit model overview for the C++ bindings.

#include <cmath>
#include <cstdio>
#include <siderust/siderust.hpp>

using namespace siderust;
using namespace qtty::literals;

int main() {
  const JulianDate jd(2458850.0);

  std::puts("=== Orbit Models ===\n");
  std::printf("Epoch: %.1f\n\n", jd.value());

  const KeplerianOrbit kepler{1.0_au, 0.0167, 0.0_deg, 0.0_deg, 102.9_deg, 100.0_deg, 2451545.0};
  const auto kepler_pos = kepler_position(kepler, jd);
  std::puts("1. KeplerianOrbit");
  std::printf("   heliocentric position = (%.12f, %.12f, %.12f) AU\n", kepler_pos.x().value(),
              kepler_pos.y().value(), kepler_pos.z().value());
  std::printf("   radius = %.12f AU\n\n", kepler_pos.distance().value());

  const MeanMotionOrbit mean_motion{1.0_au, 0.0167, 0.0_deg, 0.0_deg, 102.9_deg, 0.9856, 2451545.0};
  const auto mean_motion_pos = mean_motion.position_at(jd);
  std::puts("2. MeanMotionOrbit");
  std::printf("   heliocentric position = (%.12f, %.12f, %.12f) AU\n", mean_motion_pos.x().value(),
              mean_motion_pos.y().value(), mean_motion_pos.z().value());
  std::printf("   radius = %.12f AU\n\n", mean_motion_pos.distance().value());

  const ConicOrbit halley_like{0.586_au,  0.967,    162.3_deg, 58.4_deg,
                               111.3_deg, 38.4_deg, 2451545.0};
  const ConicOrbit hyperbolic{0.255_au, 1.2, 122.7_deg, 24.6_deg, 241.8_deg, 12.0_deg, 2451545.0};
  const auto halley_pos = halley_like.position_at(jd);
  const auto hyperbolic_pos = hyperbolic.position_at(jd);
  std::puts("3. ConicOrbit");
  std::printf("   halley-like kind = %s\n",
              halley_like.kind() == ConicKind::Elliptic ? "Elliptic" : "Hyperbolic");
  std::printf("   hyperbolic kind  = %s\n",
              hyperbolic.kind() == ConicKind::Elliptic ? "Elliptic" : "Hyperbolic");
  std::printf("   elliptic radius  = %.12f AU | hyperbolic radius = %.12f AU\n\n",
              halley_pos.distance().value(), hyperbolic_pos.distance().value());

  const PreparedOrbit prepared(kepler);
  const auto prepared_pos = prepared.position_at(jd);
  const double delta = std::sqrt(std::pow(kepler_pos.x().value() - prepared_pos.x().value(), 2) +
                                 std::pow(kepler_pos.y().value() - prepared_pos.y().value(), 2) +
                                 std::pow(kepler_pos.z().value() - prepared_pos.z().value(), 2));
  std::puts("4. PreparedOrbit");
  std::printf("   prepared radius = %.12f AU\n", prepared_pos.distance().value());
  std::printf("   chord delta vs direct Keplerian = %.3e\n", delta);

  return 0;
}
