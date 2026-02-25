#include <gtest/gtest.h>
#include <siderust/siderust.hpp>

using namespace siderust;

TEST(Observatories, RoqueDeLos) {
  auto obs = ROQUE_DE_LOS_MUCHACHOS;
  // La Palma, approx lon=-17.88, lat=28.76
  EXPECT_NEAR(obs.lon.value(), -17.88, 0.1);
  EXPECT_NEAR(obs.lat.value(), 28.76, 0.1);
  EXPECT_GT(obs.height.value(), 2000.0);
}

TEST(Observatories, ElParanal) {
  auto obs = EL_PARANAL;
  EXPECT_LT(obs.lon.value(), 0.0);
  EXPECT_LT(obs.lat.value(), 0.0); // Southern hemisphere
  EXPECT_GT(obs.height.value(), 2000.0);
}

TEST(Observatories, MaunaKea) {
  auto obs = MAUNA_KEA;
  EXPECT_NEAR(obs.lon.value(), -155.47, 0.1);
  EXPECT_NEAR(obs.lat.value(), 19.82, 0.1);
  EXPECT_GT(obs.height.value(), 4000.0);
}

TEST(Observatories, LaSilla) {
  auto obs = LA_SILLA_OBSERVATORY;
  EXPECT_LT(obs.lon.value(), 0.0);
  EXPECT_LT(obs.lat.value(), 0.0);
}

TEST(Observatories, CustomGeodetic) {
  auto g = geodetic(-3.7, 40.4, 667.0);
  EXPECT_NEAR(g.lon.value(), -3.7, 1e-10);
  EXPECT_NEAR(g.lat.value(), 40.4, 1e-10);
  EXPECT_NEAR(g.height.value(), 667.0, 1e-10);
}
