#include <gtest/gtest.h>
#include <siderust/siderust.hpp>

using namespace siderust;

// ============================================================================
// AstroContext — default construction and model query
// ============================================================================

TEST(AstroContext, DefaultCppConstructor) {
  // The C++ default-constructed AstroContext uses IAU 2006A.
  AstroContext ctx;
  EXPECT_EQ(ctx.model(), EarthOrientationModel::Iau2006A);
}

TEST(AstroContext, ExplicitModelConstructor) {
  AstroContext ctx(EarthOrientationModel::Iau2000B);
  EXPECT_EQ(ctx.model(), EarthOrientationModel::Iau2000B);
}

TEST(AstroContext, FromDefaultFfi) {
  // from_default_ffi() round-trips through the Rust library's own default.
  // We just check that it returns without throwing and yields a valid model.
  auto ctx = AstroContext::from_default_ffi();
  // The Rust default must be one of the known models.
  auto m = static_cast<int32_t>(ctx.model());
  EXPECT_GE(m, 0);
  EXPECT_LE(m, 3);
}

TEST(AstroContext, OwnedFfiContextDefaultModel) {
  // OwnedFfiContext default constructor should call siderust_context_create_default.
  detail::OwnedFfiContext fctx;
  EXPECT_NE(fctx.get(), nullptr);
  // model() should return a valid enum value.
  auto m = static_cast<int32_t>(fctx.model());
  EXPECT_GE(m, 0);
  EXPECT_LE(m, 3);
}

TEST(AstroContext, OwnedFfiContextModelRoundtrip) {
  // Context created with IAU 2000A should report IAU 2000A back.
  detail::OwnedFfiContext fctx(EarthOrientationModel::Iau2000A);
  EXPECT_EQ(fctx.model(), EarthOrientationModel::Iau2000A);
}
