#pragma once

/**
 * @file time.hpp
 * @brief Re-exports tempoch time types into the siderust namespace.
 */

#include <tempoch/tempoch.hpp>

namespace siderust {

namespace scale = tempoch::scale;
namespace format = tempoch::format;

using CivilTime = tempoch::CivilTime;
using UTC = CivilTime;
using TimeContext = tempoch::TimeContext;

template <typename S> using Time = tempoch::Time<S>;
template <typename S> using J2000Seconds = tempoch::J2000Seconds<S>;

using JulianDate = tempoch::JulianDate<scale::TT>;
using MJD = tempoch::ModifiedJulianDate<scale::TT>;
using ModifiedJulianDate = MJD;
using Period = tempoch::TTMjdPeriod;

using UT1JulianDate = tempoch::JulianDate<scale::UT1>;
using TDBJulianDate = tempoch::JulianDate<scale::TDB>;
using TCGJulianDate = tempoch::JulianDate<scale::TCG>;
using TCBJulianDate = tempoch::JulianDate<scale::TCB>;
using TAIJulianDate = tempoch::JulianDate<scale::TAI>;
using UTCJulianDate = tempoch::JulianDate<scale::UTC>;
using TTTime = tempoch::Time<scale::TT>;
using UT1Time = tempoch::Time<scale::UT1>;

using UnixTime = tempoch::UnixTime;
using GpsTime = tempoch::GpsTime;

} // namespace siderust
