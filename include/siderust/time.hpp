#pragma once

/**
 * @file time.hpp
 * @brief Public siderust time tags and typed time/period aliases.
 */

#include <tempoch/tempoch.hpp>

namespace siderust {

namespace scale = tempoch::scale;
namespace format = tempoch::format;

using CivilTime = tempoch::CivilTime;
using TimeContext = tempoch::TimeContext;

using TT = tempoch::scale::TT;
using UTC = tempoch::scale::UTC;
using UT1 = tempoch::scale::UT1;
using TDB = tempoch::scale::TDB;
using TCG = tempoch::scale::TCG;
using TCB = tempoch::scale::TCB;
using TAI = tempoch::scale::TAI;
using GPST = tempoch::scale::GPST;

using JD = tempoch::format::JD;
using MJD = tempoch::format::MJD;
using Unix = tempoch::format::Unix;
using GPS = tempoch::format::GPS;
using J2000s = tempoch::format::J2000s;

// Calendar values are civil UTC labels, not scalar encoded times.
using Calendar = CivilTime;

template <typename Scale, typename Format> using Time = tempoch::EncodedTime<Scale, Format>;
template <typename Scale, typename Format> using Period = tempoch::Period<Time<Scale, Format>>;

template <typename Scale> using TimeAxis = tempoch::Time<Scale>;
template <typename Scale> using J2000Seconds = tempoch::J2000Seconds<Scale>;

} // namespace siderust
