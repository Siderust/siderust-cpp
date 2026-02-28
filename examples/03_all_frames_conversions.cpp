// SPDX-License-Identifier: AGPL-3.0-or-later
// Copyright (C) 2026 Vallés Puig, Ramon

/// Example: all currently supported frame conversions.
///
/// Demonstrates every direct frame-rotation pair, plus identity rotations.
///
/// Build & run: cmake --build build-local --target 03_all_frame_conversions_example
///              ./build-local/03_all_frame_conversions_example

#include <siderust/siderust.hpp>

#include <iomanip>
#include <iostream>

using namespace siderust;
using namespace siderust::frames;
using namespace siderust::centers;

using C = Barycentric;
using U = qtty::AstronomicalUnit;

/// Show a frame conversion from F1→F2, round-trip F1→F2→F1, and the error.
template <typename F1, typename F2>
void show_frame_conversion(const JulianDate &jd,
                           const cartesian::Position<C, F1, U> &src) {
    auto out  = src.template to_frame<F2>(jd);
    auto back = out.template to_frame<F1>(jd);
    auto err  = (src - back).magnitude();

    std::cout << std::left << std::setw(24) << FrameTraits<F1>::name()
              << " -> " << std::setw(24) << FrameTraits<F2>::name()
              << " out=(" << std::showpos << std::setprecision(9) << out
              << std::noshowpos << ")  roundtrip=" << std::scientific
              << std::setprecision(3) << err << std::fixed
              << std::endl;
}

int main() {
    JulianDate jd(2460000.5);
    std::cout << std::fixed;
    std::cout << "Frame conversion demo at JD(TT) = " << std::setprecision(1)
              << jd << std::endl;

    cartesian::Position<C, ICRS, U> p_icrs(0.30, -0.70, 0.64);
    auto p_icrf     = p_icrs.to_frame<ICRF>(jd);
    auto p_ecl      = p_icrs.to_frame<EclipticMeanJ2000>(jd);
    auto p_eq_j2000 = p_icrs.to_frame<EquatorialMeanJ2000>(jd);
    auto p_eq_mod   = p_icrs.to_frame<EquatorialMeanOfDate>(jd);
    auto p_eq_tod   = p_icrs.to_frame<EquatorialTrueOfDate>(jd);

    // Identity conversions
    show_frame_conversion<ICRS, ICRS>(jd, p_icrs);
    show_frame_conversion<ICRF, ICRF>(jd, p_icrf);
    show_frame_conversion<EclipticMeanJ2000, EclipticMeanJ2000>(jd, p_ecl);
    show_frame_conversion<EquatorialMeanJ2000, EquatorialMeanJ2000>(jd, p_eq_j2000);
    show_frame_conversion<EquatorialMeanOfDate, EquatorialMeanOfDate>(jd, p_eq_mod);
    show_frame_conversion<EquatorialTrueOfDate, EquatorialTrueOfDate>(jd, p_eq_tod);

    // All direct non-identity provider pairs
    show_frame_conversion<ICRS, EclipticMeanJ2000>(jd, p_icrs);
    show_frame_conversion<EclipticMeanJ2000, ICRS>(jd, p_ecl);
    show_frame_conversion<ICRS, EquatorialMeanJ2000>(jd, p_icrs);
    show_frame_conversion<EquatorialMeanJ2000, ICRS>(jd, p_eq_j2000);
    show_frame_conversion<EquatorialMeanJ2000, EclipticMeanJ2000>(jd, p_eq_j2000);
    show_frame_conversion<EclipticMeanJ2000, EquatorialMeanJ2000>(jd, p_ecl);
    show_frame_conversion<EquatorialMeanJ2000, EquatorialMeanOfDate>(jd, p_eq_j2000);
    show_frame_conversion<EquatorialMeanOfDate, EquatorialMeanJ2000>(jd, p_eq_mod);
    show_frame_conversion<EquatorialMeanOfDate, EquatorialTrueOfDate>(jd, p_eq_mod);
    show_frame_conversion<EquatorialTrueOfDate, EquatorialMeanOfDate>(jd, p_eq_tod);
    show_frame_conversion<EquatorialMeanJ2000, EquatorialTrueOfDate>(jd, p_eq_j2000);
    show_frame_conversion<EquatorialTrueOfDate, EquatorialMeanJ2000>(jd, p_eq_tod);
    show_frame_conversion<ICRS, EquatorialMeanOfDate>(jd, p_icrs);
    show_frame_conversion<EquatorialMeanOfDate, ICRS>(jd, p_eq_mod);
    show_frame_conversion<ICRS, EquatorialTrueOfDate>(jd, p_icrs);
    show_frame_conversion<EquatorialTrueOfDate, ICRS>(jd, p_eq_tod);
    show_frame_conversion<ICRF, ICRS>(jd, p_icrf);
    show_frame_conversion<ICRS, ICRF>(jd, p_icrs);
    show_frame_conversion<ICRF, EquatorialMeanJ2000>(jd, p_icrf);
    show_frame_conversion<EquatorialMeanJ2000, ICRF>(jd, p_eq_j2000);
    show_frame_conversion<ICRF, EclipticMeanJ2000>(jd, p_icrf);
    show_frame_conversion<EclipticMeanJ2000, ICRF>(jd, p_ecl);
    show_frame_conversion<ICRF, EquatorialMeanOfDate>(jd, p_icrf);
    show_frame_conversion<EquatorialMeanOfDate, ICRF>(jd, p_eq_mod);
    show_frame_conversion<ICRF, EquatorialTrueOfDate>(jd, p_icrf);
    show_frame_conversion<EquatorialTrueOfDate, ICRF>(jd, p_eq_tod);

    return 0;
}
