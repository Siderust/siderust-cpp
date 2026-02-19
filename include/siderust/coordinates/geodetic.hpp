#pragma once

/**
 * @file geodetic.hpp
 * @brief WGS84 geodetic coordinate type.
 */

#include "../ffi_core.hpp"
#include "../centers.hpp"
#include "../frames.hpp"

#include <qtty/qtty.hpp>

namespace siderust {
namespace cartesian {
template<typename C, typename F, typename U>
struct Position;
}

/**
 * @brief Geodetic position (WGS84 ellipsoid).
 *
 * Fixed to frame=ECEF, center=Geocentric. Uses qtty quantities.
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

    /**
     * @brief Convert geodetic (WGS84/ECEF) to cartesian position.
     *
     * @tparam U Output length unit (default: meter).
     */
    template<typename U = qtty::Meter>
    cartesian::Position<centers::Geocentric, frames::ECEF, U> to_cartesian() const;
};

} // namespace siderust
