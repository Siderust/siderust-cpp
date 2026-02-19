#include <gtest/gtest.h>
#include <siderust/siderust.hpp>

using namespace siderust;

TEST(Observatories, RoqueDeLos) {
    auto obs = roque_de_los_muchachos();
    // La Palma, approx lon=-17.88, lat=28.76
    EXPECT_NEAR(obs.lon_deg, -17.88, 0.1);
    EXPECT_NEAR(obs.lat_deg, 28.76, 0.1);
    EXPECT_GT(obs.height_m, 2000.0);
}

TEST(Observatories, ElParanal) {
    auto obs = el_paranal();
    EXPECT_LT(obs.lon_deg, 0.0);
    EXPECT_LT(obs.lat_deg, 0.0);  // Southern hemisphere
    EXPECT_GT(obs.height_m, 2000.0);
}

TEST(Observatories, MaunaKea) {
    auto obs = mauna_kea();
    EXPECT_NEAR(obs.lon_deg, -155.47, 0.1);
    EXPECT_NEAR(obs.lat_deg, 19.82, 0.1);
    EXPECT_GT(obs.height_m, 4000.0);
}

TEST(Observatories, LaSilla) {
    auto obs = la_silla();
    EXPECT_LT(obs.lon_deg, 0.0);
    EXPECT_LT(obs.lat_deg, 0.0);
}

TEST(Observatories, CustomGeodetic) {
    auto g = geodetic(-3.7, 40.4, 667.0);
    EXPECT_NEAR(g.lon_deg, -3.7, 1e-10);
    EXPECT_NEAR(g.lat_deg, 40.4, 1e-10);
    EXPECT_NEAR(g.height_m, 667.0, 1e-10);
}
