#pragma once

/**
 * @file observatories.hpp
 * @brief Named observatory locations.
 */

#include "ffi_core.hpp"
#include "typed_coordinates.hpp"

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

// Named observatory helpers --------------------------------------------------

#define SIDERUST_OBSERVATORY(Name, fn)                     \
    inline Geodetic Name() {                               \
        siderust_geodetic_t out;                           \
        check_status(fn(&out), #Name);                     \
        return Geodetic::from_c(out);                      \
    }

/// Roque de los Muchachos (La Palma, Spain).
SIDERUST_OBSERVATORY(roque_de_los_muchachos, siderust_observatory_roque_de_los_muchachos)
/// El Paranal (Chile).
SIDERUST_OBSERVATORY(el_paranal,             siderust_observatory_el_paranal)
/// Mauna Kea (Hawaiʻi, USA).
SIDERUST_OBSERVATORY(mauna_kea,              siderust_observatory_mauna_kea)
/// La Silla (Chile).
SIDERUST_OBSERVATORY(la_silla,               siderust_observatory_la_silla)

#undef SIDERUST_OBSERVATORY

} // namespace siderust
