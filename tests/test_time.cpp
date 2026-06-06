#include <gtest/gtest.h>
#include <siderust/siderust.hpp>

#include <type_traits>

using namespace siderust;

TEST(Time, ReexportsExposeExplicitScaleTypes) {
  static_assert(
      std::is_same_v<Time<TT, JD>, tempoch::EncodedTime<tempoch::scale::TT, tempoch::format::JD>>);
  static_assert(std::is_same_v<Time<TT, MJD>,
                               tempoch::EncodedTime<tempoch::scale::TT, tempoch::format::MJD>>);
}

TEST(Time, TtJulianDateRoundtripsThroughUtcConvenience) {
  auto jd_tt = Time<TT, JD>::from_utc({2026, 7, 15, 22, 0, 0});
  auto roundtrip = jd_tt.to_utc();
  auto mjd_tt = jd_tt.to<TT, MJD>();

  static_assert(std::is_same_v<decltype(mjd_tt), Time<TT, MJD>>);

  EXPECT_EQ(roundtrip.year, 2026);
  EXPECT_EQ(roundtrip.month, 7);
  EXPECT_EQ(roundtrip.day, 15);
}

TEST(Time, TtPeriodCompilesAndBehaves) {
  Period<TT, MJD> period(Time<TT, MJD>(60200.0), Time<TT, MJD>(60201.0));
  EXPECT_NEAR(period.duration<qtty::Hour>().value(), 24.0, 1e-9);
}
