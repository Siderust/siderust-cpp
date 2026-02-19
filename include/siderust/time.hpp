#pragma once

/**
 * @file time.hpp
 * @brief C++ wrappers for Julian Date, Modified Julian Date, UTC, and Period.
 *
 * Wraps the tempoch-ffi C layer with value-semantic, exception-safe C++ types.
 */

#include "ffi_core.hpp"
#include <vector>

namespace siderust {

// ============================================================================
// UTC
// ============================================================================

/**
 * @brief UTC date-time breakdown.
 */
struct UTC {
    int32_t  year;
    uint8_t  month;
    uint8_t  day;
    uint8_t  hour;
    uint8_t  minute;
    uint8_t  second;
    uint32_t nanosecond;

    UTC() : year(2000), month(1), day(1), hour(12), minute(0), second(0), nanosecond(0) {}

    UTC(int32_t y, uint8_t mo, uint8_t d,
        uint8_t h = 0, uint8_t mi = 0, uint8_t s = 0, uint32_t ns = 0)
        : year(y), month(mo), day(d), hour(h), minute(mi), second(s), nanosecond(ns) {}

    /// Convert to the C FFI struct.
    tempoch_utc_t to_c() const {
        return {year, month, day, hour, minute, second, nanosecond};
    }

    /// Create from the C FFI struct.
    static UTC from_c(const tempoch_utc_t& c) {
        return UTC(c.year, c.month, c.day, c.hour, c.minute, c.second, c.nanosecond);
    }
};

// ============================================================================
// JulianDate
// ============================================================================

/**
 * @brief Julian Date wrapper (value type).
 */
class JulianDate {
    double m_value;

public:
    constexpr explicit JulianDate(double v) : m_value(v) {}

    /// J2000.0 epoch (2451545.0).
    static JulianDate J2000() { return JulianDate(tempoch_jd_j2000()); }

    /// Create from a UTC date-time.
    static JulianDate from_utc(const UTC& utc) {
        double jd;
        auto c = utc.to_c();
        check_tempoch_status(tempoch_jd_from_utc(c, &jd), "JulianDate::from_utc");
        return JulianDate(jd);
    }

    /// Raw value.
    constexpr double value() const { return m_value; }

    /// Convert to MJD.
    double to_mjd() const { return tempoch_jd_to_mjd(m_value); }

    /// Convert to UTC.
    UTC to_utc() const {
        tempoch_utc_t out;
        check_tempoch_status(tempoch_jd_to_utc(m_value, &out), "JulianDate::to_utc");
        return UTC::from_c(out);
    }

    /// Difference in days (this – other).
    double operator-(const JulianDate& other) const {
        return tempoch_jd_difference(m_value, other.m_value);
    }

    /// Add days.
    JulianDate operator+(double days) const {
        return JulianDate(tempoch_jd_add_days(m_value, days));
    }

    /// Julian centuries since J2000.
    double julian_centuries() const {
        return tempoch_jd_julian_centuries(m_value);
    }

    bool operator==(const JulianDate& o) const { return m_value == o.m_value; }
    bool operator!=(const JulianDate& o) const { return m_value != o.m_value; }
    bool operator< (const JulianDate& o) const { return m_value <  o.m_value; }
    bool operator<=(const JulianDate& o) const { return m_value <= o.m_value; }
    bool operator> (const JulianDate& o) const { return m_value >  o.m_value; }
    bool operator>=(const JulianDate& o) const { return m_value >= o.m_value; }
};

// ============================================================================
// MJD (Modified Julian Date)
// ============================================================================

/**
 * @brief Modified Julian Date wrapper (value type).
 */
class MJD {
    double m_value;

public:
    constexpr MJD() : m_value(0.0) {}
    constexpr explicit MJD(double v) : m_value(v) {}

    /// Create from a UTC date-time.
    static MJD from_utc(const UTC& utc) {
        double mjd;
        auto c = utc.to_c();
        check_tempoch_status(tempoch_mjd_from_utc(c, &mjd), "MJD::from_utc");
        return MJD(mjd);
    }

    /// Create from a Julian Date.
    static MJD from_jd(const JulianDate& jd) {
        return MJD(tempoch_jd_to_mjd(jd.value()));
    }

    /// Raw value.
    constexpr double value() const { return m_value; }

    /// Convert to JD.
    JulianDate to_jd() const { return JulianDate(tempoch_mjd_to_jd(m_value)); }

    /// Convert to UTC.
    UTC to_utc() const {
        tempoch_utc_t out;
        check_tempoch_status(tempoch_mjd_to_utc(m_value, &out), "MJD::to_utc");
        return UTC::from_c(out);
    }

    /// Difference in days (this – other).
    double operator-(const MJD& other) const {
        return tempoch_mjd_difference(m_value, other.m_value);
    }

    /// Add days.
    MJD operator+(double days) const {
        return MJD(tempoch_mjd_add_days(m_value, days));
    }

    bool operator==(const MJD& o) const { return m_value == o.m_value; }
    bool operator!=(const MJD& o) const { return m_value != o.m_value; }
    bool operator< (const MJD& o) const { return m_value <  o.m_value; }
    bool operator<=(const MJD& o) const { return m_value <= o.m_value; }
    bool operator> (const MJD& o) const { return m_value >  o.m_value; }
    bool operator>=(const MJD& o) const { return m_value >= o.m_value; }
};

// ============================================================================
// Period
// ============================================================================

/**
 * @brief A time period [start, end] in MJD.
 */
class Period {
    tempoch_period_mjd_t m_inner;

public:
    Period(double start_mjd, double end_mjd) {
        check_tempoch_status(
            tempoch_period_mjd_new(start_mjd, end_mjd, &m_inner),
            "Period::Period"
        );
    }

    Period(const MJD& start, const MJD& end)
        : Period(start.value(), end.value()) {}

    /// Construct from the C struct (unchecked).
    static Period from_c(const tempoch_period_mjd_t& c) {
        Period p(0.0, 1.0); // dummy
        p.m_inner = c;
        return p;
    }

    double start_mjd() const { return m_inner.start_mjd; }
    double end_mjd()   const { return m_inner.end_mjd; }
    MJD    start()     const { return MJD(m_inner.start_mjd); }
    MJD    end()       const { return MJD(m_inner.end_mjd); }

    /// Duration in days.
    double duration_days() const {
        return tempoch_period_mjd_duration_days(m_inner);
    }

    /// Intersection with another period. Throws if no overlap.
    Period intersection(const Period& other) const {
        tempoch_period_mjd_t out;
        check_tempoch_status(
            tempoch_period_mjd_intersection(m_inner, other.m_inner, &out),
            "Period::intersection"
        );
        return from_c(out);
    }

    /// Access the underlying C struct.
    const tempoch_period_mjd_t& c_inner() const { return m_inner; }
};

} // namespace siderust
