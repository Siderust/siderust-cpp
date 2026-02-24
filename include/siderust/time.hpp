#pragma once

/**
 * @file time.hpp
 * @brief Re-exports tempoch C++ types into the siderust namespace.
 *
 * The canonical implementations live in tempoch-cpp (namespace tempoch).
 * This header provides backward-compatible aliases so that existing code
 * using siderust::JulianDate, siderust::MJD, etc. continues to compile.
 */

#include <tempoch/tempoch.hpp>

namespace siderust {

using CivilTime  = tempoch::CivilTime;
using UTC        = tempoch::UTC;          // alias for CivilTime
using JulianDate = tempoch::JulianDate;   // Time<JDScale>
using MJD        = tempoch::MJD;          // Time<MJDScale>
using Period     = tempoch::Period<tempoch::MJD>;

} // namespace siderust
