// SPDX-License-Identifier: AGPL-3.0-or-later
// Copyright (C) 2026 Vallés Puig, Ramon

/**
 * @file 19_sky_grid_sampling.cpp
 * @example 19_sky_grid_sampling.cpp
 * @brief Hemispherical alt/az sky-grid sampling (SkyGrid).
 *
 * Mirrors `siderust::coordinates::SkyGrid`: tessellate the upper hemisphere
 * into alt/az cells, each carrying an approximate solid angle. The equal-area
 * construction produces cells whose solid angles sum to the 2π sr hemisphere.
 *
 * Build and run:
 *   cmake -B build && cmake --build build
 *   ./build/19_sky_grid_sampling_example
 */

#include <cassert>
#include <cmath>
#include <iostream>

#include <siderust/siderust.hpp>

int main() {
  using namespace siderust;

  // Uniform 10° grid over the upper hemisphere: 9 alt rings × 36 az steps.
  auto uniform = SkyGrid::uniform(qtty::Degree(10.0));
  auto cells = uniform.cells();
  std::cout << "uniform 10deg grid: " << cells.size() << " cells\n";
  assert(cells.size() == static_cast<std::size_t>(9 * 36));

  // Equal-area grid: per-cell solid angles sum to the hemisphere (2pi sr).
  auto equal_area = SkyGrid::equal_area(qtty::Degree(5.0), qtty::Degree(5.0));
  double total_sr = 0.0;
  for (const auto &cell : equal_area.cells()) {
    total_sr += cell.solid_angle.value();
  }
  std::cout << "equal-area total solid angle: " << total_sr
            << " sr (hemisphere = " << 2.0 * constants::pi << " sr)\n";
  assert(std::abs(total_sr - 2.0 * constants::pi) / (2.0 * constants::pi) < 0.01);

  // Apply a 20deg horizon mask via the builder.
  auto masked = SkyGrid::with_steps(qtty::Degree(10.0), qtty::Degree(10.0));
  masked.with_alt_range(qtty::Degree(20.0), qtty::Degree(90.0));
  std::cout << "20deg horizon mask: " << masked.cells().size() << " cells\n";
  assert(masked.cells().size() < cells.size());

  std::cout << "SkyGrid sampling OK\n";
  return 0;
}
