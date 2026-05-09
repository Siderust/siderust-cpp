#include <gtest/gtest.h>
#include <siderust/siderust.hpp>

using namespace siderust;

// ============================================================================
// TwilightPhase — classification from Sun altitude
// ============================================================================

TEST(Twilight, DayPhase) {
  // Sun at +10° → Day
  EXPECT_EQ(twilight_phase(qtty::Degree(10.0)), TwilightPhase::Day);
}

TEST(Twilight, CivilPhase) {
  // Sun at -3° → Civil
  EXPECT_EQ(twilight_phase(qtty::Degree(-3.0)), TwilightPhase::Civil);
}

TEST(Twilight, NauticalPhase) {
  // Sun at -9° → Nautical
  EXPECT_EQ(twilight_phase(qtty::Degree(-9.0)), TwilightPhase::Nautical);
}

TEST(Twilight, AstronomicalPhase) {
  // Sun at -15° → Astronomical
  EXPECT_EQ(twilight_phase(qtty::Degree(-15.0)), TwilightPhase::Astronomical);
}

TEST(Twilight, DarkPhase) {
  // Sun at -20° → Dark
  EXPECT_EQ(twilight_phase(qtty::Degree(-20.0)), TwilightPhase::Dark);
}

TEST(Twilight, BoundaryHorizon) {
  // Exactly at the horizon (0°) → Day (altitude > 0 is Day; 0 is Civil per IAU)
  auto phase = twilight_phase(qtty::Degree(0.0));
  // Boundary is implementation-defined; just verify it's Civil or Day.
  EXPECT_TRUE(phase == TwilightPhase::Civil || phase == TwilightPhase::Day);
}

TEST(Twilight, RadianOverload) {
  // -pi/20 rad ≈ -9° → Nautical
  constexpr double PI = 3.14159265358979323846;
  auto phase = twilight_phase(qtty::Radian(-PI / 20.0));
  EXPECT_EQ(phase, TwilightPhase::Nautical);
}
