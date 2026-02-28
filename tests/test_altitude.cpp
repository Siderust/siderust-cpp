#include <cmath>
#include <gtest/gtest.h>
#include <siderust/siderust.hpp>

using namespace siderust;
using namespace qtty::literals;

static const double PI = 3.14159265358979323846;

class AltitudeTest : public ::testing::Test {
protected:
  Geodetic obs;
  MJD start;
  MJD end_;
  Period window{MJD(0.0), MJD(1.0)};

  void SetUp() override {
    obs = ROQUE_DE_LOS_MUCHACHOS();
    start = MJD::from_jd(JulianDate::from_utc({2026, 7, 15, 18, 0, 0}));
    end_ = start + 1.0_d; // 24 hours
    window = Period(start, end_);
  }
};

// ============================================================================
// Sun
// ============================================================================

TEST_F(AltitudeTest, SunAltitudeAt) {
  qtty::Radian alt = sun::altitude_at(obs, start);
  // Should be a valid radian value
  EXPECT_GT(alt.value(), -PI / 2.0);
  EXPECT_LT(alt.value(), PI / 2.0);
}

TEST_F(AltitudeTest, SunAboveThreshold) {
  // Find periods when sun > 0 deg (daytime)
  auto periods = sun::above_threshold(obs, window, 0.0_deg);
  EXPECT_GT(periods.size(), 0u);
  for (auto &p : periods) {
    EXPECT_GT(p.duration().value(), 0.0);
  }
}

TEST_F(AltitudeTest, SunBelowThreshold) {
  // Astronomical night: sun < -18°
  auto periods = sun::below_threshold(obs, window, -18.0_deg);
  // In July at La Palma, astronomical night may be short but should exist
  // (or possibly not if too close to solstice — accept 0+)
  for (auto &p : periods) {
    EXPECT_GT(p.duration().value(), 0.0);
  }
}

TEST_F(AltitudeTest, SunCrossings) {
  auto events = sun::crossings(obs, window, 0.0_deg);
  // Expect at least 1 crossing in 24h (sunrise or sunset)
  EXPECT_GE(events.size(), 1u);
}

TEST_F(AltitudeTest, SunCulminations) {
  auto events = sun::culminations(obs, window);
  // At least one culmination (meridian passage)
  EXPECT_GE(events.size(), 1u);
}

TEST_F(AltitudeTest, SunAltitudePeriods) {
  // Find periods when sun is between -6° and 0° (civil twilight)
  auto periods =
      sun::altitude_periods(obs, window, -6.0_deg, 0.0_deg);
  for (auto &p : periods) {
    EXPECT_GT(p.duration().value(), 0.0);
  }
}

// ============================================================================
// Moon
// ============================================================================

TEST_F(AltitudeTest, MoonAltitudeAt) {
  qtty::Radian alt = moon::altitude_at(obs, start);
  EXPECT_GT(alt.value(), -PI / 2.0);
  EXPECT_LT(alt.value(), PI / 2.0);
}

TEST_F(AltitudeTest, MoonAboveThreshold) {
  auto periods = moon::above_threshold(obs, window, 0.0_deg);
  // Moon may or may not be above horizon for this date; just no crash
  for (auto &p : periods) {
    EXPECT_GT(p.duration().value(), 0.0);
  }
}

// ============================================================================
// Star
// ============================================================================

TEST_F(AltitudeTest, StarAltitudeAt) {
  const auto &vega = VEGA();
  qtty::Radian alt = star_altitude::altitude_at(vega, obs, start);
  EXPECT_GT(alt.value(), -PI / 2.0);
  EXPECT_LT(alt.value(), PI / 2.0);
}

TEST_F(AltitudeTest, StarAboveThreshold) {
  const auto &vega = VEGA();
  auto periods =
      star_altitude::above_threshold(vega, obs, window, 30.0_deg);
  // Vega should be well above 30° from La Palma in July
  EXPECT_GT(periods.size(), 0u);
}

// ============================================================================
// ICRS direction
// ============================================================================

TEST_F(AltitudeTest, IcrsAltitudeAt) {
  const spherical::direction::ICRS vega_icrs(279.23_deg,
                                             38.78_deg);
  qtty::Radian alt = icrs_altitude::altitude_at(vega_icrs, obs, start);
  EXPECT_GT(alt.value(), -PI / 2.0);
  EXPECT_LT(alt.value(), PI / 2.0);
}

TEST_F(AltitudeTest, IcrsAboveThreshold) {
  const spherical::direction::ICRS vega_icrs(279.23_deg,
                                             38.78_deg);
  auto periods = icrs_altitude::above_threshold(vega_icrs, obs, window,
                                                30.0_deg);
  EXPECT_GT(periods.size(), 0u);
}

// ============================================================================
// Target<C> — generic strongly-typed target
// ============================================================================

// Vega ICRS coordinates (J2000): RA=279.2348°, Dec=+38.7836°
TEST_F(AltitudeTest, ICRSTargetAltitudeAt) {
  ICRSTarget vega{
      spherical::direction::ICRS{279.23_deg, 38.78_deg}};
  // altitude_at returns qtty::Degree (radian/degree bug-fix verification)
  qtty::Degree alt = vega.altitude_at(obs, start);
  EXPECT_GT(alt.value(), -90.0);
  EXPECT_LT(alt.value(), 90.0);
}

TEST_F(AltitudeTest, ICRSTargetAboveThreshold) {
  ICRSTarget vega{
      spherical::direction::ICRS{279.23_deg, 38.78_deg}};
  auto periods = vega.above_threshold(obs, window, 30.0_deg);
  // Vega should rise above 30° from La Palma in July
  EXPECT_GT(periods.size(), 0u);
}

TEST_F(AltitudeTest, ICRSTargetTypedAccessors) {
  ICRSTarget vega{
      spherical::direction::ICRS{279.23_deg, 38.78_deg}};
  EXPECT_NEAR(vega.ra().value(), 279.23, 1e-9);
  EXPECT_NEAR(vega.dec().value(), 38.78, 1e-9);
  // epoch defaults to J2000
  EXPECT_NEAR(vega.epoch().value(), 2451545.0, 1e-3);
  // icrs_direction is the same for an ICRS Target
  EXPECT_NEAR(vega.icrs_direction().ra().value(), 279.23, 1e-9);
}

TEST_F(AltitudeTest, ICRSTargetPolymorphic) {
  // Verify DirectionTarget<C> is usable through the Target interface
  std::unique_ptr<Target> t = std::make_unique<ICRSTarget>(
      spherical::direction::ICRS{279.23_deg, 38.78_deg});
  qtty::Degree alt = t->altitude_at(obs, start);
  EXPECT_GT(alt.value(), -90.0);
  EXPECT_LT(alt.value(), 90.0);
}

TEST_F(AltitudeTest, EclipticTargetAltitudeAt) {
  // Vega in ecliptic J2000 coordinates (approx): lon≈279.6°, lat≈+61.8°
  EclipticMeanJ2000Target ec{spherical::direction::EclipticMeanJ2000{
      279.6_deg, 61.8_deg}};
  // ecl direction retained on the C++ side
  EXPECT_NEAR(ec.direction().lon().value(), 279.6, 1e-9);
  EXPECT_NEAR(ec.direction().lat().value(), 61.8, 1e-9);
  // ICRS ra/dec computed at construction and accessible
  EXPECT_GT(ec.icrs_direction().ra().value(), 0.0);
  EXPECT_LT(ec.icrs_direction().ra().value(), 360.0);
  // altitude should be a valid degree value
  qtty::Degree alt = ec.altitude_at(obs, start);
  EXPECT_GT(alt.value(), -90.0);
  EXPECT_LT(alt.value(), 90.0);
}

TEST_F(AltitudeTest, EquatorialMeanJ2000TargetAltitudeAt) {
  EquatorialMeanJ2000Target vega{spherical::direction::EquatorialMeanJ2000{
      279.23_deg, 38.78_deg}};
  qtty::Degree alt = vega.altitude_at(obs, start);
  EXPECT_GT(alt.value(), -90.0);
  EXPECT_LT(alt.value(), 90.0);
}
