// SPDX-License-Identifier: AGPL-3.0-or-later
// Copyright (C) 2026 Vallés Puig, Ramon

#include <cmath>
#include <fstream>
#include <sstream>
#include <string>

#include <gtest/gtest.h>
#include <siderust/oem.hpp>

using namespace siderust;

namespace {

// Minimal CCSDS OEM v2 KVN (two states) — self-contained for unit tests.
constexpr const char *kMinimalOem = R"(CCSDS_OEM_VERS = 2.0
CREATION_DATE  = 2024-01-01T00:00:00
ORIGINATOR     = SIDERUST-CPP-TEST

META_START
OBJECT_NAME          = TEST-SAT
OBJECT_ID            = 99999
CENTER_NAME          = EARTH
REF_FRAME            = GCRF
TIME_SYSTEM          = UTC
START_TIME           = 2000-01-01T00:00:00.000
STOP_TIME            = 2000-01-02T00:00:00.000
META_STOP

2000-01-01T00:00:00.000 7000.0 0.0 0.0 0.0 7.5 0.0
2000-01-02T00:00:00.000 7100.0 100.0 0.0 0.0 7.4 0.1
)";

std::string read_lisa_oem1() {
  const std::string path =
      std::string(SIDERUST_SOURCE_DIR) + "/siderust/tests/test-data/lisa/lisa_orbit_sample.oem1";
  std::ifstream in(path);
  if (!in) {
    return {};
  }
  std::ostringstream ss;
  ss << in.rdbuf();
  return ss.str();
}

} // namespace

TEST(Oem, ParseMinimalDocument) {
  const auto states = oem::parse(kMinimalOem);
  ASSERT_EQ(states.size(), 2u);

  EXPECT_NEAR(states[0].pos_km[0], 7000.0, 1e-6);
  EXPECT_NEAR(states[0].pos_km[1], 0.0, 1e-6);
  EXPECT_NEAR(states[0].pos_km[2], 0.0, 1e-6);
  EXPECT_NEAR(states[0].vel_kms[0], 0.0, 1e-6);
  EXPECT_NEAR(states[0].vel_kms[1], 7.5, 1e-6);
  EXPECT_NEAR(states[0].vel_kms[2], 0.0, 1e-6);

  EXPECT_NEAR(states[1].pos_km[0], 7100.0, 1e-6);
  EXPECT_NEAR(states[1].pos_km[1], 100.0, 1e-6);
  EXPECT_GT(states[1].epoch_jd, states[0].epoch_jd);
}

TEST(Oem, ParseLisaSampleFromSubmodule) {
  const std::string doc = read_lisa_oem1();
  if (doc.empty()) {
    GTEST_SKIP() << "LISA OEM sample not available (submodule test-data missing)";
  }

  const auto states = oem::parse(doc);
  ASSERT_EQ(states.size(), 10u);

  EXPECT_NEAR(states[0].pos_km[0], 100'000'000.0, 1.0);
  EXPECT_NEAR(states[0].pos_km[1], 50'000'000.0, 1.0);
  EXPECT_NEAR(states[0].pos_km[2], 10'000'000.0, 1.0);
  EXPECT_NEAR(states[0].vel_kms[0], 10.0, 1e-6);
  EXPECT_NEAR(states[0].vel_kms[1], 20.0, 1e-6);
  EXPECT_NEAR(states[0].vel_kms[2], 5.0, 1e-6);
  EXPECT_GT(states[0].epoch_jd, 2'460'000.0);
}

TEST(Oem, MalformedDocumentThrows) {
  EXPECT_THROW(oem::parse("not an OEM file"), InvalidArgumentError);
  EXPECT_THROW(oem::parse(""), InvalidArgumentError);
}
