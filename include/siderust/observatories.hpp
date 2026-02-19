#pragma once

/**
 * @file observatories.hpp
 * @brief Named observatory locations.
 */

#include "ffi_core.hpp"
#include "coordinates.hpp"

namespace siderust {

/**
 * @brief Create a custom geodetic position (WGS84).
 */
inline Geodetic geodetic(double lon_deg, double lat_deg, double height_m = 0.0) {
    siderust_geodetic_t out;
    check_status(siderust_geodetic_new(lon_deg, lat_deg, height_m, &out),
                 "geodetic");
    return Geodetic::from_c(out);
}

/**
 * @brief Roque de los Muchachos Observatory (La Palma, Spain).
 */
inline Geodetic roque_de_los_muchachos() {
    siderust_geodetic_t out;
    check_status(siderust_observatory_roque_de_los_muchachos(&out), "roque_de_los_muchachos");
    return Geodetic::from_c(out);
}

/**
 * @brief El Paranal Observatory (Chile).
 */
inline Geodetic el_paranal() {
    siderust_geodetic_t out;
    check_status(siderust_observatory_el_paranal(&out), "el_paranal");
    return Geodetic::from_c(out);
}

/**
 * @brief Mauna Kea Observatory (Hawaii, USA).
 */
inline Geodetic mauna_kea() {
    siderust_geodetic_t out;
    check_status(siderust_observatory_mauna_kea(&out), "mauna_kea");
    return Geodetic::from_c(out);
}

/**
 * @brief La Silla Observatory (Chile).
 */
inline Geodetic la_silla() {
    siderust_geodetic_t out;
    check_status(siderust_observatory_la_silla(&out), "la_silla");
    return Geodetic::from_c(out);
}

} // namespace siderust
