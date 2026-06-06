#include <gtest/gtest.h>
#include <siderust/siderust.hpp>

using namespace siderust;
using namespace qtty::literals;

// ============================================================================
// Star
// ============================================================================

TEST(Bodies, StarCatalogVega) {
  const auto &vega = VEGA();
  EXPECT_EQ(vega.name(), "Vega");
  EXPECT_NEAR(vega.distance_ly(), 25.0, 1.0);
  EXPECT_GT(vega.luminosity_solar(), 1.0);
}

TEST(Bodies, StarCatalogSirius) {
  const auto &sirius = SIRIUS();
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
  auto s = Star::create("TestStar",
                        StellarProperties{qtty::LightYear(100.0), SolarMass{1.0}, SolarRadius{1.0},
                                          SolarLuminosity{1.0}},
                        spherical::direction::ICRS(180.0_deg, 45.0_deg), Time<TT, JD>::J2000());
  EXPECT_EQ(s.name(), "TestStar");
  EXPECT_NEAR(s.distance_ly(), 100.0, 1e-6);
}

TEST(Bodies, StarCreateWithProperMotion) {
  ProperMotion pm{AngularRate{0.001_deg, qtty::Day(365.25)},
                  AngularRate{-0.002_deg, qtty::Day(365.25)}, RaConvention::MuAlphaStar};
  auto s = Star::create("PMStar",
                        StellarProperties{qtty::LightYear(50.0), SolarMass{1.0}, SolarRadius{1.0},
                                          SolarLuminosity{1.0}},
                        spherical::direction::ICRS(100.0_deg, 30.0_deg), Time<TT, JD>::J2000(), pm);
  EXPECT_EQ(s.name(), "PMStar");
}

// ============================================================================
// Planet
// ============================================================================

TEST(Bodies, PlanetEarth) {
  auto e = EARTH();
  EXPECT_NEAR(e.mass.value(), 5.972e24, 0.01e24);
  EXPECT_NEAR(e.radius.value(), 6371.0, 10.0);
  EXPECT_NEAR(e.orbit.semi_major_axis.value(), 1.0, 0.01);
}

TEST(Bodies, PlanetMars) {
  auto m = MARS();
  EXPECT_GT(m.mass.value(), 0);
  EXPECT_NEAR(m.orbit.semi_major_axis.value(), 1.524, 0.01);
}

TEST(Bodies, AllPlanets) {
  // Ensure all static constants are populated.
  EXPECT_GT(MERCURY().mass.value(), 0.0);
  EXPECT_GT(VENUS().mass.value(), 0.0);
  EXPECT_GT(EARTH().mass.value(), 0.0);
  EXPECT_GT(MARS().mass.value(), 0.0);
  EXPECT_GT(JUPITER().mass.value(), 0.0);
  EXPECT_GT(SATURN().mass.value(), 0.0);
  EXPECT_GT(URANUS().mass.value(), 0.0);
  EXPECT_GT(NEPTUNE().mass.value(), 0.0);
}

// ============================================================================
// BodyTarget — solar-system body via Target polymorphism
// ============================================================================

TEST(Bodies, BodyTargetSunAltitude) {
  BodyTarget sun(Body::Sun);
  auto obs = geodetic(2.35, 48.85, 35.0);
  auto mjd = Time<TT, MJD>(60000.5);
  auto alt = sun.altitude_at(obs, mjd);
  EXPECT_TRUE(std::isfinite(alt.value()));
  EXPECT_GT(alt.value(), -90.0);
  EXPECT_LT(alt.value(), 90.0);
}

TEST(Bodies, BodyTargetMarsAltitude) {
  BodyTarget mars(Body::Mars);
  auto obs = geodetic(2.35, 48.85, 35.0);
  auto mjd = Time<TT, MJD>(60000.5);
  auto alt = mars.altitude_at(obs, mjd);
  EXPECT_TRUE(std::isfinite(alt.value()));
  EXPECT_GT(alt.value(), -90.0);
  EXPECT_LT(alt.value(), 90.0);
}

TEST(Bodies, BodyTargetAllBodiesAltitude) {
  auto obs = geodetic(-17.89, 28.76, 2326.0); // ORM
  auto mjd = Time<TT, MJD>(60000.5);
  std::vector<Body> all = {Body::Sun,     Body::Moon,   Body::Mercury, Body::Venus,  Body::Mars,
                           Body::Jupiter, Body::Saturn, Body::Uranus,  Body::Neptune};
  for (auto b : all) {
    BodyTarget bt(b);
    auto alt = bt.altitude_at(obs, mjd);
    EXPECT_TRUE(std::isfinite(alt.value()));
  }
}

TEST(Bodies, BodyTargetAzimuth) {
  BodyTarget sun(Body::Sun);
  auto obs = geodetic(2.35, 48.85, 35.0);
  auto mjd = Time<TT, MJD>(60000.5);
  auto az = sun.azimuth_at(obs, mjd);
  EXPECT_GE(az.value(), 0.0);
  EXPECT_LT(az.value(), 360.0);
}

TEST(Bodies, BodyTargetJupiterAzimuth) {
  BodyTarget jup(Body::Jupiter);
  auto obs = geodetic(2.35, 48.85, 35.0);
  auto mjd = Time<TT, MJD>(60000.5);
  auto az = jup.azimuth_at(obs, mjd);
  EXPECT_TRUE(std::isfinite(az.value()));
  EXPECT_GE(az.value(), 0.0);
  EXPECT_LT(az.value(), 360.0);
}

TEST(Bodies, BodyTargetAboveThreshold) {
  BodyTarget sun(Body::Sun);
  auto obs = geodetic(2.35, 48.85, 35.0);
  auto window = Period<TT, MJD>(Time<TT, MJD>(60000.0), Time<TT, MJD>(60001.0));
  auto periods = sun.above_threshold(obs, window, qtty::Degree(0.0));
  // Sun should be above horizon for some portion of the day
  EXPECT_GT(periods.size(), 0u);
}

TEST(Bodies, BodyTargetPolymorphic) {
  auto obs = geodetic(2.35, 48.85, 35.0);
  auto mjd = Time<TT, MJD>(60000.5);

  std::vector<std::unique_ptr<Target>> targets;
  targets.push_back(std::make_unique<BodyTarget>(Body::Sun));
  targets.push_back(std::make_unique<BodyTarget>(Body::Mars));

  for (const auto &t : targets) {
    auto alt = t->altitude_at(obs, mjd);
    EXPECT_TRUE(std::isfinite(alt.value()));
  }
}

TEST(Bodies, BodyNamespaceAltitudeAt) {
  auto obs = geodetic(2.35, 48.85, 35.0);
  auto mjd = Time<TT, MJD>(60000.5);
  auto rad = body::altitude_at(Body::Saturn, obs, mjd);
  EXPECT_TRUE(std::isfinite(rad.value()));
}

TEST(Bodies, BodyNamespaceAzimuthAt) {
  auto obs = geodetic(2.35, 48.85, 35.0);
  auto mjd = Time<TT, MJD>(60000.5);
  auto rad = body::azimuth_at(Body::Venus, obs, mjd);
  EXPECT_TRUE(std::isfinite(rad.value()));
  EXPECT_GE(rad.value(), 0.0);
}

// ============================================================================
// StarTarget — Target implementation for catalog stars
// ============================================================================

TEST(Bodies, StarTargetAltitude) {
  const auto &vega = VEGA();
  StarTarget st(vega);
  auto obs = geodetic(2.35, 48.85, 35.0);
  auto mjd = Time<TT, MJD>(60000.5);
  auto alt = st.altitude_at(obs, mjd);
  EXPECT_TRUE(std::isfinite(alt.value()));
  EXPECT_GT(alt.value(), -90.0);
  EXPECT_LT(alt.value(), 90.0);
}

TEST(Bodies, StarTargetPolymorphicWithBodyTarget) {
  auto obs = geodetic(2.35, 48.85, 35.0);
  auto mjd = Time<TT, MJD>(60000.5);

  std::vector<std::unique_ptr<Target>> targets;
  targets.push_back(std::make_unique<BodyTarget>(Body::Sun));
  targets.push_back(std::make_unique<StarTarget>(VEGA()));

  for (const auto &t : targets) {
    auto alt = t->altitude_at(obs, mjd);
    EXPECT_TRUE(std::isfinite(alt.value()));
  }
}
