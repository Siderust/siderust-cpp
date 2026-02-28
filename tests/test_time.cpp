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
  auto jd = JulianDate::from_utc(original);
  auto utc = jd.to_utc();
  EXPECT_EQ(utc.year, 2026);
  EXPECT_EQ(utc.month, 7);
  EXPECT_EQ(utc.day, 15);
  // Hour may differ slightly due to TT/UTC offset
  EXPECT_NEAR(utc.hour, 22, 1);
}

TEST(Time, JulianDateArithmetic) {
  auto jd1 = JulianDate(2451545.0);
  auto jd2 = jd1 + qtty::Day(365.25);
  EXPECT_NEAR((jd2 - jd1).value(), 365.25, 1e-10);
}

TEST(Time, JulianCenturies) {
  auto jd = JulianDate::J2000();
  EXPECT_NEAR(jd.julian_centuries(), 0.0, 1e-10);
}

// ============================================================================
// MJD
// ============================================================================

TEST(Time, MjdFromJd) {
  auto jd = JulianDate::J2000();
  auto mjd = MJD::from_jd(jd);
  EXPECT_NEAR(mjd.value(), jd.to_mjd(), 1e-10);
}

TEST(Time, MjdRoundtrip) {
  auto mjd1 = MJD(60200.0);
  auto jd = mjd1.to_jd();
  auto mjd2 = MJD::from_jd(jd);
  EXPECT_NEAR(mjd1.value(), mjd2.value(), 1e-10);
}

// ============================================================================
// Period
// ============================================================================

TEST(Time, PeriodDuration) {
  Period p(MJD(60200.0), MJD(60201.0));
  EXPECT_NEAR(p.duration().value(), 1.0, 1e-10);
}

TEST(Time, PeriodIntersection) {
  Period a(MJD(60200.0), MJD(60202.0));
  Period b(MJD(60201.0), MJD(60203.0));
  auto c = a.intersection(b);
  EXPECT_NEAR(c.start().value(), 60201.0, 1e-10);
  EXPECT_NEAR(c.end().value(), 60202.0, 1e-10);
}

TEST(Time, PeriodNoIntersection) {
  Period a(MJD(60200.0), MJD(60201.0));
  Period b(MJD(60202.0), MJD(60203.0));
  EXPECT_THROW(a.intersection(b), tempoch::NoIntersectionError);
}

TEST(Time, PeriodInvalidThrows) {
  EXPECT_THROW(Period(MJD(60203.0), MJD(60200.0)), tempoch::InvalidPeriodError);
}

// ============================================================================
// Typed-quantity (_qty) methods
// ============================================================================

TEST(Time, JulianCenturiesQty) {
  auto jd = JulianDate::J2000();
  auto jc = jd.julian_centuries_qty();
  EXPECT_NEAR(jc.value(), 0.0, 1e-10);
  EXPECT_EQ(jc.unit_id(), UNIT_ID_JULIAN_CENTURY);
}

TEST(Time, JulianCenturiesQtyNonZero) {
  // 36525 days ≈ 1 Julian century
  auto jd = JulianDate(2451545.0 + 36525.0);
  auto jc = jd.julian_centuries_qty();
  EXPECT_NEAR(jc.value(), 1.0, 1e-10);
}

TEST(Time, ArithmeticWithHours) {
  auto jd1 = JulianDate(2451545.0);
  auto jd2 = jd1 + qtty::Hour(24.0);
  EXPECT_NEAR((jd2 - jd1).value(), 1.0, 1e-10);
}

TEST(Time, ArithmeticWithMinutes) {
  auto mjd1 = MJD(60200.0);
  auto mjd2 = mjd1 + qtty::Minute(1440.0);
  EXPECT_NEAR((mjd2 - mjd1).value(), 1.0, 1e-10);
}

TEST(Time, SubtractQuantityHours) {
  auto jd1 = JulianDate(2451546.0);
  auto jd2 = jd1 - qtty::Hour(12.0);
  EXPECT_NEAR(jd2.value(), 2451545.5, 1e-10);
}

TEST(Time, DifferenceConvertible) {
  auto jd1 = JulianDate(2451545.0);
  auto jd2 = JulianDate(2451546.0);
  auto diff = jd2 - jd1;
  auto hours = diff.to<qtty::Hour>();
  EXPECT_NEAR(hours.value(), 24.0, 1e-10);
}

TEST(Time, PeriodDurationInMinutes) {
  Period p(MJD(60200.0), MJD(60200.5));
  auto min = p.duration<qtty::Minute>();
  EXPECT_NEAR(min.value(), 720.0, 1e-6);
}
