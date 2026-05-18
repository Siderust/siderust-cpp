#pragma once

#ifndef SIDERUST_SATELLITE
#  error "dynamics.hpp requires the satellite feature. " \
         "Rebuild siderust-ffi with -DSIDERUST_FFI_FEATURES=satellite " \
         "and configure CMake with that flag so SIDERUST_SATELLITE is defined."
#endif

/**
 * @file dynamics.hpp
 * @brief Orbit propagation wrappers for the siderust dynamics FFI.
 *
 * Exposes DynamicsContext, OrbitState, and TwoBodyPropagator — RAII wrappers
 * over the opaque handles produced by the dynamics section of siderust-ffi.
 *
 * ## C FFI scope (current)
 *
 * The current FFI surface supports:
 * - Two-body propagation (Earth WGS-84 GM or custom GM) via DOP853.
 * - DynamicsContext with optional runtime-ephemeris, atmosphere-vtable, and
 *   gravity-field-vtable providers.
 *
 * ## Gaps vs. Rust API
 *
 * The following Rust force models are not yet exposed through the C FFI; they
 * are available in the Python, Node, and WebAssembly adapters which bind
 * directly to the Rust crate:
 * - J2 first-zonal oblateness
 * - Full spherical-harmonic geopotential (Geopotential)
 * - Cannonball atmospheric drag (DragForce)
 * - Solar radiation pressure with shadow models (CannonballSrp / ShadowModel)
 * - Sun/Moon third-body perturbations (ThirdBody / ThirdBodySunMoon)
 * - 1PN Schwarzschild relativity (CentralBodyRelativity1Pn)
 * - User-supplied RTN empirical acceleration (EmpiricalAcceleration)
 * - Integrator selection (RK4 / DOPRI5 — only DOP853 via C FFI)
 * - Variational equations / state-transition matrix (propagate_with_stm)
 * - Event-driven propagation (propagate_with_events)
 *
 * See docs/dynamics_gaps.md for the cross-adapter gap summary.
 */

#include "ffi_core.hpp"
#include "runtime_ephemeris.hpp"

#include <array>
#include <stdexcept>
#include <string>

namespace siderust {
namespace dynamics {

// ============================================================================
// DynamicsError
// ============================================================================

/// Exception class for dynamics-specific status codes.
class DynamicsError : public SiderustException {
public:
  siderust_dynamics_status_t code;

  explicit DynamicsError(const std::string &msg, siderust_dynamics_status_t c)
      : SiderustException(msg), code(c) {}
};

/// Throw a typed DynamicsError for a non-OK dynamics status code.
inline void check_dynamics(siderust_dynamics_status_t status, const char *op) {
  if (status == SIDERUST_DYNAMICS_STATUS_T_OK)
    return;

  const std::string msg = std::string(op) + " failed: ";
  switch (status) {
  case SIDERUST_DYNAMICS_STATUS_T_NULL_POINTER:
    throw DynamicsError(msg + "null pointer argument", status);
  case SIDERUST_DYNAMICS_STATUS_T_EPHEMERIS_UNAVAILABLE:
    throw DynamicsError(msg + "no ephemeris provider configured in dynamics context", status);
  case SIDERUST_DYNAMICS_STATUS_T_EOP_UNAVAILABLE:
    throw DynamicsError(msg + "Earth Orientation Parameters unavailable for epoch", status);
  case SIDERUST_DYNAMICS_STATUS_T_GRAVITY_COEFFICIENT_UNAVAILABLE:
    throw DynamicsError(msg + "gravity spherical-harmonic coefficient unavailable", status);
  case SIDERUST_DYNAMICS_STATUS_T_ALTITUDE_BELOW_SURFACE:
    throw DynamicsError(msg + "spacecraft altitude below body surface", status);
  case SIDERUST_DYNAMICS_STATUS_T_DEGENERATE_GEOMETRY:
    throw DynamicsError(msg + "degenerate geometry (zero r/v or r ∥ v)", status);
  case SIDERUST_DYNAMICS_STATUS_T_INVALID_STEP_REQUEST:
    throw DynamicsError(msg + "integrator step violated a constraint", status);
  case SIDERUST_DYNAMICS_STATUS_T_ATMOSPHERE_PROVIDER_ERROR:
    throw DynamicsError(msg + "atmosphere density provider returned an error", status);
  case SIDERUST_DYNAMICS_STATUS_T_PROVIDER_ERROR:
    throw DynamicsError(msg + "provider error", status);
  case SIDERUST_DYNAMICS_STATUS_T_GRAVITY_FIELD_UNAVAILABLE:
    throw DynamicsError(msg + "no gravity-field provider configured in dynamics context", status);
  case SIDERUST_DYNAMICS_STATUS_T_GEOPOTENTIAL_DEGREE_OUT_OF_RANGE:
    throw DynamicsError(msg + "geopotential degree/order exceeds provider maximum", status);
  case SIDERUST_DYNAMICS_STATUS_T_INTERNAL_PANIC:
    throw DynamicsError(msg + "internal Rust panic at FFI boundary", status);
  default:
    throw DynamicsError(
        msg + "unknown dynamics error (" + std::to_string(static_cast<int>(status)) + ")", status);
  }
}

// ============================================================================
// OrbitState
// ============================================================================

/// Cartesian Geocentric/GCRS state snapshot: position (km), velocity (km/s),
/// epoch (Julian Date TT).
struct StateVector {
  double x_km{0};    ///< X position component, km.
  double y_km{0};    ///< Y position component, km.
  double z_km{0};    ///< Z position component, km.
  double vx_km_s{0}; ///< X velocity component, km/s.
  double vy_km_s{0}; ///< Y velocity component, km/s.
  double vz_km_s{0}; ///< Z velocity component, km/s.
  double epoch_jd{0}; ///< Epoch as Julian Date (TT).
};

/// RAII owner of an opaque `siderust_orbit_state_t` handle.
///
/// Constructors take epoch (JD TT), Cartesian position (km), and Cartesian
/// velocity (km/s) in the Geocentric/GCRS frame.  Accessors return individual
/// components or a [`StateVector`] value object.
class OrbitState {
  siderust_orbit_state_t *handle_{nullptr};

public:
  /// Construct from individual components.
  OrbitState(double epoch_jd, double x, double y, double z, double vx, double vy, double vz) {
    check_status(siderust_orbit_state_new(epoch_jd, x, y, z, vx, vy, vz, &handle_),
                 "siderust_orbit_state_new");
  }

  /// Construct from a StateVector.
  explicit OrbitState(const StateVector &sv)
      : OrbitState(sv.epoch_jd, sv.x_km, sv.y_km, sv.z_km, sv.vx_km_s, sv.vy_km_s, sv.vz_km_s) {}

  /// Adopt an already-allocated FFI handle (takes ownership; handle may be null).
  explicit OrbitState(siderust_orbit_state_t *handle) noexcept : handle_(handle) {}

  ~OrbitState() {
    if (handle_)
      siderust_orbit_state_free(handle_);
  }

  OrbitState(const OrbitState &) = delete;
  OrbitState &operator=(const OrbitState &) = delete;

  OrbitState(OrbitState &&o) noexcept : handle_(o.handle_) { o.handle_ = nullptr; }

  OrbitState &operator=(OrbitState &&o) noexcept {
    if (this != &o) {
      if (handle_)
        siderust_orbit_state_free(handle_);
      handle_ = o.handle_;
      o.handle_ = nullptr;
    }
    return *this;
  }

  explicit operator bool() const noexcept { return handle_ != nullptr; }

  const siderust_orbit_state_t *raw() const noexcept { return handle_; }

  // ── Accessors ─────────────────────────────────────────────────────────────

  /// Position components [x, y, z] in km (Geocentric/GCRS).
  std::array<double, 3> position() const {
    double x{}, y{}, z{};
    check_status(siderust_orbit_state_position(handle_, &x, &y, &z),
                 "siderust_orbit_state_position");
    return {x, y, z};
  }

  /// Velocity components [vx, vy, vz] in km/s (GCRS).
  std::array<double, 3> velocity() const {
    double vx{}, vy{}, vz{};
    check_status(siderust_orbit_state_velocity(handle_, &vx, &vy, &vz),
                 "siderust_orbit_state_velocity");
    return {vx, vy, vz};
  }

  /// Epoch as a Julian Date (TT).
  double epoch_jd() const {
    double jd{};
    check_status(siderust_orbit_state_epoch_jd(handle_, &jd), "siderust_orbit_state_epoch_jd");
    return jd;
  }

  /// Decode all fields into a StateVector value object.
  StateVector to_vector() const {
    const auto pos = position();
    const auto vel = velocity();
    return {pos[0], pos[1], pos[2], vel[0], vel[1], vel[2], epoch_jd()};
  }
};

// ============================================================================
// DynamicsContext
// ============================================================================

/// RAII owner of an opaque `siderust_dynamics_context_t` handle.
///
/// A default-constructed context carries no providers.  Attach providers with
/// `with_ephemeris()`, `with_atmosphere()`, or `with_gravity_field()`.
///
/// The context is optional: pass `nullptr` to `TwoBodyPropagator::propagate()`
/// when no providers are needed.
class DynamicsContext {
  siderust_dynamics_context_t *handle_{nullptr};

public:
  /// Create an empty dynamics context (no providers attached).
  DynamicsContext() {
    check_status(siderust_dynamics_context_new(&handle_), "siderust_dynamics_context_new");
  }

  /// Adopt an already-allocated FFI handle (takes ownership).
  explicit DynamicsContext(siderust_dynamics_context_t *h) noexcept : handle_(h) {}

  ~DynamicsContext() {
    if (handle_)
      siderust_dynamics_context_free(handle_);
  }

  DynamicsContext(const DynamicsContext &) = delete;
  DynamicsContext &operator=(const DynamicsContext &) = delete;

  DynamicsContext(DynamicsContext &&o) noexcept : handle_(o.handle_) { o.handle_ = nullptr; }

  DynamicsContext &operator=(DynamicsContext &&o) noexcept {
    if (this != &o) {
      if (handle_)
        siderust_dynamics_context_free(handle_);
      handle_ = o.handle_;
      o.handle_ = nullptr;
    }
    return *this;
  }

  explicit operator bool() const noexcept { return handle_ != nullptr; }

  const siderust_dynamics_context_t *raw() const noexcept { return handle_; }

  // ── Provider attachment (fluent API) ──────────────────────────────────────

  /// Attach a runtime ephemeris for third-body perturbations.
  ///
  /// The ephemeris handle must remain alive for the lifetime of this context.
  DynamicsContext &with_ephemeris(const RuntimeEphemeris &eph) {
    check_status(siderust_dynamics_context_with_ephemeris(handle_, eph.raw()),
                 "dynamics_context_with_ephemeris");
    return *this;
  }

  /// Attach a C-callback atmosphere density provider.
  ///
  /// The vtable (and any `user_data` it points to) must remain alive for the
  /// lifetime of this context.
  DynamicsContext &with_atmosphere(const siderust_atmosphere_vtable_t &vtable) {
    check_status(siderust_dynamics_context_with_atmosphere(handle_, &vtable),
                 "dynamics_context_with_atmosphere");
    return *this;
  }

  /// Attach a C-callback gravity-field coefficient provider.
  ///
  /// The vtable (and any `user_data` it points to) must remain alive for the
  /// lifetime of this context.
  DynamicsContext &with_gravity_field(const siderust_gravity_vtable_t &vtable) {
    check_status(siderust_dynamics_context_with_gravity_field(handle_, &vtable),
                 "dynamics_context_with_gravity_field");
    return *this;
  }
};

// ============================================================================
// TwoBodyPropagator
// ============================================================================

/// DOP853 two-body orbit propagator.
///
/// Wraps a `siderust_propagator_t` handle configured with a TwoBody force
/// model (pure Newtonian central gravity).  Propagation is forward or backward
/// in time and returns a new `OrbitState`.
///
/// ## Force model gap
///
/// Only the two-body model is available via C FFI.  For J2, drag, SRP,
/// geopotential, third-body, relativity, empirical RTN, or variational STM,
/// use the Python or JavaScript/WebAssembly adapters which bind directly to
/// the Rust crate.  See docs/dynamics_gaps.md.
class TwoBodyPropagator {
  siderust_propagator_t *handle_{nullptr};

public:
  /// Earth two-body propagator (WGS-84 / EGM2008 GM = 398 600.441 8 km³/s²).
  TwoBodyPropagator() {
    check_status(siderust_propagator_two_body_earth_new(&handle_),
                 "siderust_propagator_two_body_earth_new");
  }

  /// Custom-GM two-body propagator.
  ///
  /// @param gm_km3_s2  Gravitational parameter in km³/s²
  ///                   (e.g. 1.327 124 4e11 for Sun, 4902.8 for Moon).
  explicit TwoBodyPropagator(double gm_km3_s2) {
    check_status(siderust_propagator_two_body_new(gm_km3_s2, &handle_),
                 "siderust_propagator_two_body_new");
  }

  ~TwoBodyPropagator() {
    if (handle_)
      siderust_propagator_free(handle_);
  }

  TwoBodyPropagator(const TwoBodyPropagator &) = delete;
  TwoBodyPropagator &operator=(const TwoBodyPropagator &) = delete;

  TwoBodyPropagator(TwoBodyPropagator &&o) noexcept : handle_(o.handle_) { o.handle_ = nullptr; }

  TwoBodyPropagator &operator=(TwoBodyPropagator &&o) noexcept {
    if (this != &o) {
      if (handle_)
        siderust_propagator_free(handle_);
      handle_ = o.handle_;
      o.handle_ = nullptr;
    }
    return *this;
  }

  explicit operator bool() const noexcept { return handle_ != nullptr; }

  // ── Propagation ───────────────────────────────────────────────────────────

  /// Propagate `state` by `dt_s` seconds (negative for backward propagation).
  ///
  /// @param state   Initial orbit state (Geocentric/GCRS, km / km/s / JD TT).
  /// @param dt_s    Propagation interval in seconds.  Negative = backward.
  /// @param ctx     Optional dynamics context.  Pass `nullptr` for pure two-body.
  ///
  /// @return        Final orbit state.
  /// @throws        DynamicsError on propagation failure.
  OrbitState propagate(const OrbitState &state, double dt_s,
                       const DynamicsContext *ctx = nullptr) const {
    siderust_orbit_state_t *out = nullptr;
    const siderust_dynamics_context_t *raw_ctx = ctx ? ctx->raw() : nullptr;
    check_dynamics(siderust_propagator_propagate(handle_, state.raw(), dt_s, raw_ctx, &out),
                   "siderust_propagator_propagate");
    return OrbitState(out);
  }
};

} // namespace dynamics
} // namespace siderust
