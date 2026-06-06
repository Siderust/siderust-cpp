// SPDX-License-Identifier: AGPL-3.0-or-later
// Copyright (C) 2026 Vallés Puig, Ramon

/// @file 15_orbit_models.cpp
/// @brief Orbit model overview for the C++ bindings.

#include <cmath>
#include <iostream>
#include <siderust/siderust.hpp>

using namespace siderust;
using namespace qtty::literals;

int main() {
  const Time<TT, JD> jd(2458850.0);

  std::cout << "=== Orbit Models ===\n\n";
  std::cout << "Epoch: " << jd << "\n\n";

  const KeplerianOrbit kepler{1.0_au,    Eccentricity{0.0167},   0.0_deg, 0.0_deg, 102.9_deg,
                              100.0_deg, Time<TT, JD>(2451545.0)};
  const auto kepler_pos = kepler_position(kepler, jd);
  std::cout << "1. KeplerianOrbit\n";
  std::cout << "   heliocentric position = " << kepler_pos << '\n';
  std::cout << "   radius = " << kepler_pos.distance() << "\n\n";

  const MeanMotionOrbit mean_motion{1.0_au,
                                    Eccentricity{0.0167},
                                    0.0_deg,
                                    0.0_deg,
                                    102.9_deg,
                                    AngularRate{qtty::Degree(0.9856), qtty::Day(1.0)},
                                    Time<TT, JD>(2451545.0)};
  const auto mean_motion_pos = mean_motion.position_at(jd);
  std::cout << "2. MeanMotionOrbit\n";
  std::cout << "   heliocentric position = " << mean_motion_pos << '\n';
  std::cout << "   radius = " << mean_motion_pos.distance() << "\n\n";

  const ConicOrbit halley_like{0.586_au, Eccentricity{0.967},    162.3_deg, 58.4_deg, 111.3_deg,
                               38.4_deg, Time<TT, JD>(2451545.0)};
  const ConicOrbit hyperbolic{0.255_au, Eccentricity{1.2},      122.7_deg, 24.6_deg, 241.8_deg,
                              12.0_deg, Time<TT, JD>(2451545.0)};
  const auto halley_pos = halley_like.position_at(jd);
  const auto hyperbolic_pos = hyperbolic.position_at(jd);
  std::cout << "3. ConicOrbit\n";
  std::cout << "   halley-like kind = " << halley_like.kind() << '\n';
  std::cout << "   hyperbolic kind  = " << hyperbolic.kind() << '\n';
  std::cout << "   elliptic radius  = " << halley_pos.distance()
            << " | hyperbolic radius = " << hyperbolic_pos.distance() << "\n\n";

  const PreparedOrbit prepared(kepler);
  const auto prepared_pos = prepared.position_at(jd);
  const double delta = std::sqrt(std::pow(kepler_pos.x().value() - prepared_pos.x().value(), 2) +
                                 std::pow(kepler_pos.y().value() - prepared_pos.y().value(), 2) +
                                 std::pow(kepler_pos.z().value() - prepared_pos.z().value(), 2));
  std::cout << std::scientific << std::setprecision(3);
  std::cout << "4. PreparedOrbit\n";
  std::cout << "   prepared radius = " << prepared_pos.distance() << '\n';
  std::cout << "   chord delta vs direct Keplerian = " << delta << '\n';

  return 0;
}
