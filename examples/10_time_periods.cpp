// SPDX-License-Identifier: AGPL-3.0-or-later
// Copyright (C) 2026 Vallés Puig, Ramon

/// @file 10_time_periods.cpp
/// @brief Ergonomic TT-default time workflow example.
///
/// Demonstrates:
/// - `Time<TT, JD>::from_utc(...)` and `to_utc()`
/// - `Time<TT, MJD>::from_jd(...)`
/// - direct date arithmetic including `+=`
/// - the explicit-scale API for advanced conversions

#include <siderust/siderust.hpp>

#include <iomanip>
#include <iostream>

using namespace siderust;
using UTCJD = Time<UTC, JD>;
template <typename T> static CivilTime to_utc_civil(const T &time) {
  return time.template to<scale::UTC>().to_civil();
}

int main() {
  auto ctx = TimeContext::with_builtin_eop();
  CivilTime civil{2000, 1, 1, 12, 0, 0};
  auto jd_tt = Time<TT, JD>::from_utc(civil);
  auto mjd_tt = Time<TT, MJD>::from_jd(jd_tt);
  auto utc = TimeAxis<TT>::from_encoded(jd_tt).to<UTC>();
  auto tt = utc.to<TT>();
  auto tai = tt.to<scale::TAI>();
  auto tdb = tt.to<TDB>();
  auto ut1 = tt.to_with<UT1>(ctx);
  auto jd_utc = utc.to<format::JD>();
  auto unix_time = utc.to<format::Unix>();
  auto gps_time = tai.to<format::GPS>();

  std::cout << "Time Scales, Formats, and Period<TT, MJD> Conversions\n"
            << "============================================\n\n";
  std::cout << "Civil UTC : " << civil << "\n";
  std::cout << "UTC axis  : " << utc << "\n";
  std::cout << "TT axis   : " << tt << "\n\n";

  std::cout << "Same instant, different encodings:\n";
  std::cout << "  JD(TT)    = " << std::fixed << std::setprecision(9) << jd_tt << "\n";
  std::cout << "  JD(UTC)   = " << jd_utc << "\n";
  std::cout << "  Time<TT, MJD>(TT)   = " << mjd_tt << "\n";
  std::cout << "  Unix(UTC) = " << std::setprecision(3) << unix_time.value() << " s\n";
  std::cout << "  GPS(TAI)  = " << gps_time.value() << " s\n\n";

  std::cout << "Same instant, different scales:\n";
  std::cout << "  TT  JD  = " << std::setprecision(9) << tt.to<format::JD>() << "\n";
  std::cout << "  TAI JD  = " << tai.to<format::JD>() << "\n";
  std::cout << "  TDB JD  = " << tdb.to<format::JD>() << "\n";
  std::cout << "  UT1 JD  = " << ut1.to<format::JD>() << "\n";

  constexpr double SPD = 86'400.0;
  double tt_minus_tai = (tt.to<format::JD>().value() - tai.to<format::JD>().value()) * SPD;
  double tt_minus_ut1 = (tt.to<format::JD>().value() - ut1.to<format::JD>().value()) * SPD;
  std::cout << "  TT-TAI  = " << std::setprecision(6) << tt_minus_tai << " s\n";
  std::cout << "  TT-UT1  = " << tt_minus_ut1 << " s\n\n";

  auto jd_end = jd_tt;
  jd_end += qtty::Hour(12.0);
  tempoch::Period<Time<TT, JD>> jd_window(jd_tt, jd_end);
  Period<TT, MJD> mjd_window(Time<TT, MJD>::from_jd(jd_window.start()),
                             Time<TT, MJD>::from_jd(jd_window.end()));
  tempoch::Period<CivilTime> civil_window(to_utc_civil(jd_window.start()),
                                          to_utc_civil(jd_window.end()));

  std::cout << "Periods over the same half-day window:\n";
  std::cout << "  JD(TT)   " << jd_window << "  duration=" << jd_window.duration<qtty::Hour>()
            << "\n";
  std::cout << "  Time<TT, MJD>(TT)  " << mjd_window
            << "  duration=" << mjd_window.duration<qtty::Hour>() << "\n";
  std::cout << "  Civil    " << civil_window << "  duration=" << civil_window.duration<qtty::Hour>()
            << "\n";

  return 0;
}
