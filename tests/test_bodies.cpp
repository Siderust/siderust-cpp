#include <gtest/gtest.h>
#include <siderust/siderust.hpp>

using namespace siderust;

// ============================================================================
// Star
// ============================================================================

TEST(Bodies, StarCatalogVega) {
    const auto& vega = VEGA;
    EXPECT_EQ(vega.name(), "Vega");
    EXPECT_NEAR(vega.distance_ly(), 25.0, 1.0);
    EXPECT_GT(vega.luminosity_solar(), 1.0);
}

TEST(Bodies, StarCatalogSirius) {
    const auto& sirius = SIRIUS;
    EXPECT_EQ(sirius.name(), "Sirius");
    EXPECT_NEAR(sirius.distance_ly(), 8.6, 0.5);
}

TEST(Bodies, StarCatalogUnknownThrows) {
    EXPECT_THROW(Star::catalog("NONEXISTENT"), UnknownStarError);
}

TEST(Bodies, StarMoveSemantics) {
    auto s1 = Star::catalog("POLARIS");
    EXPECT_TRUE(static_cast<bool>(s1));

    auto s2 = std::move(s1);
    EXPECT_TRUE(static_cast<bool>(s2));
    // s1 is now empty (moved-from)
}

TEST(Bodies, StarCreate) {
    auto s = Star::create(
        "TestStar",
        100.0,  // distance_ly
        1.0,    // mass_solar
        1.0,    // radius_solar
        1.0,    // luminosity_solar
        180.0,  // ra_deg
        45.0,   // dec_deg
        2451545.0  // epoch_jd (J2000)
    );
    EXPECT_EQ(s.name(), "TestStar");
    EXPECT_NEAR(s.distance_ly(), 100.0, 1e-6);
}

TEST(Bodies, StarCreateWithProperMotion) {
    ProperMotion pm(0.001, -0.002, RaConvention::MuAlphaStar);
    auto s = Star::create("PMStar", 50.0, 1.0, 1.0, 1.0,
                          100.0, 30.0, 2451545.0, pm);
    EXPECT_EQ(s.name(), "PMStar");
}

// ============================================================================
// Planet
// ============================================================================

TEST(Bodies, PlanetEarth) {
    auto e = EARTH;
    EXPECT_NEAR(e.mass_kg, 5.972e24, 0.01e24);
    EXPECT_NEAR(e.radius_km, 6371.0, 10.0);
    EXPECT_NEAR(e.orbit.semi_major_axis_au, 1.0, 0.01);
}

TEST(Bodies, PlanetMars) {
    auto m = MARS;
    EXPECT_GT(m.mass_kg, 0);
    EXPECT_NEAR(m.orbit.semi_major_axis_au, 1.524, 0.01);
}

TEST(Bodies, AllPlanets) {
    // Ensure all static constants are populated.
    EXPECT_GT(MERCURY.mass_kg, 0.0);
    EXPECT_GT(VENUS.mass_kg, 0.0);
    EXPECT_GT(EARTH.mass_kg, 0.0);
    EXPECT_GT(MARS.mass_kg, 0.0);
    EXPECT_GT(JUPITER.mass_kg, 0.0);
    EXPECT_GT(SATURN.mass_kg, 0.0);
    EXPECT_GT(URANUS.mass_kg, 0.0);
    EXPECT_GT(NEPTUNE.mass_kg, 0.0);
}
