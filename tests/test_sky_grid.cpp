// SPDX-License-Identifier: AGPL-3.0-or-later
// Copyright (C) 2026 Vallés Puig, Ramon

// Tests for the SkyGrid sampler mirrored from siderust::coordinates::SkyGrid.

#include <cmath>

#include <gtest/gtest.h>
#include <siderust/siderust.hpp>

using namespace siderust;

TEST(SkyGrid, UniformGridCellCount) {
  // 10° uniform grid over [0,90): 9 altitude rings × 36 azimuth steps.
  auto grid = SkyGrid::uniform(qtty::Degree(10.0));
  auto cells = grid.cells();
  EXPECT_EQ(cells.size(), static_cast<std::size_t>(9 * 36));
}

TEST(SkyGrid, DirectionsAreWithinHemisphere) {
  auto grid = SkyGrid::with_steps(qtty::Degree(15.0), qtty::Degree(30.0));
  for (const auto &cell : grid.cells()) {
    double alt = cell.direction.alt().value();
    double az = cell.direction.az().value();
    EXPECT_GE(alt, 0.0);
    EXPECT_LT(alt, 90.0);
    EXPECT_GE(az, 0.0);
    EXPECT_LT(az, 360.0);
    EXPECT_GT(cell.solid_angle.value(), 0.0);
  }
}

TEST(SkyGrid, EqualAreaSolidAnglesSumToHemisphere) {
  auto grid = SkyGrid::equal_area(qtty::Degree(5.0), qtty::Degree(5.0));
  double total = 0.0;
  for (const auto &cell : grid.cells()) {
    total += cell.solid_angle.value();
  }
  const double hemisphere = 2.0 * constants::pi;
  EXPECT_NEAR(total, hemisphere, hemisphere * 0.01);
}

TEST(SkyGrid, AltRangeMaskReducesCells) {
  auto full = SkyGrid::uniform(qtty::Degree(10.0));
  auto masked = SkyGrid::uniform(qtty::Degree(10.0));
  masked.with_alt_range(qtty::Degree(30.0), qtty::Degree(90.0));
  EXPECT_LT(masked.cells().size(), full.cells().size());
}
