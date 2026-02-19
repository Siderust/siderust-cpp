#pragma once

/**
 * @file coordinates.hpp
 * @brief Unified coordinate API — typed templates + legacy runtime-tagged types.
 *
 * This header re-exports the new compile-time typed coordinate system and
 * provides backward-compatible aliases so existing code continues to compile.
 *
 * **New API** (preferred):
 *   - `spherical::Direction<frames::ICRS>`, `IcrsDir`, `EclipticDir`, …
 *   - `cartesian::Position<centers::Heliocentric, frames::EclipticMeanJ2000, qtty::AstronomicalUnit>`
 *   - `.to_frame<Target>(jd)`, `.to_horizontal(jd, obs)`, `.to<Target>(jd)`
 *
 * **Legacy API** (deprecated — still works):
 *   - `SphericalDirection`, `CartesianPosition` (runtime Frame/Center enums)
 *   - `.transform(Frame, double)`, `.to_horizontal(Geodetic, double)`
 */

#include "ffi_core.hpp"
#include "typed_coordinates.hpp"

namespace siderust {

// ============================================================================
// Legacy Runtime-Tagged Types (backward compatibility)
// ============================================================================

/**
 * @brief Runtime-tagged spherical direction (deprecated — prefer typed Direction<F>).
 *
 * Preserved for backward compatibility with existing code that uses
 * `Frame` enum fields and `.transform(Frame, double)` calls.
 */
struct SphericalDirection {
    double lon_deg;
    double lat_deg;
    Frame  frame;

    SphericalDirection() : lon_deg(0), lat_deg(0), frame(Frame::ICRS) {}

    SphericalDirection(double lon, double lat, Frame f)
        : lon_deg(lon), lat_deg(lat), frame(f) {}

    static SphericalDirection from_c(const siderust_spherical_dir_t& c) {
        return SphericalDirection(c.lon_deg, c.lat_deg, static_cast<Frame>(c.frame));
    }

    siderust_spherical_dir_t to_c() const {
        return {lon_deg, lat_deg, static_cast<siderust_frame_t>(frame)};
    }

    double ra_deg()       const { return lon_deg; }
    double dec_deg()      const { return lat_deg; }
    double azimuth_deg()  const { return lon_deg; }
    double altitude_deg() const { return lat_deg; }

    /**
     * @brief Transform to a different frame (runtime dispatch).
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
     * @brief Transform to horizontal frame (runtime dispatch).
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

    // ================================================================
    // Interop with typed API
    // ================================================================

    /**
     * @brief Convert a typed Direction<F> to this runtime-tagged type.
     */
    template<typename F>
    static SphericalDirection from_typed(const spherical::Direction<F>& d) {
        return SphericalDirection(d.lon_deg(), d.lat_deg(),
                                 static_cast<Frame>(frames::FrameTraits<F>::ffi_id));
    }
};

/**
 * @brief Runtime-tagged Cartesian position (deprecated — prefer typed Position<C,F,U>).
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

    static CartesianPosition from_c(const siderust_cartesian_pos_t& c) {
        return CartesianPosition(
            c.x, c.y, c.z,
            static_cast<Frame>(c.frame),
            static_cast<Center>(c.center));
    }

    siderust_cartesian_pos_t to_c() const {
        return {x, y, z,
                static_cast<siderust_frame_t>(frame),
                static_cast<siderust_center_t>(center)};
    }

    /**
     * @brief Convert a typed cartesian::Position to this runtime-tagged type.
     */
    template<typename C, typename F, typename U>
    static CartesianPosition from_typed(const cartesian::Position<C, F, U>& p) {
        return CartesianPosition(
            p.x(), p.y(), p.z(),
            static_cast<Frame>(frames::FrameTraits<F>::ffi_id),
            static_cast<Center>(centers::CenterTraits<C>::ffi_id));
    }
};

// ============================================================================
// Legacy free function — geodetic to ECEF (runtime-tagged)
// ============================================================================

/**
 * @brief Convert a geodetic position to ECEF Cartesian (legacy runtime-tagged).
 *
 * @deprecated Prefer `siderust::geodetic_to_cartesian_ecef(geo)` which returns
 *             the typed `EcefCartPos`.
 */
inline CartesianPosition geodetic_to_cartesian_ecef_legacy(const Geodetic& geo) {
    siderust_cartesian_pos_t out;
    check_status(
        siderust_geodetic_to_cartesian_ecef(geo.to_c(), &out),
        "geodetic_to_cartesian_ecef"
    );
    return CartesianPosition::from_c(out);
}

} // namespace siderust
