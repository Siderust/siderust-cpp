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

using CivilTime = tempoch::CivilTime;
using UTC = tempoch::UTC;               // alias for CivilTime
using JulianDate = tempoch::JulianDate; // Time<JDScale>
using MJD = tempoch::MJD;               // Time<MJDScale>
using TDB = tempoch::TDB;               // Time<TDBScale>
using TT = tempoch::TT;                 // Time<TTScale>
using TAI = tempoch::TAI;               // Time<TAIScale>
using TCG = tempoch::TCG;               // Time<TCGScale>
using TCB = tempoch::TCB;               // Time<TCBScale>
using GPS = tempoch::GPS;               // Time<GPSScale>
using UT = tempoch::UT;                 // Time<UTScale>
using UniversalTime = tempoch::UniversalTime; // alias for UT
using JDE = tempoch::JDE;               // Time<JDEScale>
using UnixTime = tempoch::UnixTime;     // Time<UnixTimeScale>
using Period = tempoch::Period<tempoch::MJD>;

} // namespace siderust
