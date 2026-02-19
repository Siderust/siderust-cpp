#pragma once

/**
 * @file conversions.hpp
 * @ingroup coordinates_conversions
 * @brief Free coordinate conversion helpers.
 */

#include "types.hpp"

namespace siderust {

template<typename U>
inline cartesian::Position<centers::Geocentric, frames::ECEF, U>
Geodetic::to_cartesian() const {
    siderust_cartesian_pos_t out;
    check_status(
        siderust_geodetic_to_cartesian_ecef(to_c(), &out),
        "Geodetic::to_cartesian"
    );
    const auto ecef_m = EcefCartPos::from_c(out);
    return cartesian::Position<centers::Geocentric, frames::ECEF, U>(
        ecef_m.x().template to<U>(),
        ecef_m.y().template to<U>(),
        ecef_m.z().template to<U>()
    );
}

/**
 * @brief Convert a Geodetic (WGS84) to ECEF Cartesian position.
 *
 * @ingroup coordinates_conversions
 */
inline EcefCartPos geodetic_to_cartesian_ecef(const Geodetic& geo) {
    return geo.to_cartesian<qtty::Meter>();
}

} // namespace siderust
