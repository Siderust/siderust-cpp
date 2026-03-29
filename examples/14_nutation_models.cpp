// SPDX-License-Identifier: AGPL-3.0-or-later
// Copyright (C) 2026 Vallés Puig, Ramon

/// @file 14_nutation_models.cpp
/// @brief Nutation model selection example for the C++ bindings.

#include <cmath>
#include <cstdio>
#include <siderust/siderust.hpp>

using namespace siderust;
using namespace siderust::frames;

namespace {

double chord_delta(const cartesian::Direction<EquatorialTrueOfDate> &a,
                   const cartesian::Direction<EquatorialTrueOfDate> &b) {
  return std::sqrt(std::pow(a.x - b.x, 2) + std::pow(a.y - b.y, 2) +
                   std::pow(a.z - b.z, 2));
}

} // namespace

int main() {
  const JulianDate jd(2458850.0);
  const cartesian::Direction<ICRS> icrs(0.6, -0.3, 0.74);

  std::puts("=== Nutation Model Selection ===\n");
  std::printf("Epoch (TT): %.1f\n", jd.value());
  std::printf("Input ICRS direction: (%.6f, %.6f, %.6f)\n\n", icrs.x, icrs.y,
              icrs.z);

  const auto default_true = icrs.to_frame<EquatorialTrueOfDate>(jd);

  AstroContext ctx;
  const auto fast_ctx = ctx.with_model<Iau2000B>();
  const auto precession_only_ctx = ctx.with_model<Iau2006>();

  const auto fast_true =
      icrs.to_frame_with<EquatorialTrueOfDate>(jd, fast_ctx);
  const auto precession_only_true =
      icrs.to_frame_with<EquatorialTrueOfDate>(jd, precession_only_ctx);

  std::puts("1. Default transform (Iau2006A)");
  std::printf("   TOD direction = (%.12f, %.12f, %.12f)\n", default_true.x,
              default_true.y, default_true.z);

  std::puts("\n2. Custom transform with Iau2000B");
  std::printf("   TOD direction = (%.12f, %.12f, %.12f)\n", fast_true.x,
              fast_true.y, fast_true.z);
  std::printf("   Chord delta vs default = %.3e\n",
              chord_delta(default_true, fast_true));

  std::puts("\n3. Custom transform with Iau2006 (precession only)");
  std::printf("   TOD direction = (%.12f, %.12f, %.12f)\n",
              precession_only_true.x, precession_only_true.y,
              precession_only_true.z);
  std::printf("   Chord delta vs default = %.3e\n",
              chord_delta(default_true, precession_only_true));

  std::puts("\nPattern to copy:");
  std::puts("  AstroContext ctx;");
  std::puts("  auto custom = ctx.with_model<Iau2000B>();");
  std::puts(
      "  auto tod = icrs.to_frame_with<EquatorialTrueOfDate>(jd, custom);");
  return 0;
}
