// SPDX-License-Identifier: AGPL-3.0-or-later
// Copyright (C) 2026 Vallés Puig, Ramon

/// @file 10_time_periods.cpp
/// @brief Time Scales, Formats, and Period Conversions Example.
///
/// Demonstrates `tempoch` (re-exported via `siderust::time`):
/// - Viewing the same absolute instant in every supported time scale
/// - Using the common type aliases (`JulianDate`, `MJD`, `TDB`, `TT`, …)
/// - Converting `Period<S>` between scales
/// - ΔT = TT − UT1

#include <siderust/siderust.hpp>

#include <cstdio>
#include <iomanip>
#include <iostream>

using namespace siderust;

// ── Helper: print a single scale value + JD round-trip drift ────────────────

template <typename S>
void print_scale(const char *label,
                 const tempoch::Time<S> &time,
                 const JulianDate &reference_jd) {
    // Convert back to JD so we can measure round-trip drift.
    auto jd_back = time.template to<tempoch::JDScale>();
    double drift_days = jd_back.value() - reference_jd.value();
    double drift_s = drift_days * 86400.0;
    std::cout << "   " << std::left << std::setw(8) << label
              << " value = " << std::right << std::setw(16) << std::fixed
              << std::setprecision(9) << time
              << "  | JD roundtrip drift = " << std::scientific
              << std::setprecision(3) << drift_s << " s" << std::endl;
}

// ── Helper: print a Period in a given scale ─────────────────────────────────

template <typename T>
void print_period(const char *label, const tempoch::Period<T> &period) {
    auto dur = period.template duration<qtty::Day>();
    std::cout << "   " << std::left << std::setw(8) << label << std::right
              << " [" << std::fixed << std::setprecision(9)
              << period.start() << ", " << period.end() << "]"
              << "  \u0394 = " << dur << std::endl;
}

int main() {
    std::cout << "Time Scales, Formats, and Period Conversions\n"
              << "============================================\n\n";

    // ── Reference instant: 2000-01-01T12:00:00 UTC ──────────────────────────

    auto jd = JulianDate::from_utc({2000, 1, 1, 12, 0, 0});

    // Convert to every supported scale.
    auto jde    = jd.to<tempoch::JDEScale>();
    auto mjd    = jd.to<tempoch::MJDScale>();
    auto tdb    = jd.to<tempoch::TDBScale>();
    auto tt     = jd.to<tempoch::TTScale>();
    auto tai    = jd.to<tempoch::TAIScale>();
    auto tcg    = jd.to<tempoch::TCGScale>();
    auto tcb    = jd.to<tempoch::TCBScale>();
    auto gps    = jd.to<tempoch::GPSScale>();
    auto unix_t = jd.to<tempoch::UnixTimeScale>();
    auto ut     = jd.to<tempoch::UTScale>();

    auto utc_civil = jd.to_utc();

    std::cout << "Reference UTC instant: "
              << utc_civil.year << "-"
              << std::setfill('0') << std::setw(2) << (int)utc_civil.month << "-"
              << std::setw(2) << (int)utc_civil.day << "T"
              << std::setw(2) << (int)utc_civil.hour << ":"
              << std::setw(2) << (int)utc_civil.minute << ":"
              << std::setw(2) << (int)utc_civil.second
              << std::setfill(' ')
              << "\n\n";

    // ── 1) Each supported time scale for the same instant ───────────────────

    std::cout << "1) Each supported time scale for the same instant:\n";
    print_scale("JD",   jd,     jd);
    print_scale("JDE",  jde,    jd);
    print_scale("MJD",  mjd,    jd);
    print_scale("TDB",  tdb,    jd);
    print_scale("TT",   tt,     jd);
    print_scale("TAI",  tai,    jd);
    print_scale("TCG",  tcg,    jd);
    print_scale("TCB",  tcb,    jd);
    print_scale("GPS",  gps,    jd);
    print_scale("Unix", unix_t, jd);
    print_scale("UT",   ut,     jd);

    auto delta_t = ut.delta_t();
    std::cout << "   " << std::left << std::setw(8) << "UT" << std::right
              << " delta_t = " << std::fixed << std::setprecision(3)
              << delta_t << " (TT - UT)\n" << std::endl;

    // ── 2) Time formats / aliases ───────────────────────────────────────────

    std::cout << "2) Time formats / aliases:\n";
    std::cout << "   JulianDate alias:         " << std::fixed
              << std::setprecision(9) << jd << std::endl;
    std::cout << "   JDE (JulianEphemeris):    " << jde << std::endl;
    std::cout << "   ModifiedJulianDate alias: " << mjd << std::endl;
    std::cout << "   UniversalTime alias:      " << ut << std::endl;

    auto utc_rt = jd.to_utc();
    std::cout << "   UTC roundtrip from JD:    "
              << utc_rt.year << "-"
              << std::setfill('0') << std::setw(2) << (int)utc_rt.month << "-"
              << std::setw(2) << (int)utc_rt.day << "T"
              << std::setw(2) << (int)utc_rt.hour << ":"
              << std::setw(2) << (int)utc_rt.minute << ":"
              << std::setw(2) << (int)utc_rt.second
              << std::setfill(' ')
              << "\n\n";

    // ── 3) Period representations and conversions ───────────────────────────

    std::cout << "3) Period representations and conversions:\n";

    auto jd_end = JulianDate(jd.value() + 0.5);
    tempoch::Period<JulianDate> period_jd(jd, jd_end);

    // Convert period endpoints to each scale via .to<>().
    auto mk_period = [](auto start_jd, auto end_jd, auto /*tag*/) {
        using S = std::decay_t<decltype(start_jd.template to<tempoch::JDScale>())>;
        (void)sizeof(S);  // suppress unused
        auto s = start_jd;
        auto e = end_jd;
        return tempoch::Period(s, e);
    };
    (void)mk_period;

    tempoch::Period period_jde(period_jd.start().to<tempoch::JDEScale>(),
                               period_jd.end().to<tempoch::JDEScale>());
    tempoch::Period period_mjd(period_jd.start().to<tempoch::MJDScale>(),
                               period_jd.end().to<tempoch::MJDScale>());
    tempoch::Period period_tdb(period_jd.start().to<tempoch::TDBScale>(),
                               period_jd.end().to<tempoch::TDBScale>());
    tempoch::Period period_tt(period_jd.start().to<tempoch::TTScale>(),
                              period_jd.end().to<tempoch::TTScale>());
    tempoch::Period period_tai(period_jd.start().to<tempoch::TAIScale>(),
                               period_jd.end().to<tempoch::TAIScale>());
    tempoch::Period period_tcg(period_jd.start().to<tempoch::TCGScale>(),
                               period_jd.end().to<tempoch::TCGScale>());
    tempoch::Period period_tcb(period_jd.start().to<tempoch::TCBScale>(),
                               period_jd.end().to<tempoch::TCBScale>());
    tempoch::Period period_gps(period_jd.start().to<tempoch::GPSScale>(),
                               period_jd.end().to<tempoch::GPSScale>());
    tempoch::Period period_unix(period_jd.start().to<tempoch::UnixTimeScale>(),
                                period_jd.end().to<tempoch::UnixTimeScale>());
    tempoch::Period period_ut(period_jd.start().to<tempoch::UTScale>(),
                              period_jd.end().to<tempoch::UTScale>());

    print_period("JD",   period_jd);
    print_period("JDE",  period_jde);
    print_period("MJD",  period_mjd);
    print_period("TDB",  period_tdb);
    print_period("TT",   period_tt);
    print_period("TAI",  period_tai);
    print_period("TCG",  period_tcg);
    print_period("TCB",  period_tcb);
    print_period("GPS",  period_gps);
    print_period("Unix", period_unix);
    print_period("UT",   period_ut);

    // UTC period via CivilTime
    auto utc_start = period_jd.start().to_utc();
    auto utc_end   = period_jd.end().to_utc();
    tempoch::Period<tempoch::CivilTime> period_utc(utc_start, utc_end);
    auto utc_dur = period_utc.duration<qtty::Day>();
    std::cout << "   UTC      [" << utc_start << " -> " << utc_end
              << "]  Δ = " << utc_dur << "\n\n";

    // ── 4) UTC ↔ typed period conversions ───────────────────────────────────

    std::cout << "4) UtcPeriod / CivilTime period conversions back to typed periods:\n";

    auto utc_ref     = tempoch::CivilTime{2000, 1, 1, 12, 0, 0};
    auto utc_ref_end = tempoch::CivilTime{2000, 1, 1, 18, 0, 0};
    tempoch::Period<tempoch::CivilTime> utc_window(utc_ref, utc_ref_end);

    std::cout << "   UTC      [" << utc_ref << " -> " << utc_ref_end
              << "]  Δ = " << utc_window.duration<qtty::Day>() << "\n";

    // Convert UTC endpoints → JD → print
    auto from_utc_jd_start = JulianDate::from_utc(utc_ref);
    auto from_utc_jd_end   = JulianDate::from_utc(utc_ref_end);
    tempoch::Period<JulianDate> from_utc_jd(from_utc_jd_start, from_utc_jd_end);
    print_period("JD", from_utc_jd);

    // → MJD
    tempoch::Period from_utc_mjd(from_utc_jd_start.to<tempoch::MJDScale>(),
                                  from_utc_jd_end.to<tempoch::MJDScale>());
    print_period("MJD", from_utc_mjd);

    // → UT
    tempoch::Period from_utc_ut(from_utc_jd_start.to<tempoch::UTScale>(),
                                 from_utc_jd_end.to<tempoch::UTScale>());
    print_period("UT", from_utc_ut);

    // → UnixTime
    tempoch::Period from_utc_unix(from_utc_jd_start.to<tempoch::UnixTimeScale>(),
                                   from_utc_jd_end.to<tempoch::UnixTimeScale>());
    print_period("Unix", from_utc_unix);

    // Roundtrip: MJD → CivilTime
    auto utc_rt_start = from_utc_mjd.start().to_utc();
    auto utc_rt_end   = from_utc_mjd.end().to_utc();
    std::cout << "   UTC<-MJD [" << utc_rt_start << " -> " << utc_rt_end << "]\n";

    return 0;
}
