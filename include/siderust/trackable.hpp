#pragma once

/**
 * @file trackable.hpp
 * @brief Abstract base class for trackable celestial objects.
 *
 * `Trackable` defines a polymorphic interface for any celestial object
 * whose altitude and azimuth can be computed at an observer location.
 * Implementations include:
 *
 * - **Target** — fixed ICRS direction (RA/Dec)
 * - **StarTarget** — adapter for `Star` catalog objects
 * - **BodyTarget** — solar-system bodies (Sun, Moon, planets, Pluto)
 *
 * Use `std::unique_ptr<Trackable>` to hold heterogeneous collections of
 * trackable objects.
 *
 * ### Example
 * @code
 * auto sun = std::make_unique<siderust::BodyTarget>(siderust::Body::Sun);
 * qtty::Degree alt = sun->altitude_at(obs, now);
 *
 * // Polymorphic usage
 * std::vector<std::unique_ptr<siderust::Trackable>> targets;
 * targets.push_back(std::move(sun));
 * targets.push_back(std::make_unique<siderust::StarTarget>(VEGA));
 * for (const auto& t : targets) {
 *     std::cout << t->altitude_at(obs, now).value() << "\n";
 * }
 * @endcode
 */

#include "altitude.hpp"
#include "azimuth.hpp"
#include "coordinates.hpp"
#include "time.hpp"
#include <memory>
#include <vector>

namespace siderust {

/**
 * @brief Abstract interface for any object whose altitude/azimuth can be computed.
 *
 * This class defines the common API shared by all trackable celestial objects.
 * Implementations must provide altitude_at and azimuth_at at minimum; the
 * remaining methods have default implementations that throw if not overridden.
 */
class Trackable {
  public:
    virtual ~Trackable() = default;

    // ------------------------------------------------------------------
    // Altitude queries
    // ------------------------------------------------------------------

    /**
     * @brief Compute altitude at a given MJD instant.
     *
     * The return unit varies by implementation (radians for sun/moon/star,
     * degrees for Target/BodyTarget). Check the concrete class documentation.
     *
     * @note For BodyTarget, returns radians; for Target, returns degrees.
     */
    virtual qtty::Degree altitude_at(const Geodetic& obs, const MJD& mjd) const = 0;

    /**
     * @brief Find periods when the object is above a threshold altitude.
     */
    virtual std::vector<Period> above_threshold(
        const Geodetic& obs, const Period& window,
        qtty::Degree threshold, const SearchOptions& opts = {}) const = 0;

    /**
     * @brief Find periods when the object is below a threshold altitude.
     */
    virtual std::vector<Period> below_threshold(
        const Geodetic& obs, const Period& window,
        qtty::Degree threshold, const SearchOptions& opts = {}) const = 0;

    /**
     * @brief Find threshold-crossing events (rising / setting).
     */
    virtual std::vector<CrossingEvent> crossings(
        const Geodetic& obs, const Period& window,
        qtty::Degree threshold, const SearchOptions& opts = {}) const = 0;

    /**
     * @brief Find culmination (local altitude extremum) events.
     */
    virtual std::vector<CulminationEvent> culminations(
        const Geodetic& obs, const Period& window,
        const SearchOptions& opts = {}) const = 0;

    // ------------------------------------------------------------------
    // Azimuth queries
    // ------------------------------------------------------------------

    /**
     * @brief Compute azimuth (degrees, N-clockwise) at a given MJD instant.
     */
    virtual qtty::Degree azimuth_at(const Geodetic& obs, const MJD& mjd) const = 0;

    /**
     * @brief Find epochs when the object crosses a given azimuth bearing.
     */
    virtual std::vector<AzimuthCrossingEvent> azimuth_crossings(
        const Geodetic& obs, const Period& window,
        qtty::Degree bearing, const SearchOptions& opts = {}) const = 0;

    // Non-copyable, non-movable from base
    Trackable()                              = default;
    Trackable(const Trackable&)              = delete;
    Trackable& operator=(const Trackable&)   = delete;
    Trackable(Trackable&&)                   = default;
    Trackable& operator=(Trackable&&)        = default;
};

} // namespace siderust
