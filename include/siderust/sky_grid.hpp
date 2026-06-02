#pragma once

/**
 * @file sky_grid.hpp
 * @brief Hemispherical alt/az sky-grid sampler mirroring
 * `siderust::coordinates::SkyGrid`.
 *
 * Materialises every cell of a hemispherical altitude/azimuth grid as a
 * Horizontal direction together with its approximate solid angle, wrapping the
 * siderust-ffi `siderust_sky_grid_cells` API with an RAII-managed output
 * vector.
 */

#include "coordinates/spherical.hpp"
#include "ffi_core.hpp"
#include <qtty/qtty.hpp>
#include <vector>

namespace siderust {

namespace detail {

struct SkyGridCellsGuard {
  SiderustSkyGridCell *ptr = nullptr;
  uintptr_t count = 0;

  ~SkyGridCellsGuard() { siderust_sky_grid_cells_free(ptr, count); }

  SkyGridCellsGuard() = default;
  SkyGridCellsGuard(const SkyGridCellsGuard &) = delete;
  SkyGridCellsGuard &operator=(const SkyGridCellsGuard &) = delete;
  SkyGridCellsGuard(SkyGridCellsGuard &&) = delete;
  SkyGridCellsGuard &operator=(SkyGridCellsGuard &&) = delete;
};

} // namespace detail

/**
 * @brief A single sky-grid cell: a Horizontal direction and its solid angle.
 *
 * Mirrors `siderust::coordinates::SkyGridCell`.
 */
struct SkyGridCell {
  spherical::Direction<frames::Horizontal> direction; ///< Alt/az sky direction.
  qtty::Steradian solid_angle;                        ///< Approximate solid angle of the cell.

  static SkyGridCell from_c(const SiderustSkyGridCell &c) {
    return SkyGridCell{spherical::Direction<frames::Horizontal>(qtty::Degree(c.azimuth_deg),
                                                                qtty::Degree(c.altitude_deg)),
                       qtty::Steradian(c.solid_angle_sr)};
  }
};

/**
 * @brief Typed hemispherical alt/az grid sampler.
 *
 * Mirrors `siderust::coordinates::SkyGrid`. Construct with one of the static
 * factories and call @ref cells to materialise every grid cell.
 */
class SkyGrid {
  double alt_min_;
  double alt_max_;
  double alt_step_;
  double az_step_;
  bool equal_area_;

  SkyGrid(double alt_min, double alt_max, double alt_step, double az_step, bool equal_area)
      : alt_min_(alt_min), alt_max_(alt_max), alt_step_(alt_step), az_step_(az_step),
        equal_area_(equal_area) {}

public:
  /// Uniform grid with equal altitude and azimuth steps over `[0°, 90°)`.
  static SkyGrid uniform(qtty::Degree step) {
    return SkyGrid(0.0, 90.0, step.value(), step.value(), false);
  }

  /// Uniform grid with independent altitude and azimuth steps over `[0°, 90°)`.
  static SkyGrid with_steps(qtty::Degree alt_step, qtty::Degree az_step) {
    return SkyGrid(0.0, 90.0, alt_step.value(), az_step.value(), false);
  }

  /// Equal-area grid: azimuth count per altitude ring scales with `cos(alt)`.
  static SkyGrid equal_area(qtty::Degree alt_step, qtty::Degree az_step_at_horizon) {
    return SkyGrid(0.0, 90.0, alt_step.value(), az_step_at_horizon.value(), true);
  }

  /// Restrict the altitude range (builder, e.g. a `10°..90°` horizon mask).
  SkyGrid &with_alt_range(qtty::Degree lo, qtty::Degree hi) {
    alt_min_ = lo.value();
    alt_max_ = hi.value();
    return *this;
  }

  /// Materialise every cell of the grid.
  std::vector<SkyGridCell> cells() const {
    SiderustSkyGridCell *ptr = nullptr;
    uintptr_t count = 0;
    check_status(
        siderust_sky_grid_cells(alt_min_, alt_max_, alt_step_, az_step_, equal_area_, &ptr, &count),
        "SkyGrid::cells");

    detail::SkyGridCellsGuard guard{};
    guard.ptr = ptr;
    guard.count = count;

    std::vector<SkyGridCell> result;
    result.reserve(count);
    for (uintptr_t i = 0; i < count; ++i) {
      result.push_back(SkyGridCell::from_c(ptr[i]));
    }
    return result;
  }

  /// Number of cells the grid materialises.
  std::size_t size() const { return cells().size(); }
};

} // namespace siderust
