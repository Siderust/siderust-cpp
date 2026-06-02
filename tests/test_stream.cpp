// SPDX-License-Identifier: AGPL-3.0-or-later
// Copyright (C) 2026 Vallés Puig, Ramon

#include <sstream>
#include <string>

#include <gtest/gtest.h>
#include <siderust/siderust.hpp>

using namespace siderust;
using namespace siderust::frames;
using namespace siderust::centers;

namespace {

template <typename T> std::string stream_to_string(const T &value) {
  std::ostringstream oss;
  oss << value;
  return oss.str();
}

} // namespace

TEST(StreamOutput, CartesianDirection) {
  const auto s = stream_to_string(cartesian::Direction<ICRS>{0.6, 0.8, 0.0});
  EXPECT_NE(s.find("ICRS"), std::string::npos);
  EXPECT_NE(s.find("direction"), std::string::npos);
  EXPECT_NE(s.find("x="), std::string::npos);
}

TEST(StreamOutput, CartesianPosition) {
  cartesian::Position<Heliocentric, EclipticMeanJ2000, qtty::AstronomicalUnit> pos(1.0, 0.0, 0.0);
  const auto s = stream_to_string(pos);
  EXPECT_NE(s.find("Heliocentric"), std::string::npos);
  EXPECT_NE(s.find("EclipticMeanJ2000"), std::string::npos);
  EXPECT_NE(s.find("x="), std::string::npos);
}

TEST(StreamOutput, CartesianDisplacement) {
  cartesian::Displacement<EclipticMeanJ2000, qtty::AstronomicalUnit> d(
      qtty::AstronomicalUnit(0.1), qtty::AstronomicalUnit(0.0), qtty::AstronomicalUnit(0.0));
  const auto s = stream_to_string(d);
  EXPECT_NE(s.find("EclipticMeanJ2000"), std::string::npos);
  EXPECT_NE(s.find("displacement"), std::string::npos);
  EXPECT_NE(s.find("dx="), std::string::npos);
}

TEST(StreamOutput, SphericalDirectionRaDec) {
  spherical::Direction<EquatorialMeanJ2000> dir(qtty::Degree(10.0), qtty::Degree(20.0));
  const auto s = stream_to_string(dir);
  EXPECT_NE(s.find("EquatorialMeanJ2000"), std::string::npos);
  EXPECT_NE(s.find("ra="), std::string::npos);
  EXPECT_NE(s.find("dec="), std::string::npos);
}

TEST(StreamOutput, SphericalDirectionHorizontal) {
  spherical::Direction<Horizontal> dir(qtty::Degree(90.0), qtty::Degree(45.0));
  const auto s = stream_to_string(dir);
  EXPECT_NE(s.find("Horizontal"), std::string::npos);
  EXPECT_NE(s.find("az="), std::string::npos);
  EXPECT_NE(s.find("alt="), std::string::npos);
}

TEST(StreamOutput, SphericalDirectionLonLat) {
  spherical::Direction<EclipticMeanJ2000> dir(qtty::Degree(30.0), qtty::Degree(-10.0));
  const auto s = stream_to_string(dir);
  EXPECT_NE(s.find("EclipticMeanJ2000"), std::string::npos);
  EXPECT_NE(s.find("lon="), std::string::npos);
  EXPECT_NE(s.find("lat="), std::string::npos);
}

TEST(StreamOutput, SphericalPosition) {
  spherical::Position<Geocentric, EquatorialMeanJ2000, qtty::AstronomicalUnit> pos(
      qtty::Degree(45.0), qtty::Degree(10.0), qtty::AstronomicalUnit(1.0));
  const auto s = stream_to_string(pos);
  EXPECT_NE(s.find("Geocentric"), std::string::npos);
  EXPECT_NE(s.find("ra="), std::string::npos);
  EXPECT_NE(s.find("r="), std::string::npos);
}

TEST(StreamOutput, Geodetic) {
  const auto s = stream_to_string(Geodetic(2.35, 48.85, 35.0));
  EXPECT_NE(s.find("Geodetic"), std::string::npos);
  EXPECT_NE(s.find("lon="), std::string::npos);
  EXPECT_NE(s.find("lat="), std::string::npos);
}

TEST(StreamOutput, JulianDate) {
  const auto s = stream_to_string(JulianDate::J2000());
  EXPECT_NE(s.find("TT"), std::string::npos);
  EXPECT_NE(s.find("JD"), std::string::npos);
}

TEST(StreamOutput, QttyQuantityInCoordinate) {
  cartesian::Position<Heliocentric, EclipticMeanJ2000, qtty::AstronomicalUnit> pos(1.0, 0.0, 0.0);
  const auto s = stream_to_string(pos);
  EXPECT_NE(s.find("x="), std::string::npos);
  EXPECT_NE(s.find(qtty::UnitTraits<qtty::AstronomicalUnit>::symbol()), std::string::npos);
}
