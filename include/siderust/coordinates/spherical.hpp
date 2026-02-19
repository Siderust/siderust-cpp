#pragma once

/**
 * @file spherical.hpp
 * @brief Typed spherical coordinate templates.
 */

#include "../centers.hpp"
#include "../frames.hpp"
#include "../time.hpp"
#include "geodetic.hpp"

#include <qtty/qtty.hpp>

#include <type_traits>

namespace siderust {
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

    qtty::Degree lon;  ///< Azimuthal angle (RA, longitude, azimuth ...).
    qtty::Degree lat;  ///< Polar angle (Dec, latitude, altitude ...).

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

    // -- Generic accessors (always available, frame-agnostic) --

    /// Azimuthal component (RA, longitude, azimuth ... frame-dependent).
    qtty::Degree azimuthal() const { return lon; }
    /// Polar component (Dec, latitude, altitude ... frame-dependent).
    qtty::Degree polar() const { return lat; }

    // -- RA / Dec (equatorial frames: ICRS, ICRF, Equatorial* only) --

    /** @brief Right ascension. Only available for equatorial frames. */
    template<typename F_ = F, std::enable_if_t<frames::has_ra_dec_v<F_>, int> = 0>
    qtty::Degree ra() const { return lon; }

    /** @brief Declination. Only available for equatorial frames. */
    template<typename F_ = F, std::enable_if_t<frames::has_ra_dec_v<F_>, int> = 0>
    qtty::Degree dec() const { return lat; }

    // -- Azimuth / Altitude (Horizontal frame only) --

    /** @brief Azimuth (short form). Only available for Horizontal frame. */
    template<typename F_ = F, std::enable_if_t<frames::has_az_alt_v<F_>, int> = 0>
    qtty::Degree az() const { return lon; }

    /** @brief Altitude (short form). Only available for Horizontal frame. */
    template<typename F_ = F, std::enable_if_t<frames::has_az_alt_v<F_>, int> = 0>
    qtty::Degree alt() const { return lat; }

    /** @brief Azimuth (long form). Only available for Horizontal frame. */
    template<typename F_ = F, std::enable_if_t<frames::has_az_alt_v<F_>, int> = 0>
    qtty::Degree azimuth() const { return lon; }

    /** @brief Altitude (long form). Only available for Horizontal frame. */
    template<typename F_ = F, std::enable_if_t<frames::has_az_alt_v<F_>, int> = 0>
    qtty::Degree altitude() const { return lat; }

    // -- Longitude / Latitude (ecliptic, galactic, and generic frames) --

    /** @brief Ecliptic / galactic longitude. Only for lon/lat frames. */
    template<typename F_ = F, std::enable_if_t<frames::has_lon_lat_v<F_>, int> = 0>
    qtty::Degree longitude() const { return lon; }

    /** @brief Ecliptic / galactic latitude. Only for lon/lat frames. */
    template<typename F_ = F, std::enable_if_t<frames::has_lon_lat_v<F_>, int> = 0>
    qtty::Degree latitude() const { return lat; }

    siderust_spherical_dir_t to_c() const {
        return {lon.value(), lat.value(), frame_id()};
    }

    static Direction from_c(const siderust_spherical_dir_t& c) {
        return Direction(c.lon_deg, c.lat_deg);
    }

    /**
     * @brief Transform to a different reference frame.
     *
     * Only enabled for frame pairs with a FrameRotationProvider in the FFI.
     * Attempting an unsupported transform is a compile-time error.
     */
    template<typename Target>
    std::enable_if_t<
        frames::has_frame_transform_v<F, Target>,
        Direction<Target>
    >
    to_frame(const JulianDate& jd) const {
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
 */
template<typename C, typename F, typename U = qtty::Meter>
struct Position {
    static_assert(frames::is_frame_v<F>, "F must be a valid frame tag");
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

    static constexpr siderust_frame_t frame_id() { return frames::FrameTraits<F>::ffi_id; }
    static constexpr siderust_center_t center_id() { return centers::CenterTraits<C>::ffi_id; }

    U distance() const { return dist; }
};

} // namespace spherical
} // namespace siderust
