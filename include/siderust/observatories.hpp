#pragma once

/**
 * @file observatories.hpp
 * @brief Named observatory locations.
 */

#include "ffi_core.hpp"
#include "coordinates.hpp"

namespace siderust {

namespace detail {

inline Geodetic make_roque_de_los_muchachos() {
    siderust_geodetic_t out;
    check_status(siderust_observatory_roque_de_los_muchachos(&out), "ROQUE_DE_LOS_MUCHACHOS");
    return Geodetic::from_c(out);
}

inline Geodetic make_el_paranal() {
    siderust_geodetic_t out;
    check_status(siderust_observatory_el_paranal(&out), "EL_PARANAL");
    return Geodetic::from_c(out);
}

inline Geodetic make_mauna_kea() {
    siderust_geodetic_t out;
    check_status(siderust_observatory_mauna_kea(&out), "MAUNA_KEA");
    return Geodetic::from_c(out);
}

inline Geodetic make_la_silla() {
    siderust_geodetic_t out;
    check_status(siderust_observatory_la_silla(&out), "LA_SILLA_OBSERVATORY");
    return Geodetic::from_c(out);
}

} // namespace detail

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
inline const Geodetic ROQUE_DE_LOS_MUCHACHOS = detail::make_roque_de_los_muchachos();

/**
 * @brief El Paranal Observatory (Chile).
 */
inline const Geodetic EL_PARANAL = detail::make_el_paranal();

/**
 * @brief Mauna Kea Observatory (Hawaii, USA).
 */
inline const Geodetic MAUNA_KEA = detail::make_mauna_kea();

/**
 * @brief La Silla Observatory (Chile).
 */
inline const Geodetic LA_SILLA_OBSERVATORY = detail::make_la_silla();

// Backward-compatible function aliases.
inline Geodetic roque_de_los_muchachos() { return ROQUE_DE_LOS_MUCHACHOS; }
inline Geodetic el_paranal() { return EL_PARANAL; }
inline Geodetic mauna_kea() { return MAUNA_KEA; }
inline Geodetic la_silla() { return LA_SILLA_OBSERVATORY; }

} // namespace siderust
