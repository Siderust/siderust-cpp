#pragma once

/**
 * @file astro_context.hpp
 * @brief Thin C++ context for selecting Earth-orientation / nutation models.
 */

#include "ffi_core.hpp"

namespace siderust {

struct Iau2000A {
  static constexpr EarthOrientationModel model_id = EarthOrientationModel::Iau2000A;
};

struct Iau2000B {
  static constexpr EarthOrientationModel model_id = EarthOrientationModel::Iau2000B;
};

struct Iau2006 {
  static constexpr EarthOrientationModel model_id = EarthOrientationModel::Iau2006;
};

struct Iau2006A {
  static constexpr EarthOrientationModel model_id = EarthOrientationModel::Iau2006A;
};

class AstroContext {
  EarthOrientationModel model_ = EarthOrientationModel::Iau2006A;

public:
  constexpr AstroContext() = default;
  constexpr explicit AstroContext(EarthOrientationModel model) : model_(model) {}

  constexpr EarthOrientationModel model() const { return model_; }

  template <typename ModelTag> constexpr AstroContext with_model() const {
    return AstroContext(ModelTag::model_id);
  }

  /// Create an `AstroContext` reflecting the Rust library's built-in default.
  static AstroContext from_default_ffi() {
    siderust_context_t *h = nullptr;
    check_status(siderust_context_create_default(&h), "AstroContext::from_default_ffi");
    siderust_earth_orientation_model_t model_out{};
    auto st = siderust_context_get_model(h, &model_out);
    siderust_context_free(h);
    check_status(st, "AstroContext::from_default_ffi::get_model");
    return AstroContext(static_cast<EarthOrientationModel>(model_out));
  }
};

namespace detail {

/// RAII wrapper around a `siderust_context_t*` created from an
/// `EarthOrientationModel`.  Used by the coordinate transform wrappers
/// to call the new `_with_context` FFI variants.
class OwnedFfiContext {
public:
  /// Create a context using the Rust library's built-in default model.
  OwnedFfiContext() {
    check_status(siderust_context_create_default(&handle_), "AstroContext::create_default");
  }

  explicit OwnedFfiContext(EarthOrientationModel model) {
    check_status(siderust_context_create_with_model(
                     static_cast<siderust_earth_orientation_model_t>(model), &handle_),
                 "AstroContext::create");
  }
  explicit OwnedFfiContext(const AstroContext &ctx) : OwnedFfiContext(ctx.model()) {}

  OwnedFfiContext(const OwnedFfiContext &) = delete;
  OwnedFfiContext &operator=(const OwnedFfiContext &) = delete;

  ~OwnedFfiContext() {
    if (handle_) {
      siderust_context_free(handle_);
    }
  }

  /// Query the Earth-orientation model stored inside this FFI context handle.
  EarthOrientationModel model() const {
    siderust_earth_orientation_model_t out{};
    check_status(siderust_context_get_model(handle_, &out), "OwnedFfiContext::model");
    return static_cast<EarthOrientationModel>(out);
  }

  const siderust_context_t *get() const { return handle_; }

private:
  siderust_context_t *handle_ = nullptr;
};

} // namespace detail

} // namespace siderust
