// SPDX-License-Identifier: AGPL-3.0-or-later
// Copyright (C) 2026 Vallés Puig, Ramon

/**
 * @file 17_sgp4_from_tle.cpp
 * @brief Worked example: propagate a TLE through SGP4 to a few epochs.
 *
 * C++17 mirror of the Rust example `17_sgp4_from_tle.rs`.
 *
 * Loads the canonical Vallado SGP4 demo satellite (NORAD 5), builds an SGP4
 * propagator (WGS-72), and prints the TEME state at the TLE epoch and at three
 * later epochs (360, 720, 1080 minutes).
 */

#include <siderust/sgp4.hpp>

#include <cstdio>

static constexpr const char *L1 =
    "1 00005U 58002B   00179.78495062  .00000023  00000-0  28098-4 0  4753";
static constexpr const char *L2 =
    "2 00005  34.2682 348.7242 1859667 331.7664  19.3264 10.82419157413667";

int main() {
    auto tle  = siderust::tle::Tle::parse(L1, L2);
    auto prop = siderust::sgp4::Propagator(tle);

    std::printf("Satellite     : NORAD %u\n", tle.norad_id());
    std::printf("Gravity model : %d (0=WGS-72, 1=WGS-72/IAU, 2=WGS-84)\n",
                prop.gravity_model());
    std::printf("TLE epoch (JD): %.10f\n\n", prop.epoch_jd_utc());

    std::printf("%12s  %14s  %14s  %14s  %10s  %10s  %10s\n",
                "t [min]", "x [km]", "y [km]", "z [km]",
                "vx [km/s]", "vy [km/s]", "vz [km/s]");

    for (double dt_min : {0.0, 360.0, 720.0, 1080.0}) {
        double jd = prop.epoch_jd_utc() + dt_min / 1440.0;
        auto   s  = prop.propagate_at(jd);
        std::printf("%12.1f  %14.6f  %14.6f  %14.6f  %10.6f  %10.6f  %10.6f\n",
                    dt_min,
                    s.pos_km[0], s.pos_km[1], s.pos_km[2],
                    s.vel_kms[0], s.vel_kms[1], s.vel_kms[2]);
    }

    return 0;
}
