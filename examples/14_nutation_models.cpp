// SPDX-License-Identifier: AGPL-3.0-or-later
// Copyright (C) 2026 Vallés Puig, Ramon

/// @file 14_nutation_models.cpp
/// @brief Nutation model selection example for the C++ bindings.

#include <cmath>
#include <iostream>
#include <siderust/siderust.hpp>

using namespace siderust;
using namespace siderust::frames;

namespace {

double chord_delta(const cartesian::Direction<EquatorialTrueOfDate> &a,
                   const cartesian::Direction<EquatorialTrueOfDate> &b) {
  return std::sqrt(std::pow(a.x - b.x, 2) + std::pow(a.y - b.y, 2) + std::pow(a.z - b.z, 2));
}

} // namespace

int main() {
  const Time<TT, JD> jd(2458850.0);
  const cartesian::Direction<ICRS> icrs(0.6, -0.3, 0.74);

  std::cout << "=== Nutation Model Selection ===\n\n";
  std::cout << "Epoch (TT): " << jd << '\n';
  std::cout << "Input ICRS direction: " << icrs << "\n\n";

  const auto default_true = icrs.to_frame<EquatorialTrueOfDate>(jd);

  AstroContext ctx;
  const auto fast_ctx = ctx.with_model<Iau2000B>();
  const auto precession_only_ctx = ctx.with_model<Iau2006>();

  const auto fast_true = icrs.to_frame_with<EquatorialTrueOfDate>(jd, fast_ctx);
  const auto precession_only_true =
      icrs.to_frame_with<EquatorialTrueOfDate>(jd, precession_only_ctx);

  std::cout << std::scientific << std::setprecision(3);
  std::cout << "1. Default transform (Iau2006A)\n";
  std::cout << "   TOD direction = " << default_true << '\n';

  std::cout << "\n2. Custom transform with Iau2000B\n";
  std::cout << "   TOD direction = " << fast_true << '\n';
  std::cout << "   Chord delta vs default = " << chord_delta(default_true, fast_true) << '\n';

  std::cout << "\n3. Custom transform with Iau2006 (precession only)\n";
  std::cout << "   TOD direction = " << precession_only_true << '\n';
  std::cout << "   Chord delta vs default = " << chord_delta(default_true, precession_only_true)
            << "\n\n";

  std::cout << std::fixed;
  std::cout << "Pattern to copy:\n";
  std::cout << "  AstroContext ctx;\n";
  std::cout << "  auto custom = ctx.with_model<Iau2000B>();\n";
  std::cout << "  auto tod = icrs.to_frame_with<EquatorialTrueOfDate>(jd, custom);\n";
  return 0;
}
