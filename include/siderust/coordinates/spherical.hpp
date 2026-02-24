#pragma once

/**
 * @file spherical.hpp
 * @ingroup coordinates_spherical
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
 * @ingroup coordinates_spherical
 * @tparam F  Reference frame tag (e.g. `frames::ICRS`).
 *
 * @par Accessors
 * Access values through frame-appropriate getters:
 * - Equatorial frames: `ra()`, `dec()`
 * - Horizontal frame: `az()`, `al()` / `alt()`
 * - Lon/lat frames: `lon()`, `lat()`
 */
template <typename F>
struct Direction {
    static_assert(frames::is_frame_v<F>, "F must be a valid frame tag");

  private:
    qtty::Degree azimuth_; ///< Azimuthal component (RA/longitude/azimuth).
    qtty::Degree polar_;   ///< Polar component (Dec/latitude/altitude).

  public:
    Direction() : azimuth_(qtty::Degree(0)), polar_(qtty::Degree(0)) {}

    Direction(qtty::Degree azimuth, qtty::Degree polar)
        : azimuth_(azimuth), polar_(polar) {}

    /// Raw-double convenience (degrees).
    Direction(double azimuth_deg, double polar_deg)
        : azimuth_(qtty::Degree(azimuth_deg)), polar_(qtty::Degree(polar_deg)) {}

    /// @name Frame info
    /// @{
    static constexpr siderust_frame_t frame_id() {
        return frames::FrameTraits<F>::ffi_id;
    }
    static constexpr const char* frame_name() {
        return frames::FrameTraits<F>::name();
    }
    /// @}

    /// @name RA / Dec (equatorial frames only)
    /// @{
    template <typename F_ = F, std::enable_if_t<frames::has_ra_dec_v<F_>, int> = 0>
    qtty::Degree ra() const { return azimuth_; }

    template <typename F_ = F, std::enable_if_t<frames::has_ra_dec_v<F_>, int> = 0>
    qtty::Degree dec() const { return polar_; }
    /// @}

    /// @name Azimuth / Altitude (Horizontal frame only)
    /// @{
    template <typename F_ = F, std::enable_if_t<frames::has_az_alt_v<F_>, int> = 0>
    qtty::Degree az() const { return azimuth_; }

    template <typename F_ = F, std::enable_if_t<frames::has_az_alt_v<F_>, int> = 0>
    qtty::Degree al() const { return polar_; }

    template <typename F_ = F, std::enable_if_t<frames::has_az_alt_v<F_>, int> = 0>
    qtty::Degree alt() const { return polar_; }

    template <typename F_ = F, std::enable_if_t<frames::has_az_alt_v<F_>, int> = 0>
    qtty::Degree altitude() const { return polar_; }
    /// @}

    /// @name Longitude / Latitude (lon/lat frames)
    /// @{
    template <typename F_ = F, std::enable_if_t<frames::has_lon_lat_v<F_>, int> = 0>
    qtty::Degree lon() const { return azimuth_; }

    template <typename F_ = F, std::enable_if_t<frames::has_lon_lat_v<F_>, int> = 0>
    qtty::Degree lat() const { return polar_; }

    template <typename F_ = F, std::enable_if_t<frames::has_lon_lat_v<F_>, int> = 0>
    qtty::Degree longitude() const { return azimuth_; }

    template <typename F_ = F, std::enable_if_t<frames::has_lon_lat_v<F_>, int> = 0>
    qtty::Degree latitude() const { return polar_; }
    /// @}

    /// @name FFI interop
    /// @{
    siderust_spherical_dir_t to_c() const {
        return {polar_.value(), azimuth_.value(), frame_id()};
    }

    static Direction from_c(const siderust_spherical_dir_t& c) {
        return Direction(c.azimuth_deg, c.polar_deg);
    }
    /// @}

    /**
     * @brief Transform to a different reference frame.
     *
     * Only enabled for frame pairs with a FrameRotationProvider in the FFI.
     * Attempting an unsupported transform is a compile-time error.
     *
     * @tparam Target Destination frame tag.
     */
    template <typename Target>
    std::enable_if_t<
        frames::has_frame_transform_v<F, Target>,
        Direction<Target>>
    to_frame(const JulianDate& jd) const {
        if constexpr (std::is_same_v<F, Target>) {
            return Direction<Target>(azimuth_.value(), polar_.value());
        } else {
            siderust_spherical_dir_t out;
            check_status(
                siderust_spherical_dir_transform_frame(
                    polar_.value(), azimuth_.value(),
                    frames::FrameTraits<F>::ffi_id,
                    frames::FrameTraits<Target>::ffi_id,
                    jd.value(), &out),
                "Direction::to_frame");
            return Direction<Target>::from_c(out);
        }
    }

    /**
     * @brief Shorthand: `.to<Target>(jd)` (calls `to_frame`).
     */
    template <typename Target>
    auto to(const JulianDate& jd) const
        -> decltype(this->template to_frame<Target>(jd)) {
        return to_frame<Target>(jd);
    }

    /**
     * @brief Transform to the horizontal (alt-az) frame.
     */
    template <typename F_ = F>
    std::enable_if_t<
        frames::has_horizontal_transform_v<F_>,
        Direction<frames::Horizontal>>
    to_horizontal(const JulianDate& jd, const Geodetic& observer) const {
        siderust_spherical_dir_t out;
        check_status(
            siderust_spherical_dir_to_horizontal(
                polar_.value(), azimuth_.value(),
                frames::FrameTraits<F>::ffi_id,
                jd.value(), observer.to_c(), &out),
            "Direction::to_horizontal");
        return Direction<frames::Horizontal>::from_c(out);
    }
};

/**
 * @brief A spherical position (direction + distance), compile-time tagged.
 *
 * Mirrors Rust's `affn::spherical::Position<C, F, U>`.
 *
 * @ingroup coordinates_spherical
 * @tparam C Reference center tag (e.g. `centers::Barycentric`).
 * @tparam F Reference frame tag (e.g. `frames::ICRS`).
 * @tparam U Distance unit (default: `qtty::Meter`).
 */
template <typename C, typename F, typename U = qtty::Meter>
struct Position {
    static_assert(frames::is_frame_v<F>, "F must be a valid frame tag");
    static_assert(centers::is_center_v<C>, "C must be a valid center tag");

  private:
    qtty::Degree azimuth_;
    qtty::Degree polar_;
    U            dist_;

  public:
    Position()
        : azimuth_(qtty::Degree(0)), polar_(qtty::Degree(0)), dist_(U(0)) {}

    Position(qtty::Degree azimuth, qtty::Degree polar, U dist)
        : azimuth_(azimuth), polar_(polar), dist_(dist) {}

    Position(double azimuth_deg, double polar_deg, double dist_val)
        : azimuth_(qtty::Degree(azimuth_deg)),
          polar_(qtty::Degree(polar_deg)),
          dist_(U(dist_val)) {}

    /// Extract the direction component.
    Direction<F> direction() const {
        return Direction<F>(azimuth_, polar_);
    }

    /// @name Component accessors by frame convention
    /// @{
    template <typename F_ = F, std::enable_if_t<frames::has_ra_dec_v<F_>, int> = 0>
    qtty::Degree ra() const { return azimuth_; }

    template <typename F_ = F, std::enable_if_t<frames::has_ra_dec_v<F_>, int> = 0>
    qtty::Degree dec() const { return polar_; }

    template <typename F_ = F, std::enable_if_t<frames::has_az_alt_v<F_>, int> = 0>
    qtty::Degree az() const { return azimuth_; }

    template <typename F_ = F, std::enable_if_t<frames::has_az_alt_v<F_>, int> = 0>
    qtty::Degree al() const { return polar_; }

    template <typename F_ = F, std::enable_if_t<frames::has_az_alt_v<F_>, int> = 0>
    qtty::Degree alt() const { return polar_; }

    template <typename F_ = F, std::enable_if_t<frames::has_lon_lat_v<F_>, int> = 0>
    qtty::Degree lon() const { return azimuth_; }

    template <typename F_ = F, std::enable_if_t<frames::has_lon_lat_v<F_>, int> = 0>
    qtty::Degree lat() const { return polar_; }
    /// @}

    static constexpr siderust_frame_t  frame_id() { return frames::FrameTraits<F>::ffi_id; }
    static constexpr siderust_center_t center_id() { return centers::CenterTraits<C>::ffi_id; }

    U distance() const { return dist_; }
};

} // namespace spherical
} // namespace siderust
