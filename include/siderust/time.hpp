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

using UTC        = tempoch::UTC;
using JulianDate = tempoch::JulianDate;
using MJD        = tempoch::MJD;
using Period     = tempoch::Period;

} // namespace siderust
