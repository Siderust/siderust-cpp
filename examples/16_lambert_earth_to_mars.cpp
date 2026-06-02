// SPDX-License-Identifier: AGPL-3.0-or-later
// Copyright (C) 2026 Vallés Puig, Ramon

/**
 * @file 16_lambert_earth_to_mars.cpp
 * @brief Worked example: heliocentric Earth → Mars Lambert transfer.
 *
 * C++17 mirror of the Rust example `16_lambert_earth_to_mars.rs`.
 *
 * Approximates a single-revolution prograde transfer from a notional Earth
 * position to a notional Mars position 60° ahead, using a Hohmann-like time of
 * flight.  Prints departure / arrival velocities and the implied Δv at each end.
 */

#include <siderust/constants.hpp>
#include <siderust/lambert.hpp>

#include <cmath>
#include <iomanip>
#include <iostream>

static constexpr double AU_KM = 1.495'978'707e8;
static constexpr double MU_SUN = 1.327'124'400'18e11; // km³ / s²
static constexpr double SECONDS_PER_DAY = 86'400.0;

int main() {
  const double r_earth_km = AU_KM;
  const double r_mars_km = 1.524 * AU_KM;
  const double phase_lead = 60.0 * siderust::constants::pi / 180.0;

  const double r1[3] = {r_earth_km, 0.0, 0.0};
  const double r2[3] = {
      r_mars_km * std::cos(phase_lead),
      r_mars_km * std::sin(phase_lead),
      0.0,
  };

  const double tof_days = 258.0;
  const double tof_s = tof_days * SECONDS_PER_DAY;

  auto sol = siderust::lambert::solve(r1, r2, tof_s, MU_SUN);

  const auto &v1 = sol.v1_kms;
  const auto &v2 = sol.v2_kms;
  const double v1_mag = std::sqrt(v1[0] * v1[0] + v1[1] * v1[1] + v1[2] * v1[2]);
  const double v2_mag = std::sqrt(v2[0] * v2[0] + v2[1] * v2[1] + v2[2] * v2[2]);

  const double v_earth_circ = std::sqrt(MU_SUN / r_earth_km);
  const double v_mars_circ = std::sqrt(MU_SUN / r_mars_km);

  auto hypot3 = [](double a, double b, double c) { return std::sqrt(a * a + b * b + c * c); };
  const double dv_dep = hypot3(v1[0] - 0.0, v1[1] - v_earth_circ, v1[2]);
  const double dv_arr = hypot3(v2[0] - (-v_mars_circ * std::sin(phase_lead)),
                               v2[1] - (v_mars_circ * std::cos(phase_lead)), v2[2]);

  std::cout << std::fixed << std::setprecision(3);
  std::cout << "=== Lambert Earth → Mars (toy heliocentric placement) ===\n";
  std::cout << "  r1 = (" << r_earth_km << ", 0, 0) km\n";
  std::cout << "  r2 = (" << r_mars_km * std::cos(phase_lead) << ", "
            << r_mars_km * std::sin(phase_lead) << ", 0) km\n";
  std::cout << "  ToF = " << tof_days << " d (" << std::setprecision(0) << tof_s << " s)\n\n";

  std::cout << std::setprecision(4) << sol << "\n\n";
  std::cout << "|v1| = " << v1_mag << " km/s, |v2| = " << v2_mag << " km/s\n\n";

  std::cout << "Δv accounting (vs. circular planetary motion):\n";
  std::cout << "  Δv at Earth  = " << dv_dep << " km/s\n";
  std::cout << "  Δv at Mars   = " << dv_arr << " km/s\n";
  std::cout << "  Δv total     = " << (dv_dep + dv_arr) << " km/s\n";

  return 0;
}
