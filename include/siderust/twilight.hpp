#pragma once

/**
 * @file twilight.hpp
 * @brief Twilight / darkness phase classification from Sun altitude.
 *
 * Wraps the `siderust_twilight_classification_deg` and
 * `siderust_twilight_classification_rad` FFI functions as a single free
 * function `twilight_phase()` that accepts a `qtty::Degree` value and returns
 * a typed `TwilightPhase` enum.
 *
 * ### Sky-condition boundaries (IAU/USNO convention)
 * | Phase        | Sun altitude |
 * |---|---|
 * | Day          | > 0° |
 * | Civil        | -6° to 0° |
 * | Nautical     | -12° to -6° |
 * | Astronomical | -18° to -12° |
 * | Dark         | < -18° |
 *
 * ### Example
 * @code
 * #include <siderust/twilight.hpp>
 * #include <siderust/time.hpp>
 *
 * auto phase = siderust::twilight_phase(qtty::Degree(-7.5));
 * // phase == TwilightPhase::Nautical
 * @endcode
 */

#include "ffi_core.hpp"

#include <qtty/qtty.hpp>

namespace siderust {

/**
 * @brief Sky-condition phase determined by the Sun's altitude.
 *
 * Mirrors `SiderustTwilightPhase` from `siderust-ffi`.
 */
enum class TwilightPhase : int32_t {
  Day = SIDERUST_TWILIGHT_PHASE_T_DAY,           ///< Sun above horizon.
  Civil = SIDERUST_TWILIGHT_PHASE_T_CIVIL,       ///< -6° to 0°.
  Nautical = SIDERUST_TWILIGHT_PHASE_T_NAUTICAL, ///< -12° to -6°.
  Astronomical =
      SIDERUST_TWILIGHT_PHASE_T_ASTRONOMICAL, ///< -18° to -12°.
  Dark = SIDERUST_TWILIGHT_PHASE_T_DARK,      ///< Below -18°.
};

/**
 * @brief Classify the sky condition from the Sun's altitude in degrees.
 *
 * @param altitude  Sun altitude above the horizon (degrees, signed).
 *                  Negative values mean the Sun is below the horizon.
 * @return          The corresponding `TwilightPhase` value.
 * @throws NullPointerError  (should never happen in normal operation).
 */
inline TwilightPhase twilight_phase(qtty::Degree altitude) {
  siderust_twilight_phase_t out{};
  check_status(siderust_twilight_classification_deg(altitude.value(), &out),
               "twilight_phase");
  return static_cast<TwilightPhase>(out);
}

/**
 * @brief Classify the sky condition from the Sun's altitude in radians.
 *
 * @param altitude  Sun altitude (radians).
 * @return          The corresponding `TwilightPhase` value.
 */
inline TwilightPhase twilight_phase(qtty::Radian altitude) {
  siderust_twilight_phase_t out{};
  check_status(siderust_twilight_classification_rad(altitude.value(), &out),
               "twilight_phase(rad)");
  return static_cast<TwilightPhase>(out);
}

} // namespace siderust
