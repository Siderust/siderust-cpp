#pragma once

/**
 * @file ffi_core.hpp
 * @brief Error handling and utility base for the siderust C++ wrapper.
 *
 * Maps C-style status codes from siderust-ffi / tempoch-ffi to a typed C++
 * exception hierarchy, and provides RAII helpers for opaque handles.
 */

#include <cstddef>
#include <ostream>
#include <stdexcept>
#include <string>

#include <tempoch/ffi_core.hpp>

#ifdef __cplusplus
using QttyQuantity = qtty_quantity_t;
#endif

extern "C" {
#include "siderust_ffi.h"
}

namespace siderust {

// ============================================================================
// Exception Hierarchy
// ============================================================================

class SiderustException : public std::runtime_error {
public:
  explicit SiderustException(const std::string &msg)
      : std::runtime_error(msg) {}
};

class NullPointerError : public SiderustException {
public:
  explicit NullPointerError(const std::string &msg) : SiderustException(msg) {}
};

class InvalidFrameError : public SiderustException {
public:
  explicit InvalidFrameError(const std::string &msg) : SiderustException(msg) {}
};

class InvalidCenterError : public SiderustException {
public:
  explicit InvalidCenterError(const std::string &msg)
      : SiderustException(msg) {}
};

class TransformFailedError : public SiderustException {
public:
  explicit TransformFailedError(const std::string &msg)
      : SiderustException(msg) {}
};

class InvalidBodyError : public SiderustException {
public:
  explicit InvalidBodyError(const std::string &msg) : SiderustException(msg) {}
};

class UnknownStarError : public SiderustException {
public:
  explicit UnknownStarError(const std::string &msg) : SiderustException(msg) {}
};

class InvalidPeriodError : public SiderustException {
public:
  explicit InvalidPeriodError(const std::string &msg)
      : SiderustException(msg) {}
};

class AllocationFailedError : public SiderustException {
public:
  explicit AllocationFailedError(const std::string &msg)
      : SiderustException(msg) {}
};

class InvalidArgumentError : public SiderustException {
public:
  explicit InvalidArgumentError(const std::string &msg)
      : SiderustException(msg) {}
};

class InternalPanicError : public SiderustException {
public:
  explicit InternalPanicError(const std::string &msg)
      : SiderustException(msg) {}
};

class DataLoadError : public SiderustException {
public:
  explicit DataLoadError(const std::string &msg) : SiderustException(msg) {}
};

class OutOfRangeError : public SiderustException {
public:
  explicit OutOfRangeError(const std::string &msg) : SiderustException(msg) {}
};

class NoEopDataError : public SiderustException {
public:
  explicit NoEopDataError(const std::string &msg) : SiderustException(msg) {}
};

// ============================================================================
// Error Translation
// ============================================================================

inline void check_status(siderust_status_t status, const char *operation) {
  if (status == SIDERUST_STATUS_T_OK)
    return;

  std::string msg = std::string(operation) + " failed: ";
  switch (status) {
  case SIDERUST_STATUS_T_NULL_POINTER:
    throw NullPointerError(msg + "null output pointer");
  case SIDERUST_STATUS_T_INVALID_FRAME:
    throw InvalidFrameError(msg + "invalid or unsupported frame");
  case SIDERUST_STATUS_T_INVALID_CENTER:
    throw InvalidCenterError(msg + "invalid or unsupported center");
  case SIDERUST_STATUS_T_TRANSFORM_FAILED:
    throw TransformFailedError(msg + "coordinate transform failed");
  case SIDERUST_STATUS_T_INVALID_BODY:
    throw InvalidBodyError(msg + "invalid body");
  case SIDERUST_STATUS_T_UNKNOWN_STAR:
    throw UnknownStarError(msg + "unknown star name");
  case SIDERUST_STATUS_T_INVALID_PERIOD:
    throw InvalidPeriodError(msg + "invalid period (start > end)");
  case SIDERUST_STATUS_T_ALLOCATION_FAILED:
    throw AllocationFailedError(msg + "memory allocation failed");
  case SIDERUST_STATUS_T_INVALID_ARGUMENT:
    throw InvalidArgumentError(msg + "invalid argument");
  case SIDERUST_STATUS_T_INTERNAL_PANIC:
    throw InternalPanicError(msg + "internal panic in Rust FFI");
  case SIDERUST_STATUS_T_DATA_ERROR:
    throw DataLoadError(msg + "data loading error (I/O, download, or parse)");
  case SIDERUST_STATUS_T_OUT_OF_RANGE:
    throw OutOfRangeError(msg + "epoch outside covered data range");
  case SIDERUST_STATUS_T_NO_EOP_DATA:
    throw NoEopDataError(msg + "Earth Orientation Parameters unavailable for epoch");
  default:
    throw SiderustException(msg + "unknown error (" + std::to_string(status) +
                            ")");
  }
}

/// @brief Backward-compatible wrapper — delegates to tempoch::check_status.
inline void check_tempoch_status(tempoch_status_t status,
                                 const char *operation) {
  tempoch::check_status(status, operation);
}

// ============================================================================
// FFI version
// ============================================================================

/**
 * @brief Returns the siderust-ffi ABI version (major*10000 + minor*100 +
 * patch).
 */
inline uint32_t ffi_version() { return siderust_ffi_version(); }

// ============================================================================
// Frame and Center Enums (C++ typed)
// ============================================================================

enum class Frame : int32_t {
  ICRS = SIDERUST_FRAME_T_ICRS,
  EclipticMeanJ2000 = SIDERUST_FRAME_T_ECLIPTIC_MEAN_J2000,
  EquatorialMeanJ2000 = SIDERUST_FRAME_T_EQUATORIAL_MEAN_J2000,
  EquatorialMeanOfDate = SIDERUST_FRAME_T_EQUATORIAL_MEAN_OF_DATE,
  EquatorialTrueOfDate = SIDERUST_FRAME_T_EQUATORIAL_TRUE_OF_DATE,
  Horizontal = SIDERUST_FRAME_T_HORIZONTAL,
  ECEF = SIDERUST_FRAME_T_ECEF,
  Galactic = SIDERUST_FRAME_T_GALACTIC,
  GCRS = SIDERUST_FRAME_T_GCRS,
  EclipticOfDate = SIDERUST_FRAME_T_ECLIPTIC_OF_DATE,
  EclipticTrueOfDate = SIDERUST_FRAME_T_ECLIPTIC_TRUE_OF_DATE,
  CIRS = SIDERUST_FRAME_T_CIRS,
  TIRS = SIDERUST_FRAME_T_TIRS,
  ITRF = SIDERUST_FRAME_T_ITRF,
  ICRF = SIDERUST_FRAME_T_ICRF,
};

enum class Center : int32_t {
  Barycentric = SIDERUST_CENTER_T_BARYCENTRIC,
  Heliocentric = SIDERUST_CENTER_T_HELIOCENTRIC,
  Geocentric = SIDERUST_CENTER_T_GEOCENTRIC,
  Topocentric = SIDERUST_CENTER_T_TOPOCENTRIC,
  Bodycentric = SIDERUST_CENTER_T_BODYCENTRIC,
};

enum class EarthOrientationModel : int32_t {
  Iau2000A = SIDERUST_EARTH_ORIENTATION_MODEL_T_IAU2000_A,
  Iau2000B = SIDERUST_EARTH_ORIENTATION_MODEL_T_IAU2000_B,
  Iau2006 = SIDERUST_EARTH_ORIENTATION_MODEL_T_IAU2006,
  Iau2006A = SIDERUST_EARTH_ORIENTATION_MODEL_T_IAU2006_A,
};

enum class CrossingDirection : int32_t {
  Rising = SIDERUST_CROSSING_DIRECTION_T_RISING,
  Setting = SIDERUST_CROSSING_DIRECTION_T_SETTING,
};

enum class CulminationKind : int32_t {
  Max = SIDERUST_CULMINATION_KIND_T_MAX,
  Min = SIDERUST_CULMINATION_KIND_T_MIN,
};

// ============================================================================
// Stream operators for enums
// ============================================================================

inline std::ostream &operator<<(std::ostream &os, CrossingDirection dir) {
  switch (dir) {
  case CrossingDirection::Rising:
    return os << "rising";
  case CrossingDirection::Setting:
    return os << "setting";
  }
  return os << "unknown";
}

inline std::ostream &operator<<(std::ostream &os, CulminationKind kind) {
  switch (kind) {
  case CulminationKind::Max:
    return os << "max";
  case CulminationKind::Min:
    return os << "min";
  }
  return os << "unknown";
}

inline std::ostream &operator<<(std::ostream &os, EarthOrientationModel model) {
  switch (model) {
  case EarthOrientationModel::Iau2000A:
    return os << "Iau2000A";
  case EarthOrientationModel::Iau2000B:
    return os << "Iau2000B";
  case EarthOrientationModel::Iau2006:
    return os << "Iau2006";
  case EarthOrientationModel::Iau2006A:
    return os << "Iau2006A";
  }
  return os << "Unknown";
}

enum class RaConvention : int32_t {
  MuAlpha = SIDERUST_RA_CONVENTION_T_MU_ALPHA,
  MuAlphaStar = SIDERUST_RA_CONVENTION_T_MU_ALPHA_STAR,
};

namespace detail {

/// Build a `siderust_subject_t` for a solar-system body.
inline siderust_subject_t make_body_subject(SiderustBody b) {
  siderust_subject_t s{};
  s.kind = SIDERUST_SUBJECT_KIND_T_BODY;
  s.body = b;
  return s;
}

/// Build a `siderust_subject_t` for a star, borrowing the handle.
inline siderust_subject_t make_star_subject(const SiderustStar *h) {
  siderust_subject_t s{};
  s.kind = SIDERUST_SUBJECT_KIND_T_STAR;
  s.star_handle = h;
  return s;
}

/// Build a `siderust_subject_t` for a fixed ICRS direction.
inline siderust_subject_t
make_icrs_subject(const siderust_spherical_dir_t &dir) {
  siderust_subject_t s{};
  s.kind = SIDERUST_SUBJECT_KIND_T_ICRS;
  s.icrs_dir = dir;
  return s;
}

/// Build a `siderust_subject_t` for a generic target opaque handle.
inline siderust_subject_t
make_generic_target_subject(const SiderustGenericTarget *h) {
  siderust_subject_t s{};
  s.kind = SIDERUST_SUBJECT_KIND_T_GENERIC_TARGET;
  s.generic_target_handle = h;
  return s;
}

} // namespace detail

} // namespace siderust
