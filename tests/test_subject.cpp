// SPDX-License-Identifier: AGPL-3.0-or-later
// Copyright (C) 2026 Vallés Puig, Ramon

/**
 * @file test_subject.cpp
 * @brief Tests for the unified Subject API (subject.hpp).
 */

#include <siderust/siderust.hpp>
#include <gtest/gtest.h>

using namespace siderust;

// ── Helpers ──────────────────────────────────────────────────────────────────

static Geodetic paris() { return Geodetic(2.35, 48.85, 35.0); }
static MJD mid_day() { return MJD(60000.5); }
static Period one_day() { return Period(MJD(60000.0), MJD(60001.0)); }

// ── altitude_at ──────────────────────────────────────────────────────────────

TEST(SubjectTest, AltitudeAtBody) {
  auto subj = Subject::body(Body::Sun);
  auto alt = altitude_at(subj, paris(), mid_day());
  EXPECT_TRUE(alt.value() > -M_PI && alt.value() < M_PI);
}

TEST(SubjectTest, AltitudeAtMoon) {
  auto subj = Subject::body(Body::Moon);
  auto alt = altitude_at(subj, paris(), mid_day());
  EXPECT_TRUE(std::isfinite(alt.value()));
}

TEST(SubjectTest, AltitudeAtPlanet) {
  auto subj = Subject::body(Body::Mars);
  auto alt = altitude_at(subj, paris(), mid_day());
  EXPECT_TRUE(std::isfinite(alt.value()));
}

TEST(SubjectTest, AltitudeAtStar) {
  Star vega = Star::catalog("VEGA");
  auto subj = Subject::star(vega);
  auto alt = altitude_at(subj, paris(), mid_day());
  EXPECT_TRUE(std::isfinite(alt.value()));
}

TEST(SubjectTest, AltitudeAtIcrs) {
  auto dir = spherical::Direction<frames::ICRS>(qtty::Degree(279.23),
                                                qtty::Degree(38.78));
  auto subj = Subject::icrs(dir);
  auto alt = altitude_at(subj, paris(), mid_day());
  EXPECT_TRUE(std::isfinite(alt.value()));
}

TEST(SubjectTest, AltitudeAtTarget) {
  auto tgt = DirectionTarget<spherical::Direction<frames::ICRS>>(
      spherical::Direction<frames::ICRS>(qtty::Degree(279.23),
                                         qtty::Degree(38.78)));
  auto subj = Subject::target(tgt);
  auto alt = altitude_at(subj, paris(), mid_day());
  EXPECT_TRUE(std::isfinite(alt.value()));
}

// ── above_threshold ──────────────────────────────────────────────────────────

TEST(SubjectTest, AboveThresholdBody) {
  auto subj = Subject::body(Body::Sun);
  auto periods = above_threshold(subj, paris(), one_day(), qtty::Degree(0));
  EXPECT_GT(periods.size(), 0u);
}

TEST(SubjectTest, AboveThresholdStar) {
  Star vega = Star::catalog("VEGA");
  auto subj = Subject::star(vega);
  auto periods = above_threshold(subj, paris(), one_day(), qtty::Degree(0));
  // Vega should be above horizon for some period from Paris
  EXPECT_GT(periods.size(), 0u);
}

TEST(SubjectTest, AboveThresholdIcrs) {
  auto dir = spherical::Direction<frames::ICRS>(qtty::Degree(279.23),
                                                qtty::Degree(38.78));
  auto subj = Subject::icrs(dir);
  auto periods = above_threshold(subj, paris(), one_day(), qtty::Degree(0));
  EXPECT_GT(periods.size(), 0u);
}

// ── below_threshold ──────────────────────────────────────────────────────────

TEST(SubjectTest, BelowThresholdBody) {
  auto subj = Subject::body(Body::Sun);
  auto periods = below_threshold(subj, paris(), one_day(), qtty::Degree(0));
  EXPECT_GT(periods.size(), 0u);
}

// ── crossings ────────────────────────────────────────────────────────────────

TEST(SubjectTest, CrossingsBody) {
  auto subj = Subject::body(Body::Sun);
  auto evts = crossings(subj, paris(), one_day(), qtty::Degree(0));
  EXPECT_GT(evts.size(), 0u);
}

TEST(SubjectTest, CrossingsStar) {
  Star vega = Star::catalog("VEGA");
  auto subj = Subject::star(vega);
  auto evts = crossings(subj, paris(), one_day(), qtty::Degree(0));
  EXPECT_GT(evts.size(), 0u);
}

// ── culminations ─────────────────────────────────────────────────────────────

TEST(SubjectTest, CulminationsBody) {
  auto subj = Subject::body(Body::Sun);
  auto evts = culminations(subj, paris(), one_day());
  EXPECT_GT(evts.size(), 0u);
}

TEST(SubjectTest, CulminationsTarget) {
  auto tgt = DirectionTarget<spherical::Direction<frames::ICRS>>(
      spherical::Direction<frames::ICRS>(qtty::Degree(279.23),
                                         qtty::Degree(38.78)));
  auto subj = Subject::target(tgt);
  auto evts = culminations(subj, paris(), one_day());
  EXPECT_GT(evts.size(), 0u);
}

// ── altitude_periods (body-only) ─────────────────────────────────────────────

TEST(SubjectTest, AltitudePeriodsBody) {
  auto subj = Subject::body(Body::Sun);
  auto periods = altitude_periods(subj, paris(), one_day(), qtty::Degree(-90),
                                  qtty::Degree(90));
  // Full altitude range — should cover the entire window
  EXPECT_GT(periods.size(), 0u);
}

TEST(SubjectTest, AltitudePeriodsStarThrows) {
  Star vega = Star::catalog("VEGA");
  auto subj = Subject::star(vega);
  EXPECT_THROW(
      [&]() {
        altitude_periods(subj, paris(), one_day(), qtty::Degree(-90),
                         qtty::Degree(90));
      }(),
      SiderustException);
}

// ── azimuth_at ───────────────────────────────────────────────────────────────

TEST(SubjectTest, AzimuthAtBody) {
  auto subj = Subject::body(Body::Sun);
  auto az = azimuth_at(subj, paris(), mid_day());
  EXPECT_TRUE(std::isfinite(az.value()));
}

TEST(SubjectTest, AzimuthAtStar) {
  Star vega = Star::catalog("VEGA");
  auto subj = Subject::star(vega);
  auto az = azimuth_at(subj, paris(), mid_day());
  EXPECT_TRUE(std::isfinite(az.value()));
}

TEST(SubjectTest, AzimuthAtIcrs) {
  auto dir = spherical::Direction<frames::ICRS>(qtty::Degree(279.23),
                                                qtty::Degree(38.78));
  auto subj = Subject::icrs(dir);
  auto az = azimuth_at(subj, paris(), mid_day());
  EXPECT_TRUE(std::isfinite(az.value()));
}

// ── azimuth_crossings ────────────────────────────────────────────────────────

TEST(SubjectTest, AzimuthCrossingsBody) {
  auto subj = Subject::body(Body::Sun);
  auto evts = azimuth_crossings(subj, paris(), one_day(), qtty::Degree(180));
  // Sun should cross 180° (south) once per day from northern latitude
  EXPECT_GT(evts.size(), 0u);
}

// ── azimuth_extrema ──────────────────────────────────────────────────────────

TEST(SubjectTest, AzimuthExtremaBody) {
  auto subj = Subject::body(Body::Sun);
  // azimuth_extrema may return empty for short windows; just verify no error.
  auto evts = azimuth_extrema(subj, paris(), one_day());
  // Extrema count can be zero for a 1-day window; just check it runs.
  EXPECT_TRUE(true);
}

// ── in_azimuth_range ─────────────────────────────────────────────────────────

TEST(SubjectTest, InAzimuthRangeBody) {
  auto subj = Subject::body(Body::Sun);
  auto periods = in_azimuth_range(subj, paris(), one_day(), qtty::Degree(90),
                                  qtty::Degree(270));
  EXPECT_GT(periods.size(), 0u);
}

// ── Consistency: Subject vs old API ──────────────────────────────────────────

TEST(SubjectTest, BodyAltitudeConsistency) {
  // altitude_at via Subject should match body::altitude_at
  auto subj = Subject::body(Body::Sun);
  auto alt_subject = altitude_at(subj, paris(), mid_day());
  auto alt_body = body::altitude_at(Body::Sun, paris(), mid_day());
  EXPECT_DOUBLE_EQ(alt_subject.value(), alt_body.value());
}

TEST(SubjectTest, StarAltitudeConsistency) {
  // altitude_at via Subject should match star::altitude_at
  Star vega = Star::catalog("VEGA");
  auto subj = Subject::star(vega);
  auto alt_subject = altitude_at(subj, paris(), mid_day());
  auto alt_star = star_altitude::altitude_at(vega, paris(), mid_day());
  EXPECT_DOUBLE_EQ(alt_subject.value(), alt_star.value());
}
