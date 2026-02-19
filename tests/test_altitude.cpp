#include <gtest/gtest.h>
#include <siderust/siderust.hpp>
#include <cmath>

using namespace siderust;

static const double PI = 3.14159265358979323846;

class AltitudeTest : public ::testing::Test {
protected:
    Geodetic obs;
    MJD      start;
    MJD      end_;

    void SetUp() override {
        obs    = roque_de_los_muchachos();
        start  = MJD::from_jd(JulianDate::from_utc({2026, 7, 15, 18, 0, 0}));
        end_   = start + 1.0;  // 24 hours
    }
};

// ============================================================================
// Sun
// ============================================================================

TEST_F(AltitudeTest, SunAltitudeAt) {
    double alt = sun::altitude_at(obs, start);
    // Should be a valid radian value
    EXPECT_GT(alt, -PI / 2.0);
    EXPECT_LT(alt, PI / 2.0);
}

TEST_F(AltitudeTest, SunAboveThreshold) {
    // Find periods when sun > 0 deg (daytime)
    auto periods = sun::above_threshold(obs, start, end_, 0.0);
    EXPECT_GT(periods.size(), 0u);
    for (auto& p : periods) {
        EXPECT_GT(p.duration_days(), 0.0);
    }
}

TEST_F(AltitudeTest, SunBelowThreshold) {
    // Astronomical night: sun < -18°
    auto periods = sun::below_threshold(obs, start, end_, -18.0);
    // In July at La Palma, astronomical night may be short but should exist
    // (or possibly not if too close to solstice — accept 0+)
    for (auto& p : periods) {
        EXPECT_GT(p.duration_days(), 0.0);
    }
}

TEST_F(AltitudeTest, SunCrossings) {
    auto events = sun::crossings(obs, start, end_, 0.0);
    // Expect at least 1 crossing in 24h (sunrise or sunset)
    EXPECT_GE(events.size(), 1u);
}

TEST_F(AltitudeTest, SunCulminations) {
    auto events = sun::culminations(obs, start, end_);
    // At least one culmination (meridian passage)
    EXPECT_GE(events.size(), 1u);
}

TEST_F(AltitudeTest, SunAltitudePeriods) {
    // Find periods when sun is between -6° and 0° (civil twilight)
    auto periods = sun::altitude_periods(obs, start, end_, -6.0, 0.0);
    for (auto& p : periods) {
        EXPECT_GT(p.duration_days(), 0.0);
    }
}

// ============================================================================
// Moon
// ============================================================================

TEST_F(AltitudeTest, MoonAltitudeAt) {
    double alt = moon::altitude_at(obs, start);
    EXPECT_GT(alt, -PI / 2.0);
    EXPECT_LT(alt, PI / 2.0);
}

TEST_F(AltitudeTest, MoonAboveThreshold) {
    auto periods = moon::above_threshold(obs, start, end_, 0.0);
    // Moon may or may not be above horizon for this date; just no crash
    for (auto& p : periods) {
        EXPECT_GT(p.duration_days(), 0.0);
    }
}

// ============================================================================
// Star
// ============================================================================

TEST_F(AltitudeTest, StarAltitudeAt) {
    auto vega = Star::catalog("VEGA");
    double alt = star_altitude::altitude_at(vega, obs, start);
    EXPECT_GT(alt, -PI / 2.0);
    EXPECT_LT(alt, PI / 2.0);
}

TEST_F(AltitudeTest, StarAboveThreshold) {
    auto vega = Star::catalog("VEGA");
    auto periods = star_altitude::above_threshold(vega, obs, start, end_, 30.0);
    // Vega should be well above 30° from La Palma in July
    EXPECT_GT(periods.size(), 0u);
}

// ============================================================================
// ICRS direction
// ============================================================================

TEST_F(AltitudeTest, IcrsAltitudeAt) {
    // Vega coordinates
    double alt = icrs_altitude::altitude_at(279.23, 38.78, obs, start);
    EXPECT_GT(alt, -PI / 2.0);
    EXPECT_LT(alt, PI / 2.0);
}

TEST_F(AltitudeTest, IcrsAboveThreshold) {
    auto periods = icrs_altitude::above_threshold(
        279.23, 38.78, obs, start, end_, 30.0);
    EXPECT_GT(periods.size(), 0u);
}
