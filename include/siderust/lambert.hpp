#pragma once

/**
 * @file lambert.hpp
 * @brief Lambert solver C++ wrapper.
 *
 * Wraps `siderust_lambert_solve` (Izzo's algorithm) with a value-semantic C++17
 * interface.  All inputs/outputs use plain `double` arrays — no heavy unit types
 * cross the FFI; callers can wrap results in qtty types as desired.
 *
 * ### Typical usage
 * @code
 * #include <siderust/lambert.hpp>
 *
 * double r1[3] = { 1.496e8, 0.0, 0.0 };   // Earth position, km
 * double r2[3] = { 1.978e8, 1.142e8, 0.0 }; // Mars position, km
 * auto sol = siderust::lambert::solve(r1, r2, 258.0 * 86400.0, 1.327e11);
 * // sol.v1_kms  — departure velocity in km/s
 * // sol.v2_kms  — arrival velocity in km/s
 * @endcode
 */

#include "ffi_core.hpp"

#include <array>
#include <cstdint>
#include <ostream>

namespace siderust {

/// @defgroup lambert Lambert solver
/// @{
namespace lambert {

/// Branch selector for the Lambert solver.
enum class Branch : int {
  Prograde = 0,   ///< Direct (prograde) single-revolution transfer.
  Retrograde = 1, ///< Retrograde single-revolution transfer.
};

/// Householder-iteration diagnostics accompanying a Lambert solution.
struct Diagnostics {
  std::uint32_t iterations;  ///< Number of Householder iterations performed.
  double residual;           ///< Final residual (dimensionless).
  std::uint32_t revolutions; ///< Number of complete revolutions (0 = single-rev).
};

/// Complete solution to a Lambert boundary-value problem.
struct Solution {
  std::array<double, 3> v1_kms; ///< Departure velocity [vx, vy, vz], km/s.
  std::array<double, 3> v2_kms; ///< Arrival velocity [vx, vy, vz], km/s.
  Diagnostics diag;             ///< Solver diagnostics.
};

namespace detail {

inline Solution solve_impl(const double *r1_km, const double *r2_km, double tof_s, double mu_km3_s2,
                           Branch branch) {
  Solution sol{};
  SiderustLambertDiagnostics diag{};
  const auto status =
      siderust_lambert_solve(r1_km, r2_km, tof_s, mu_km3_s2, static_cast<int>(branch),
                             sol.v1_kms.data(), sol.v2_kms.data(), &diag);
  check_status(status, "lambert::solve");
  sol.diag = {diag.iterations, diag.residual, diag.revolutions};
  return sol;
}

} // namespace detail

/**
 * @brief Solve Lambert's single-revolution two-point boundary-value problem.
 *
 * @param r1_km       Departure position [x, y, z] in km (3 elements).
 * @param r2_km       Arrival position [x, y, z] in km (3 elements).
 * @param tof_s       Time of flight in seconds.
 * @param mu_km3_s2   Gravitational parameter of the central body, km³·s⁻².
 * @param branch      Prograde (default) or retrograde transfer.
 * @return Solution   Departure/arrival velocities and solver diagnostics.
 *
 * @throws siderust::InvalidArgumentError  if the solver fails to converge or
 *         the input geometry is degenerate.
 */
inline Solution solve(const double (&r1_km)[3], const double (&r2_km)[3], double tof_s,
                      double mu_km3_s2, Branch branch = Branch::Prograde) {
  return detail::solve_impl(r1_km, r2_km, tof_s, mu_km3_s2, branch);
}

/// Overload accepting `std::array<double,3>` positions.
inline Solution solve(const std::array<double, 3> &r1_km, const std::array<double, 3> &r2_km,
                      double tof_s, double mu_km3_s2, Branch branch = Branch::Prograde) {
  return detail::solve_impl(r1_km.data(), r2_km.data(), tof_s, mu_km3_s2, branch);
}

/**
 * @brief Stream a Lambert solution (velocities in km/s).
 */
inline std::ostream &operator<<(std::ostream &os, const Solution &sol) {
  os << "Lambert (v1=(" << sol.v1_kms[0] << ", " << sol.v1_kms[1] << ", " << sol.v1_kms[2]
     << ") km/s, v2=(" << sol.v2_kms[0] << ", " << sol.v2_kms[1] << ", " << sol.v2_kms[2]
     << ") km/s, iters=" << sol.diag.iterations << ", residual=" << sol.diag.residual << ')';
  return os;
}

} // namespace lambert
/// @}

} // namespace siderust
