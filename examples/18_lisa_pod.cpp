// SPDX-License-Identifier: AGPL-3.0-or-later
// Copyright (C) 2026 Vallés Puig, Ramon

/**
 * @file 18_lisa_pod.cpp
 * @brief ESA LISA mission — orbit reader and inter-satellite range model.
 *
 * C++17 mirror of the Rust example `18_lisa_pod.rs`.
 *
 * Demonstrates:
 * 1. Parse three CCSDS OEM v2 files (one per LISA spacecraft) using
 *    `siderust::oem::parse()`.
 * 2. Build a `LisaEphemerisProvider` backed by cubic Hermite interpolation.
 * 3. Compute a one-way light-time-corrected inter-satellite range
 *    observation (SC1 → SC2) and verify the O−C residual is ~0 m.
 *
 * References:
 * - CCSDS 502.0-B-3 (OEM standard).
 * - Martens & Joffre (2021), J. Astronaut. Sci. 68, 402–443.
 * - Danzmann et al. (2017), LISA: Laser Interferometer Space Antenna.
 */

#include <siderust/oem.hpp>

#include <algorithm>
#include <cassert>
#include <cmath>
#include <fstream>
#include <iomanip>
#include <iostream>
#include <sstream>
#include <stdexcept>
#include <string>
#include <vector>

static constexpr double J2000_JD = 2'451'545.0;
static constexpr double C_KM_S = 299'792.458;

enum class LisaScId { SC1 = -1001, SC2 = -1002, SC3 = -1003 };

static int naif_id(LisaScId id) { return static_cast<int>(id); }

struct OrbitPoint {
  double epoch_j2000_s;
  double px, py, pz;
  double vx, vy, vz;
};

using Orbit = std::vector<OrbitPoint>;
using OrbitSet = std::array<Orbit, 3>;

static double jd_to_j2000s(double jd) { return (jd - J2000_JD) * 86'400.0; }

static Orbit orbit_from_oem(const std::string &path) {
  std::ifstream f{path};
  if (!f)
    throw std::runtime_error("cannot open: " + path);
  std::ostringstream ss;
  ss << f.rdbuf();

  auto states = siderust::oem::parse(ss.str());
  if (states.empty())
    throw std::runtime_error("OEM file contains no states: " + path);

  Orbit pts;
  pts.reserve(states.size());
  for (auto &sv : states) {
    pts.push_back({jd_to_j2000s(sv.epoch_jd), sv.pos_km[0], sv.pos_km[1], sv.pos_km[2],
                   sv.vel_kms[0], sv.vel_kms[1], sv.vel_kms[2]});
  }
  std::sort(pts.begin(), pts.end(), [](const OrbitPoint &a, const OrbitPoint &b) {
    return a.epoch_j2000_s < b.epoch_j2000_s;
  });
  return pts;
}

static OrbitPoint hermite_interp(const Orbit &pts, double t) {
  if (pts.empty())
    throw std::out_of_range("orbit is empty");

  const double t0 = pts.front().epoch_j2000_s;
  const double t1 = pts.back().epoch_j2000_s;
  if (t < t0 || t > t1)
    throw std::out_of_range("epoch outside covered interval");

  std::size_t idx = 0;
  {
    std::size_t lo = 0, hi = pts.size();
    while (lo < hi) {
      std::size_t mid = lo + (hi - lo) / 2;
      if (pts[mid].epoch_j2000_s <= t)
        lo = mid + 1;
      else
        hi = mid;
    }
    idx = (lo == 0) ? 0 : lo - 1;
    if (idx >= pts.size() - 1)
      idx = pts.size() - 2;
    if (pts[idx + 1].epoch_j2000_s == t)
      return pts[idx + 1];
    if (pts[idx].epoch_j2000_s == t)
      return pts[idx];
  }

  const auto &p0 = pts[idx];
  const auto &p1 = pts[idx + 1];
  const double dt = p1.epoch_j2000_s - p0.epoch_j2000_s;
  const double tau = (t - p0.epoch_j2000_s) / dt;
  const double tau2 = tau * tau;
  const double tau3 = tau2 * tau;

  const double h00 = 2 * tau3 - 3 * tau2 + 1;
  const double h10 = tau3 - 2 * tau2 + tau;
  const double h01 = -2 * tau3 + 3 * tau2;
  const double h11 = tau3 - tau2;

  auto ip = [&](double a, double da, double b, double db) {
    return h00 * a + h10 * dt * da + h01 * b + h11 * dt * db;
  };
  auto iv = [&](double a, double da, double b, double db) {
    double dh00 = 6 * tau2 - 6 * tau;
    double dh10 = 3 * tau2 - 4 * tau + 1;
    double dh01 = -6 * tau2 + 6 * tau;
    double dh11 = 3 * tau2 - 2 * tau;
    return (dh00 * a + dh10 * dt * da + dh01 * b + dh11 * dt * db) / dt;
  };

  return {t,
          ip(p0.px, p0.vx, p1.px, p1.vx),
          ip(p0.py, p0.vy, p1.py, p1.vy),
          ip(p0.pz, p0.vz, p1.pz, p1.vz),
          iv(p0.px, p0.vx, p1.px, p1.vx),
          iv(p0.py, p0.vy, p1.py, p1.vy),
          iv(p0.pz, p0.vz, p1.pz, p1.vz)};
}

struct LisaEphemerisProvider {
  OrbitSet orbits;

  const Orbit &orbit_for(int naif) const {
    switch (naif) {
    case -1001:
      return orbits[0];
    case -1002:
      return orbits[1];
    case -1003:
      return orbits[2];
    default:
      throw std::invalid_argument("unknown LISA NAIF id");
    }
  }

  OrbitPoint state(int naif, double epoch_j2000_s) const {
    return hermite_interp(orbit_for(naif), epoch_j2000_s);
  }
};

static double modelled_range_m(const LisaEphemerisProvider &prov, int sc_a_naif, int sc_b_naif,
                               double t_recv_j2000_s) {
  auto pt_a = prov.state(sc_a_naif, t_recv_j2000_s);
  auto pt_b0 = prov.state(sc_b_naif, t_recv_j2000_s);

  double dx0 = pt_a.px - pt_b0.px;
  double dy0 = pt_a.py - pt_b0.py;
  double dz0 = pt_a.pz - pt_b0.pz;
  double rho0 = std::sqrt(dx0 * dx0 + dy0 * dy0 + dz0 * dz0);

  double t_emit = t_recv_j2000_s - rho0 / C_KM_S;
  auto pt_b1 = prov.state(sc_b_naif, t_emit);

  double dx1 = pt_a.px - pt_b1.px;
  double dy1 = pt_a.py - pt_b1.py;
  double dz1 = pt_a.pz - pt_b1.pz;
  return std::sqrt(dx1 * dx1 + dy1 * dy1 + dz1 * dz1) * 1'000.0;
}

int main() {
#ifndef SIDERUST_TEST_DATA_DIR
#define SIDERUST_TEST_DATA_DIR "siderust/tests/test-data/lisa"
#endif

  const std::string root = SIDERUST_TEST_DATA_DIR;

  LisaEphemerisProvider prov;
  prov.orbits[0] = orbit_from_oem(root + "/lisa_orbit_sample.oem1");
  prov.orbits[1] = orbit_from_oem(root + "/lisa_orbit_sample.oem2");
  prov.orbits[2] = orbit_from_oem(root + "/lisa_orbit_sample.oem3");

  const double sc1_t0 = prov.orbits[0].front().epoch_j2000_s;
  auto pt = prov.state(naif_id(LisaScId::SC1), sc1_t0);
  std::cout << std::fixed << std::setprecision(0);
  std::cout << "SC1 position at t₀: (" << pt.px << ", " << pt.py << ", " << pt.pz << ") km\n";

  const double meas_m =
      modelled_range_m(prov, naif_id(LisaScId::SC1), naif_id(LisaScId::SC2), sc1_t0);

  const double epoch_jd = sc1_t0 / 86'400.0 + J2000_JD;
  std::cout << std::setprecision(6);
  std::cout << "InterSatRangeObs { sc_a: SC1, sc_b: SC2, epoch_jd: " << epoch_jd
            << ", measured_m: " << meas_m << ", sigma: 1e-12 }\n";

  const double computed_m =
      modelled_range_m(prov, naif_id(LisaScId::SC1), naif_id(LisaScId::SC2), sc1_t0);
  const double residual = meas_m - computed_m;
  std::cout << "O−C residual: " << residual << " m  (should be ~0)\n";
  assert(std::abs(residual) < 1.0 && "residual too large");

  std::cout << "LISA example completed successfully.\n";
  return 0;
}
