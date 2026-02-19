#pragma once

/**
 * @file ffi_core.hpp
 * @brief Error handling and utility base for the siderust C++ wrapper.
 *
 * Maps C-style status codes from siderust-ffi / tempoch-ffi to a typed C++
 * exception hierarchy, and provides RAII helpers for opaque handles.
 */

#include <cstddef>
#include <stdexcept>
#include <string>

extern "C" {
#include "siderust_ffi.h"
#include "tempoch_ffi.h"
}

namespace siderust {

// ============================================================================
// Exception Hierarchy
// ============================================================================

class SiderustException : public std::runtime_error {
public:
    explicit SiderustException(const std::string& msg) : std::runtime_error(msg) {}
};

class NullPointerError : public SiderustException {
public:
    explicit NullPointerError(const std::string& msg) : SiderustException(msg) {}
};

class InvalidFrameError : public SiderustException {
public:
    explicit InvalidFrameError(const std::string& msg) : SiderustException(msg) {}
};

class InvalidCenterError : public SiderustException {
public:
    explicit InvalidCenterError(const std::string& msg) : SiderustException(msg) {}
};

class TransformFailedError : public SiderustException {
public:
    explicit TransformFailedError(const std::string& msg) : SiderustException(msg) {}
};

class InvalidBodyError : public SiderustException {
public:
    explicit InvalidBodyError(const std::string& msg) : SiderustException(msg) {}
};

class UnknownStarError : public SiderustException {
public:
    explicit UnknownStarError(const std::string& msg) : SiderustException(msg) {}
};

class InvalidPeriodError : public SiderustException {
public:
    explicit InvalidPeriodError(const std::string& msg) : SiderustException(msg) {}
};

class AllocationFailedError : public SiderustException {
public:
    explicit AllocationFailedError(const std::string& msg) : SiderustException(msg) {}
};

class InvalidArgumentError : public SiderustException {
public:
    explicit InvalidArgumentError(const std::string& msg) : SiderustException(msg) {}
};

// ============================================================================
// Error Translation
// ============================================================================

inline void check_status(siderust_status_t status, const char* operation) {
    if (status == SIDERUST_STATUS_T_OK) return;

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
        default:
            throw SiderustException(msg + "unknown error (" + std::to_string(status) + ")");
    }
}

inline void check_tempoch_status(tempoch_status_t status, const char* operation) {
    if (status == TEMPOCH_STATUS_T_OK) return;

    std::string msg = std::string(operation) + " failed: ";
    switch (status) {
        case TEMPOCH_STATUS_T_NULL_POINTER:
            throw NullPointerError(msg + "null output pointer");
        case TEMPOCH_STATUS_T_UTC_CONVERSION_FAILED:
            throw SiderustException(msg + "UTC conversion failed");
        case TEMPOCH_STATUS_T_INVALID_PERIOD:
            throw InvalidPeriodError(msg + "invalid period");
        case TEMPOCH_STATUS_T_NO_INTERSECTION:
            throw SiderustException(msg + "periods do not intersect");
        default:
            throw SiderustException(msg + "unknown tempoch error (" + std::to_string(status) + ")");
    }
}

// ============================================================================
// Frame and Center Enums (C++ typed)
// ============================================================================

enum class Frame : int32_t {
    ICRS                   = SIDERUST_FRAME_T_ICRS,
    EclipticMeanJ2000      = SIDERUST_FRAME_T_ECLIPTIC_MEAN_J2000,
    EquatorialMeanJ2000    = SIDERUST_FRAME_T_EQUATORIAL_MEAN_J2000,
    EquatorialMeanOfDate   = SIDERUST_FRAME_T_EQUATORIAL_MEAN_OF_DATE,
    EquatorialTrueOfDate   = SIDERUST_FRAME_T_EQUATORIAL_TRUE_OF_DATE,
    Horizontal             = SIDERUST_FRAME_T_HORIZONTAL,
    ECEF                   = SIDERUST_FRAME_T_ECEF,
    Galactic               = SIDERUST_FRAME_T_GALACTIC,
    GCRS                   = SIDERUST_FRAME_T_GCRS,
    EclipticOfDate         = SIDERUST_FRAME_T_ECLIPTIC_OF_DATE,
    EclipticTrueOfDate     = SIDERUST_FRAME_T_ECLIPTIC_TRUE_OF_DATE,
    CIRS                   = SIDERUST_FRAME_T_CIRS,
    TIRS                   = SIDERUST_FRAME_T_TIRS,
    ITRF                   = SIDERUST_FRAME_T_ITRF,
    ICRF                   = SIDERUST_FRAME_T_ICRF,
};

enum class Center : int32_t {
    Barycentric  = SIDERUST_CENTER_T_BARYCENTRIC,
    Heliocentric = SIDERUST_CENTER_T_HELIOCENTRIC,
    Geocentric   = SIDERUST_CENTER_T_GEOCENTRIC,
    Topocentric  = SIDERUST_CENTER_T_TOPOCENTRIC,
    Bodycentric  = SIDERUST_CENTER_T_BODYCENTRIC,
};

enum class CrossingDirection : int32_t {
    Rising  = SIDERUST_CROSSING_DIRECTION_T_RISING,
    Setting = SIDERUST_CROSSING_DIRECTION_T_SETTING,
};

enum class CulminationKind : int32_t {
    Max = SIDERUST_CULMINATION_KIND_T_MAX,
    Min = SIDERUST_CULMINATION_KIND_T_MIN,
};

enum class RaConvention : int32_t {
    MuAlpha     = SIDERUST_RA_CONVENTION_T_MU_ALPHA,
    MuAlphaStar = SIDERUST_RA_CONVENTION_T_MU_ALPHA_STAR,
};

} // namespace siderust
