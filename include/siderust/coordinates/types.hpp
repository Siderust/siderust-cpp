#pragma once

/**
 * @file types.hpp
 * @ingroup coordinates_types
 * @brief Concise aliases for common typed coordinates.
 *
 * Mirrors the Rust `coordinates::types` style:
 * - concise aliases (`IcrsDir`, `IcrsPos`, ...)
 * - nested compatibility namespaces (`types::direction`, `types::position`)
 *
 * This header is intended as the "index" of commonly used coordinate types.
 */

#include "cartesian.hpp"
#include "spherical.hpp"

namespace siderust {

/// Common, readable aliases for typed coordinates.
namespace types {

/** @addtogroup coordinates_types
 * @{
 */

// ============================================================================
// Concise aliases
// ============================================================================

/// @name Spherical directions
/// @{
/// ICRS spherical direction (RA/Dec).
using IcrsDir                 = spherical::Direction<frames::ICRS>;
/// ICRF spherical direction (RA/Dec; treated ≡ ICRS).
using IcrfDir                 = spherical::Direction<frames::ICRF>;
/// Mean ecliptic & equinox of J2000.0 direction (lon/lat).
using EclipticDir             = spherical::Direction<frames::EclipticMeanJ2000>;
/// Mean equatorial of J2000.0 direction (RA/Dec).
using EquatorialJ2000Dir      = spherical::Direction<frames::EquatorialMeanJ2000>;
/// Mean equatorial of date direction (RA/Dec).
using EquatorialMeanOfDateDir = spherical::Direction<frames::EquatorialMeanOfDate>;
/// True equatorial of date direction (RA/Dec).
using EquatorialTrueOfDateDir = spherical::Direction<frames::EquatorialTrueOfDate>;
/// Local horizontal direction (az/alt).
using HorizontalDir           = spherical::Direction<frames::Horizontal>;
/// Galactic direction (l/b).
using GalacticDir             = spherical::Direction<frames::Galactic>;
/// @}

/// @name Spherical positions
/// @{
/// Barycentric ICRS spherical position.
using IcrsPos       = spherical::Position<centers::Barycentric, frames::ICRS>;
/// Geocentric ICRS spherical position.
using GcrsPos       = spherical::Position<centers::Geocentric, frames::ICRS>;
/// Heliocentric ICRS spherical position.
using HcrsPos       = spherical::Position<centers::Heliocentric, frames::ICRS>;
/// Heliocentric mean-ecliptic-of-J2000 spherical position.
using EclipticPos   = spherical::Position<centers::Heliocentric, frames::EclipticMeanJ2000>;
/// Topocentric horizontal spherical position.
using HorizontalPos = spherical::Position<centers::Topocentric, frames::Horizontal>;
/// @}

/// @name Cartesian directions
/// @{
/// ICRS cartesian direction.
using IcrsCartDir     = cartesian::Direction<frames::ICRS>;
/// Mean ecliptic & equinox of J2000.0 cartesian direction.
using EclipticCartDir = cartesian::Direction<frames::EclipticMeanJ2000>;
/// @}

/// @name Cartesian positions (meter)
/// @{
/// Barycentric ICRS cartesian position in meters.
using IcrsCartPos     = cartesian::Position<centers::Barycentric, frames::ICRS>;
/// Geocentric ICRS cartesian position in meters.
using GcrsCartPos     = cartesian::Position<centers::Geocentric, frames::ICRS>;
/// Heliocentric mean-ecliptic-of-J2000 cartesian position in meters.
using EclipticCartPos = cartesian::Position<centers::Heliocentric, frames::EclipticMeanJ2000>;
/// Geocentric ECEF cartesian position in meters.
using EcefCartPos     = cartesian::Position<centers::Geocentric, frames::ECEF>;
/// @}

/// @name Cartesian positions (astronomical unit)
/// @{
/// Barycentric ICRS cartesian position in AU.
using IcrsCartPosAU      = cartesian::Position<centers::Barycentric, frames::ICRS, qtty::AstronomicalUnit>;
/// Geocentric ICRS cartesian position in AU.
using GcrsCartPosAU      = cartesian::Position<centers::Geocentric, frames::ICRS, qtty::AstronomicalUnit>;
/// Heliocentric mean-ecliptic-of-J2000 cartesian position in AU.
using EclipticCartPosAU  = cartesian::Position<centers::Heliocentric, frames::EclipticMeanJ2000, qtty::AstronomicalUnit>;
/// Barycentric mean-ecliptic-of-J2000 cartesian position in AU.
using HelioBaryCartPosAU = cartesian::Position<centers::Barycentric, frames::EclipticMeanJ2000, qtty::AstronomicalUnit>;
/// Barycentric mean-ecliptic-of-J2000 cartesian position in AU (compat alias).
using GeoBaryCartPosAU   = cartesian::Position<centers::Barycentric, frames::EclipticMeanJ2000, qtty::AstronomicalUnit>;
/// @}

/// @name Cartesian positions (kilometer)
/// @{
/// Geocentric mean-ecliptic-of-J2000 cartesian position in km.
using MoonGeoCartPosKM = cartesian::Position<centers::Geocentric, frames::EclipticMeanJ2000, qtty::Kilometer>;
/// @}

// ============================================================================
// Rust-style compatibility namespaces
// ============================================================================

namespace direction {
using EclipticMeanJ2000   = EclipticDir;
using EquatorialMeanJ2000 = EquatorialJ2000Dir;
using EquatorialMeanOfDate = EquatorialMeanOfDateDir;
using EquatorialTrueOfDate = EquatorialTrueOfDateDir;
using Galactic            = GalacticDir;
using Horizontal          = HorizontalDir;
using ICRS                = IcrsDir;
using ICRF                = IcrfDir;
}

namespace position {
using EclipticMeanJ2000   = EclipticPos;
using GCRS                = GcrsPos;
using HCRS                = HcrsPos;
using Horizontal          = HorizontalPos;
using ICRS                = IcrsPos;
}

} // namespace types

/** @} */ // end of group coordinates_types

// ============================================================================
// Backward-compatible root aliases
// ============================================================================

/** @addtogroup coordinates_types
 * @{
 */

/** @name Backward-compatible root aliases
 *
 * Prefer the explicit `siderust::types::*` names in new code.
 * @{
 */

using types::EcefCartPos;
using types::EclipticCartDir;
using types::EclipticCartPos;
using types::EclipticCartPosAU;
using types::EclipticDir;
using types::EclipticPos;
using types::EquatorialJ2000Dir;
using types::EquatorialMeanOfDateDir;
using types::EquatorialTrueOfDateDir;
using types::GalacticDir;
using types::GcrsCartPos;
using types::GcrsCartPosAU;
using types::GcrsPos;
using types::GeoBaryCartPosAU;
using types::HcrsPos;
using types::HelioBaryCartPosAU;
using types::HorizontalDir;
using types::HorizontalPos;
using types::IcrfDir;
using types::IcrsCartDir;
using types::IcrsCartPos;
using types::IcrsCartPosAU;
using types::IcrsDir;
using types::IcrsPos;
using types::MoonGeoCartPosKM;

/** @} */ // end of name Backward-compatible root aliases

// ============================================================================
// Intuitive short namespaces
// ============================================================================

namespace direction {
using Icrs = types::IcrsDir;
using Icrf = types::IcrfDir;
using Ecl = types::EclipticDir;
using EqJ2000 = types::EquatorialJ2000Dir;
using EqMod = types::EquatorialMeanOfDateDir;
using EqTod = types::EquatorialTrueOfDateDir;
using Hor = types::HorizontalDir;
using Gal = types::GalacticDir;

using ICRS = Icrs;
using ICRF = Icrf;
using EclipticMeanJ2000 = Ecl;
using EquatorialMeanJ2000 = EqJ2000;
using EquatorialMeanOfDate = EqMod;
using EquatorialTrueOfDate = EqTod;
using Horizontal = Hor;
using Galactic = Gal;
}

namespace position {
using Icrs = types::IcrsPos;
using Gcrs = types::GcrsPos;
using Hcrs = types::HcrsPos;
using Ecl = types::EclipticPos;
using Hor = types::HorizontalPos;

using ICRS = Icrs;
using GCRS = Gcrs;
using HCRS = Hcrs;
using EclipticMeanJ2000 = Ecl;
using Horizontal = Hor;
}

/** @} */ // end of group coordinates_types

} // namespace siderust
