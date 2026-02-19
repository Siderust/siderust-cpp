#pragma once

/**
 * @file types.hpp
 * @ingroup coordinates_types
 * @brief Common coordinate aliases (umbrella).
 *
 * Aliases are exposed in namespace-scoped form, e.g.:
 * - `spherical::direction::ICRS`
 * - `spherical::position::ICRS<qtty::Meter>`
 * - `cartesian::position::ECEF<qtty::Meter>`
 *
 * This header includes all modular alias headers under `coordinates/types/...`.
 *
 * For narrow includes, use the specific headers directly, e.g.:
 * - `coordinates/types/spherical/direction/equatorial.hpp`
 * - `coordinates/types/spherical/direction/ecliptic.hpp`
 * - `coordinates/types/spherical/direction/horizontal.hpp`
 * - `coordinates/types/spherical/position/equatorial.hpp`
 * - `coordinates/types/cartesian/position/geodetic.hpp`
 */

#include "types/cartesian.hpp"
#include "types/spherical.hpp"
