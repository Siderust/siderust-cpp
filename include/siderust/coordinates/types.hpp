#pragma once

/**
 * @file types.hpp
 * @brief Concise aliases for common typed coordinates.
 *
 * Mirrors the Rust `coordinates::types` style:
 * - concise aliases (`IcrsDir`, `IcrsPos`, ...)
 * - nested compatibility namespaces (`types::direction`, `types::position`)
 */

#include "cartesian.hpp"
#include "spherical.hpp"

namespace siderust {

namespace types {

// ============================================================================
// Concise aliases
// ============================================================================

// -- Spherical Directions --
using IcrsDir                 = spherical::Direction<frames::ICRS>;
using IcrfDir                 = spherical::Direction<frames::ICRF>;
using EclipticDir             = spherical::Direction<frames::EclipticMeanJ2000>;
using EquatorialJ2000Dir      = spherical::Direction<frames::EquatorialMeanJ2000>;
using EquatorialMeanOfDateDir = spherical::Direction<frames::EquatorialMeanOfDate>;
using EquatorialTrueOfDateDir = spherical::Direction<frames::EquatorialTrueOfDate>;
using HorizontalDir           = spherical::Direction<frames::Horizontal>;
using GalacticDir             = spherical::Direction<frames::Galactic>;

// -- Spherical Positions --
using IcrsPos       = spherical::Position<centers::Barycentric, frames::ICRS>;
using GcrsPos       = spherical::Position<centers::Geocentric, frames::ICRS>;
using HcrsPos       = spherical::Position<centers::Heliocentric, frames::ICRS>;
using EclipticPos   = spherical::Position<centers::Heliocentric, frames::EclipticMeanJ2000>;
using HorizontalPos = spherical::Position<centers::Topocentric, frames::Horizontal>;

// -- Cartesian Directions --
using IcrsCartDir     = cartesian::Direction<frames::ICRS>;
using EclipticCartDir = cartesian::Direction<frames::EclipticMeanJ2000>;

// -- Cartesian Positions (Meter) --
using IcrsCartPos     = cartesian::Position<centers::Barycentric, frames::ICRS>;
using GcrsCartPos     = cartesian::Position<centers::Geocentric, frames::ICRS>;
using EclipticCartPos = cartesian::Position<centers::Heliocentric, frames::EclipticMeanJ2000>;
using EcefCartPos     = cartesian::Position<centers::Geocentric, frames::ECEF>;

// -- Cartesian Positions (AU) --
using IcrsCartPosAU      = cartesian::Position<centers::Barycentric, frames::ICRS, qtty::AstronomicalUnit>;
using GcrsCartPosAU      = cartesian::Position<centers::Geocentric, frames::ICRS, qtty::AstronomicalUnit>;
using EclipticCartPosAU  = cartesian::Position<centers::Heliocentric, frames::EclipticMeanJ2000, qtty::AstronomicalUnit>;
using HelioBaryCartPosAU = cartesian::Position<centers::Barycentric, frames::EclipticMeanJ2000, qtty::AstronomicalUnit>;
using GeoBaryCartPosAU   = cartesian::Position<centers::Barycentric, frames::EclipticMeanJ2000, qtty::AstronomicalUnit>;

// -- Cartesian Positions (Kilometer) --
using MoonGeoCartPosKM = cartesian::Position<centers::Geocentric, frames::EclipticMeanJ2000, qtty::Kilometer>;

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

// ============================================================================
// Backward-compatible root aliases
// ============================================================================

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

} // namespace siderust
