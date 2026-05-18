#include <gtest/gtest.h>
#include <siderust/siderust.hpp>

#include <type_traits>

using namespace siderust;

TEST(Time, ReexportsExposeExplicitScaleTypes) {
  static_assert(std::is_same_v<JulianDate, tempoch::JulianDate<tempoch::scale::TT>>);
  static_assert(std::is_same_v<ModifiedJulianDate,
                               tempoch::ModifiedJulianDate<tempoch::scale::TT>>);
}

TEST(Time, TtJulianDateRoundtripsThroughUtcConvenience) {
  auto jd_tt = JulianDate::from_utc({2026, 7, 15, 22, 0, 0});
  auto roundtrip = jd_tt.to_utc();

  EXPECT_EQ(roundtrip.year, 2026);
  EXPECT_EQ(roundtrip.month, 7);
  EXPECT_EQ(roundtrip.day, 15);
}

TEST(Time, TtPeriodCompilesAndBehaves) {
  Period period(ModifiedJulianDate(60200.0), ModifiedJulianDate(60201.0));
  EXPECT_NEAR(period.duration<qtty::Hour>().value(), 24.0, 1e-9);
}
