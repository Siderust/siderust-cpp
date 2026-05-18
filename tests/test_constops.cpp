// SPDX-License-Identifier: AGPL-3.0-or-later

#include <gtest/gtest.h>
#include <siderust/constops.hpp>

using namespace siderust::constops;

TEST(ConstopsGroundAssetId, FormatAndParseRoundTrip) {
  const auto key = format_ground_asset_id("aws", "bochum");
  EXPECT_EQ(key, "aws:bochum");

  const auto parts = parse_ground_asset_id(key);
  EXPECT_EQ(parts.first, "aws");
  EXPECT_EQ(parts.second, "bochum");
}

TEST(ConstopsGroundAssetId, ParseRejectsMalformedValues) {
  EXPECT_THROW(parse_ground_asset_id(""), ConstopsError);
  EXPECT_THROW(parse_ground_asset_id("aws"), ConstopsError);
  EXPECT_THROW(parse_ground_asset_id(":bochum"), ConstopsError);
  EXPECT_THROW(parse_ground_asset_id("aws:"), ConstopsError);
}

TEST(ConstopsRoutes, UseSlashRoutes) {
  EXPECT_STREQ(kRoutePassesGenerate, "/v1/passes/generate");
  EXPECT_STREQ(kRouteLinksScore, "/v1/links/score");
  EXPECT_STREQ(kRouteSchedulesSolve, "/v1/schedules/solve");
  EXPECT_STREQ(kRouteSchedulesValidate, "/v1/schedules/validate");
  EXPECT_STREQ(kRouteOrbitStatesIngest, "/v1/orbit-states/ingest");
}
