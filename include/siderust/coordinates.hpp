#pragma once

/**
 * @file coordinates.hpp
 * @brief Spherical directions, Cartesian positions, and geodetic coordinates.
 *
 * Provides typed C++ wrappers for coordinate transformations exposed by
 * siderust-ffi, including frame-to-frame and to-horizontal conversions.
 */

#include "ffi_core.hpp"

namespace siderust {

// ============================================================================
// Geodetic (WGS84)
// ============================================================================

/**
 * @brief Geodetic position (WGS84 ellipsoid).
 */
struct Geodetic {
    double lon_deg;    ///< Longitude (east positive) in degrees.
    double lat_deg;    ///< Latitude (north positive) in degrees.
    double height_m;   ///< Height above ellipsoid in metres.

    Geodetic() : lon_deg(0), lat_deg(0), height_m(0) {}

    Geodetic(double lon, double lat, double h = 0.0)
        : lon_deg(lon), lat_deg(lat), height_m(h) {}

    /// Convert to the C FFI struct.
    siderust_geodetic_t to_c() const {
        return {lon_deg, lat_deg, height_m};
    }

    /// Create from the C FFI struct.
    static Geodetic from_c(const siderust_geodetic_t& c) {
        return Geodetic(c.lon_deg, c.lat_deg, c.height_m);
    }
};

// ============================================================================
// SphericalDirection
// ============================================================================

/**
 * @brief A direction on the celestial sphere (lon/lat or RA/Dec) with frame tag.
 */
struct SphericalDirection {
    double lon_deg;  ///< Longitude / RA in degrees.
    double lat_deg;  ///< Latitude / Dec in degrees.
    Frame  frame;    ///< Reference frame.

    SphericalDirection() : lon_deg(0), lat_deg(0), frame(Frame::ICRS) {}

    SphericalDirection(double lon, double lat, Frame f)
        : lon_deg(lon), lat_deg(lat), frame(f) {}

    /// Create from the C FFI struct.
    static SphericalDirection from_c(const siderust_spherical_dir_t& c) {
        return SphericalDirection(c.lon_deg, c.lat_deg, static_cast<Frame>(c.frame));
    }

    /// Convert to the C FFI struct.
    siderust_spherical_dir_t to_c() const {
        return {lon_deg, lat_deg, static_cast<siderust_frame_t>(frame)};
    }

    // -- Convenience aliases for ICRS users --

    /// RA alias for lon_deg (ICRS convention).
    double ra_deg()  const { return lon_deg; }
    /// Dec alias for lat_deg (ICRS convention).
    double dec_deg() const { return lat_deg; }

    // -- Convenience aliases for Horizontal users --

    /// Azimuth alias for lon_deg (Horizontal convention).
    double azimuth_deg()  const { return lon_deg; }
    /// Altitude alias for lat_deg (Horizontal convention).
    double altitude_deg() const { return lat_deg; }

    // ========================================================================
    // Frame transforms
    // ========================================================================

    /**
     * @brief Transform this direction to a different reference frame.
     *
     * Supported target frames: ICRS, EclipticMeanJ2000, EquatorialMeanJ2000,
     * EquatorialMeanOfDate, EquatorialTrueOfDate.
     *
     * @param target Target frame.
     * @param jd     Julian Date (required for time-dependent frames).
     * @return SphericalDirection in the target frame.
     */
    SphericalDirection transform(Frame target, double jd) const {
        siderust_spherical_dir_t out;
        check_status(
            siderust_spherical_dir_transform_frame(
                lon_deg, lat_deg,
                static_cast<siderust_frame_t>(frame),
                static_cast<siderust_frame_t>(target),
                jd, &out),
            "SphericalDirection::transform"
        );
        return from_c(out);
    }

    /**
     * @brief Transform this direction to the horizontal (alt-az) frame.
     *
     * @param observer Geodetic location of the observer.
     * @param jd       Julian Date of the observation.
     * @return SphericalDirection in the Horizontal frame (lon=azimuth, lat=altitude).
     */
    SphericalDirection to_horizontal(const Geodetic& observer, double jd) const {
        siderust_spherical_dir_t out;
        check_status(
            siderust_spherical_dir_to_horizontal(
                lon_deg, lat_deg,
                static_cast<siderust_frame_t>(frame),
                jd, observer.to_c(), &out),
            "SphericalDirection::to_horizontal"
        );
        return from_c(out);
    }
};

// ============================================================================
// CartesianPosition
// ============================================================================

/**
 * @brief A 3D Cartesian position with frame and center tags.
 */
struct CartesianPosition {
    double x;
    double y;
    double z;
    Frame  frame;
    Center center;

    CartesianPosition()
        : x(0), y(0), z(0),
          frame(Frame::ICRS), center(Center::Barycentric) {}

    CartesianPosition(double x_, double y_, double z_, Frame f, Center c)
        : x(x_), y(y_), z(z_), frame(f), center(c) {}

    /// Create from the C FFI struct.
    static CartesianPosition from_c(const siderust_cartesian_pos_t& c) {
        return CartesianPosition(
            c.x, c.y, c.z,
            static_cast<Frame>(c.frame),
            static_cast<Center>(c.center));
    }

    /// Convert to the C FFI struct.
    siderust_cartesian_pos_t to_c() const {
        return {x, y, z,
                static_cast<siderust_frame_t>(frame),
                static_cast<siderust_center_t>(center)};
    }
};

// ============================================================================
// Free functions
// ============================================================================

/**
 * @brief Convert a geodetic position to ECEF Cartesian.
 */
inline CartesianPosition geodetic_to_cartesian_ecef(const Geodetic& geo) {
    siderust_cartesian_pos_t out;
    check_status(
        siderust_geodetic_to_cartesian_ecef(geo.to_c(), &out),
        "geodetic_to_cartesian_ecef"
    );
    return CartesianPosition::from_c(out);
}

} // namespace siderust
