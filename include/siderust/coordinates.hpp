#pragma once

/**
 * @file coordinates.hpp
 * @brief Coordinate module umbrella.
 *
 * @defgroup coordinates Coordinates
 * @brief Typed coordinate types and conversions.
 *
 * Primary include: `#include <siderust/coordinates.hpp>`.
 *
 * The coordinate API is split into:
 * - @ref coordinates_geodetic
 * - @ref coordinates_spherical
 * - @ref coordinates_cartesian
 * - @ref coordinates_types
 * - @ref coordinates_conversions
 *
 * For a quick overview of the available coordinate types and their main
 * methods, see @ref coordinate_types.
 *
 * This umbrella header includes:
 * - `coordinates/geodetic.hpp`
 * - `coordinates/spherical.hpp`
 * - `coordinates/cartesian.hpp`
 * - `coordinates/types.hpp`
 * - `coordinates/conversions.hpp`
 *
 * @{
 */

/** @defgroup coordinates_geodetic Geodetic coordinates
 *  @ingroup coordinates
 */

/** @defgroup coordinates_spherical Spherical coordinates
 *  @ingroup coordinates
 */

/** @defgroup coordinates_cartesian Cartesian coordinates
 *  @ingroup coordinates
 */

/** @defgroup coordinates_types Common coordinate aliases
 *  @ingroup coordinates
 */

/** @defgroup coordinates_conversions Coordinate conversions
 *  @ingroup coordinates
 */

#include "coordinates/cartesian.hpp"
#include "coordinates/conversions.hpp"
#include "coordinates/geodetic.hpp"
#include "coordinates/spherical.hpp"
#include "coordinates/types.hpp"

/** @} */ // end of group coordinates
