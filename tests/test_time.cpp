#include <gtest/gtest.h>
#include <siderust/siderust.hpp>

using namespace siderust;

// ============================================================================
// JulianDate
// ============================================================================

TEST(Time, JulianDateJ2000) {
    auto jd = JulianDate::J2000();
    EXPECT_DOUBLE_EQ(jd.value(), 2451545.0);
}

TEST(Time, JulianDateFromUtc) {
    // UTC noon 2000-01-01 differs from J2000 (TT) by ~64s leap seconds
    auto jd = JulianDate::from_utc({2000, 1, 1, 12, 0, 0});
    EXPECT_NEAR(jd.value(), 2451545.0, 0.001);
}

TEST(Time, JulianDateRoundtripUtc) {
    UTC original(2026, 7, 15, 22, 0, 0);
    auto jd  = JulianDate::from_utc(original);
    auto utc = jd.to_utc();
    EXPECT_EQ(utc.year, 2026);
    EXPECT_EQ(utc.month, 7);
    EXPECT_EQ(utc.day, 15);
    // Hour may differ slightly due to TT/UTC offset
    EXPECT_NEAR(utc.hour, 22, 1);
}

TEST(Time, JulianDateArithmetic) {
    auto jd1 = JulianDate(2451545.0);
    auto jd2 = jd1 + 365.25;
    EXPECT_NEAR(jd2 - jd1, 365.25, 1e-10);
}

TEST(Time, JulianCenturies) {
    auto jd = JulianDate::J2000();
    EXPECT_NEAR(jd.julian_centuries(), 0.0, 1e-10);
}

// ============================================================================
// MJD
// ============================================================================

TEST(Time, MjdFromJd) {
    auto jd  = JulianDate::J2000();
    auto mjd = MJD::from_jd(jd);
    EXPECT_NEAR(mjd.value(), jd.to_mjd(), 1e-10);
}

TEST(Time, MjdRoundtrip) {
    auto mjd1 = MJD(60200.0);
    auto jd   = mjd1.to_jd();
    auto mjd2 = MJD::from_jd(jd);
    EXPECT_NEAR(mjd1.value(), mjd2.value(), 1e-10);
}

// ============================================================================
// Period
// ============================================================================

TEST(Time, PeriodDuration) {
    Period p(60200.0, 60201.0);
    EXPECT_NEAR(p.duration_days(), 1.0, 1e-10);
}

TEST(Time, PeriodIntersection) {
    Period a(60200.0, 60202.0);
    Period b(60201.0, 60203.0);
    auto c = a.intersection(b);
    EXPECT_NEAR(c.start_mjd(), 60201.0, 1e-10);
    EXPECT_NEAR(c.end_mjd(), 60202.0, 1e-10);
}

TEST(Time, PeriodNoIntersection) {
    Period a(60200.0, 60201.0);
    Period b(60202.0, 60203.0);
    EXPECT_THROW(a.intersection(b), SiderustException);
}

TEST(Time, PeriodInvalidThrows) {
    EXPECT_THROW(Period(60203.0, 60200.0), InvalidPeriodError);
}
