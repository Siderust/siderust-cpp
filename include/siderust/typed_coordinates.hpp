#pragma once

/**
 * @file typed_coordinates.hpp
 * @brief Compile-time typed coordinate types mirroring Rust's affn crate.
 *
 * Provides:
 * - `spherical::Direction<Frame>`          — direction on the celestial sphere
 * - `spherical::Position<Center,Frame,U>`  — spherical position with distance
 * - `cartesian::Direction<Frame>`          — unit-vector direction
 * - `cartesian::Position<Center,Frame,U>`  — 3D affine point
 * - `Geodetic`                             — WGS84 ellipsoidal position
 *
 * All types integrate with `qtty::Quantity` for unit-safe angles and lengths.
 * Frame/center tags provide compile-time transform validity checking via the
 * trait predicates in frames.hpp / centers.hpp.
 */

#include "ffi_core.hpp"
#include "frames.hpp"
#include "centers.hpp"
#include "time.hpp"

#include <qtty/qtty.hpp>

#include <type_traits>

namespace siderust {

// ============================================================================
// Geodetic (WGS84) — always ECEF + Geocentric
// ============================================================================

/**
 * @brief Geodetic position (WGS84 ellipsoid).
 *
 * Fixed to frame=ECEF, center=Geocentric.  Uses qtty quantities.
 */
struct Geodetic {
    qtty::Degree lon;    ///< Longitude (east positive).
    qtty::Degree lat;    ///< Latitude (north positive).
    qtty::Meter  height; ///< Height above ellipsoid.

    Geodetic()
        : lon(qtty::Degree(0)), lat(qtty::Degree(0)), height(qtty::Meter(0)) {}

    Geodetic(qtty::Degree lon_, qtty::Degree lat_, qtty::Meter h = qtty::Meter(0))
        : lon(lon_), lat(lat_), height(h) {}

    /// Raw-double convenience constructor (degrees, metres).
    Geodetic(double lon_deg, double lat_deg, double height_m = 0.0)
        : lon(qtty::Degree(lon_deg)), lat(qtty::Degree(lat_deg)),
          height(qtty::Meter(height_m)) {}

    /// Convert to C FFI struct.
    siderust_geodetic_t to_c() const {
        return {lon.value(), lat.value(), height.value()};
    }

    /// Create from C FFI struct.
    static Geodetic from_c(const siderust_geodetic_t& c) {
        return Geodetic(c.lon_deg, c.lat_deg, c.height_m);
    }

    // Legacy accessors (raw double)
    double lon_deg()  const { return lon.value(); }
    double lat_deg()  const { return lat.value(); }
    double height_m() const { return height.value(); }
};

// ============================================================================
// Spherical Coordinates
// ============================================================================

namespace spherical {

/**
 * @brief A direction on the celestial sphere, compile-time tagged by frame.
 *
 * Mirrors Rust's `affn::spherical::Direction<F>`.
 *
 * @tparam F  Reference frame tag (e.g. `frames::ICRS`).
 */
template<typename F>
struct Direction {
    static_assert(frames::is_frame_v<F>, "F must be a valid frame tag");

    qtty::Degree lon;  ///< Azimuthal angle (RA, longitude, azimuth …).
    qtty::Degree lat;  ///< Polar angle (Dec, latitude, altitude …).

    Direction() : lon(qtty::Degree(0)), lat(qtty::Degree(0)) {}

    Direction(qtty::Degree lon_, qtty::Degree lat_)
        : lon(lon_), lat(lat_) {}

    /// Raw-double convenience (degrees).
    Direction(double lon_deg, double lat_deg)
        : lon(qtty::Degree(lon_deg)), lat(qtty::Degree(lat_deg)) {}

    // -- Frame info --
    static constexpr siderust_frame_t frame_id() {
        return frames::FrameTraits<F>::ffi_id;
    }
    static constexpr const char* frame_name() {
        return frames::FrameTraits<F>::name();
    }

    // -- Named accessors (use SphericalNaming) --

    /// Right ascension / longitude / azimuth (frame-dependent name).
    qtty::Degree azimuthal()  const { return lon; }
    /// Declination / latitude / altitude (frame-dependent name).
    qtty::Degree polar()      const { return lat; }

    // Convenience double accessors
    double lon_deg() const { return lon.value(); }
    double lat_deg() const { return lat.value(); }

    // ================================================================
    // ICRS aliases
    // ================================================================
    double ra_deg()  const { return lon.value(); }
    double dec_deg() const { return lat.value(); }

    // ================================================================
    // Horizontal aliases
    // ================================================================
    double azimuth_deg()  const { return lon.value(); }
    double altitude_deg() const { return lat.value(); }

    // ================================================================
    // FFI conversion
    // ================================================================

    siderust_spherical_dir_t to_c() const {
        return {lon.value(), lat.value(), frame_id()};
    }

    static Direction from_c(const siderust_spherical_dir_t& c) {
        return Direction(c.lon_deg, c.lat_deg);
    }

    // ================================================================
    // Frame transforms
    // ================================================================

    /**
     * @brief Transform to a different reference frame.
     *
     * Only enabled for frame pairs with a FrameRotationProvider in the FFI.
     * Attempting an unsupported transform is a **compile-time error**.
     *
     * @tparam Target  Target frame tag.
     * @param  jd      Julian Date (needed for time-dependent frames).
     * @return Direction<Target>
     */
    template<typename Target>
    std::enable_if_t<
        frames::has_frame_transform_v<F, Target>,
        Direction<Target>
    >
    to_frame(const JulianDate& jd) const {
        // Same-frame shortcut
        if constexpr (std::is_same_v<F, Target>) {
            return Direction<Target>(lon.value(), lat.value());
        } else {
            siderust_spherical_dir_t out;
            check_status(
                siderust_spherical_dir_transform_frame(
                    lon.value(), lat.value(),
                    frames::FrameTraits<F>::ffi_id,
                    frames::FrameTraits<Target>::ffi_id,
                    jd.value(), &out),
                "Direction::to_frame"
            );
            return Direction<Target>::from_c(out);
        }
    }

    /**
     * @brief Shorthand: `.to<Target>(jd)` (calls `to_frame`).
     */
    template<typename Target>
    auto to(const JulianDate& jd) const
        -> decltype(this->template to_frame<Target>(jd))
    {
        return to_frame<Target>(jd);
    }

    /**
     * @brief Transform to the horizontal (alt-az) frame.
     *
     * Only enabled for frames with a horizontal-transform path.
     *
     * @param jd       Julian Date.
     * @param observer Geodetic observer location.
     * @return Direction<frames::Horizontal>
     */
    template<typename F_ = F>
    std::enable_if_t<
        frames::has_horizontal_transform_v<F_>,
        Direction<frames::Horizontal>
    >
    to_horizontal(const JulianDate& jd, const Geodetic& observer) const {
        siderust_spherical_dir_t out;
        check_status(
            siderust_spherical_dir_to_horizontal(
                lon.value(), lat.value(),
                frames::FrameTraits<F>::ffi_id,
                jd.value(), observer.to_c(), &out),
            "Direction::to_horizontal"
        );
        return Direction<frames::Horizontal>::from_c(out);
    }
};

/**
 * @brief A spherical position (direction + distance), compile-time tagged.
 *
 * Mirrors Rust's `affn::spherical::Position<C, F, U>`.
 *
 * @tparam C  Center tag (e.g. `centers::Barycentric`).
 * @tparam F  Frame tag  (e.g. `frames::ICRS`).
 * @tparam U  Distance unit (default: `qtty::Meter`).
 */
template<typename C, typename F, typename U = qtty::Meter>
struct Position {
    static_assert(frames::is_frame_v<F>,   "F must be a valid frame tag");
    static_assert(centers::is_center_v<C>, "C must be a valid center tag");

    qtty::Degree lon;  ///< Azimuthal angle.
    qtty::Degree lat;  ///< Polar angle.
    U            dist; ///< Radial distance.

    Position()
        : lon(qtty::Degree(0)), lat(qtty::Degree(0)), dist(U(0)) {}

    Position(qtty::Degree lon_, qtty::Degree lat_, U dist_)
        : lon(lon_), lat(lat_), dist(dist_) {}

    Position(double lon_deg, double lat_deg, double dist_val)
        : lon(qtty::Degree(lon_deg)), lat(qtty::Degree(lat_deg)), dist(U(dist_val)) {}

    /// Extract the direction component.
    Direction<F> direction() const {
        return Direction<F>(lon, lat);
    }

    // Frame/center info
    static constexpr siderust_frame_t  frame_id()  { return frames::FrameTraits<F>::ffi_id; }
    static constexpr siderust_center_t center_id() { return centers::CenterTraits<C>::ffi_id; }

    // Convenience double accessors
    double lon_deg()    const { return lon.value(); }
    double lat_deg()    const { return lat.value(); }
    double distance()   const { return dist.value(); }
};

} // namespace spherical

// ============================================================================
// Cartesian Coordinates
// ============================================================================

namespace cartesian {

/**
 * @brief A unit-vector direction in Cartesian form, compile-time frame-tagged.
 *
 * Mirrors Rust's `affn::cartesian::Direction<F>`.
 *
 * @tparam F  Reference frame tag.
 */
template<typename F>
struct Direction {
    static_assert(frames::is_frame_v<F>, "F must be a valid frame tag");

    double x;
    double y;
    double z;

    Direction() : x(0), y(0), z(0) {}
    Direction(double x_, double y_, double z_) : x(x_), y(y_), z(z_) {}

    static constexpr siderust_frame_t frame_id() {
        return frames::FrameTraits<F>::ffi_id;
    }
};

/**
 * @brief A 3D Cartesian position, compile-time tagged by center, frame, unit.
 *
 * Mirrors Rust's `affn::cartesian::Position<C, F, U>`.
 *
 * @tparam C  Center tag.
 * @tparam F  Frame tag.
 * @tparam U  Length unit (default: `qtty::Meter`).
 */
template<typename C, typename F, typename U = qtty::Meter>
struct Position {
    static_assert(frames::is_frame_v<F>,   "F must be a valid frame tag");
    static_assert(centers::is_center_v<C>, "C must be a valid center tag");

    U comp_x;
    U comp_y;
    U comp_z;

    Position()
        : comp_x(U(0)), comp_y(U(0)), comp_z(U(0)) {}

    Position(U x_, U y_, U z_)
        : comp_x(x_), comp_y(y_), comp_z(z_) {}

    Position(double x_, double y_, double z_)
        : comp_x(U(x_)), comp_y(U(y_)), comp_z(U(z_)) {}

    // Component accessors (raw double)
    double x() const { return comp_x.value(); }
    double y() const { return comp_y.value(); }
    double z() const { return comp_z.value(); }

    static constexpr siderust_frame_t  frame_id()  { return frames::FrameTraits<F>::ffi_id; }
    static constexpr siderust_center_t center_id() { return centers::CenterTraits<C>::ffi_id; }

    /// Convert to C FFI struct.
    siderust_cartesian_pos_t to_c() const {
        return {comp_x.value(), comp_y.value(), comp_z.value(),
                frame_id(), center_id()};
    }

    /// Create from C FFI struct (ignoring runtime frame/center — trust the type).
    static Position from_c(const siderust_cartesian_pos_t& c) {
        return Position(c.x, c.y, c.z);
    }
};

} // namespace cartesian

// ============================================================================
// Type Aliases (match Rust's types.rs)
// ============================================================================

// -- Spherical Directions --
using IcrsDir                = spherical::Direction<frames::ICRS>;
using IcrfDir                = spherical::Direction<frames::ICRF>;
using EclipticDir            = spherical::Direction<frames::EclipticMeanJ2000>;
using EquatorialJ2000Dir     = spherical::Direction<frames::EquatorialMeanJ2000>;
using EquatorialMeanOfDateDir = spherical::Direction<frames::EquatorialMeanOfDate>;
using EquatorialTrueOfDateDir = spherical::Direction<frames::EquatorialTrueOfDate>;
using HorizontalDir          = spherical::Direction<frames::Horizontal>;
using GalacticDir            = spherical::Direction<frames::Galactic>;

// -- Spherical Positions --
using IcrsPos      = spherical::Position<centers::Barycentric,  frames::ICRS>;
using GcrsPos      = spherical::Position<centers::Geocentric,   frames::ICRS>;
using HcrsPos      = spherical::Position<centers::Heliocentric, frames::ICRS>;
using EclipticPos  = spherical::Position<centers::Heliocentric, frames::EclipticMeanJ2000>;
using HorizontalPos = spherical::Position<centers::Topocentric, frames::Horizontal>;

// -- Cartesian Directions --
using IcrsCartDir    = cartesian::Direction<frames::ICRS>;
using EclipticCartDir = cartesian::Direction<frames::EclipticMeanJ2000>;

// -- Cartesian Positions (Meter) --
using IcrsCartPos     = cartesian::Position<centers::Barycentric,  frames::ICRS>;
using GcrsCartPos     = cartesian::Position<centers::Geocentric,   frames::ICRS>;
using EclipticCartPos = cartesian::Position<centers::Heliocentric, frames::EclipticMeanJ2000>;
using EcefCartPos     = cartesian::Position<centers::Geocentric,   frames::ECEF>;

// -- Cartesian Positions (AU) --
using IcrsCartPosAU     = cartesian::Position<centers::Barycentric,  frames::ICRS,                qtty::AstronomicalUnit>;
using GcrsCartPosAU     = cartesian::Position<centers::Geocentric,   frames::ICRS,                qtty::AstronomicalUnit>;
using EclipticCartPosAU = cartesian::Position<centers::Heliocentric, frames::EclipticMeanJ2000,   qtty::AstronomicalUnit>;
using HelioBaryCartPosAU = cartesian::Position<centers::Barycentric, frames::EclipticMeanJ2000,   qtty::AstronomicalUnit>;
using GeoBaryCartPosAU   = cartesian::Position<centers::Barycentric, frames::EclipticMeanJ2000,   qtty::AstronomicalUnit>;

// -- Cartesian Positions (Kilometer) --
using MoonGeoCartPosKM  = cartesian::Position<centers::Geocentric,  frames::EclipticMeanJ2000,   qtty::Kilometer>;

// ============================================================================
// Free functions — Geodetic conversions
// ============================================================================

/**
 * @brief Convert a Geodetic (WGS84) to ECEF Cartesian position.
 */
inline EcefCartPos geodetic_to_cartesian_ecef(const Geodetic& geo) {
    siderust_cartesian_pos_t out;
    check_status(
        siderust_geodetic_to_cartesian_ecef(geo.to_c(), &out),
        "geodetic_to_cartesian_ecef"
    );
    return EcefCartPos::from_c(out);
}

} // namespace siderust
