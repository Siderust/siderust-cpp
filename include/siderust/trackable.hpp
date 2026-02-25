#pragma once

/**
 * @file trackable.hpp
 * @brief Abstract base class for all celestial targets.
 *
 * `Target` is the unified concept for anything in the sky that can be
 * pointed at from an observer location.  Concrete implementations cover:
 *
 * - **DirectionTarget** — fixed spherical direction in any supported frame
 *   (ICRS, equatorial, ecliptic).  Aliased as `ICRSTarget`, etc.
 * - **StarTarget** — adapter for `Star` catalog objects
 * - **BodyTarget** — solar-system bodies (Sun, Moon, planets)
 * - *(future)* **SatelliteTarget** — Earth-orbiting satellites (TLE/SGP4)
 *
 * Every `Target` carries a human-readable `name()`.
 * Use `std::unique_ptr<Target>` to hold heterogeneous collections.
 *
 * ### Example
 * @code
 * auto sun = std::make_unique<siderust::BodyTarget>(siderust::Body::Sun);
 * std::cout << sun->name() << "\n";           // "Sun"
 * qtty::Degree alt = sun->altitude_at(obs, now);
 *
 * // Polymorphic usage
 * std::vector<std::unique_ptr<siderust::Target>> targets;
 * targets.push_back(std::move(sun));
 * targets.push_back(std::make_unique<siderust::StarTarget>(VEGA));
 * for (const auto& t : targets) {
 *     std::cout << t->name() << ": " << t->altitude_at(obs, now) << "\n";
 * }
 * @endcode
 */

#include "altitude.hpp"
#include "azimuth.hpp"
#include "coordinates.hpp"
#include "time.hpp"
#include <memory>
#include <string>
#include <vector>

namespace siderust {

/**
 * @brief Abstract base for any celestial object that can be tracked from an
 * observer location.
 *
 * Subclasses represent concrete target kinds: fixed sky directions, catalog
 * stars, solar-system bodies, and (in the future) satellites.  All must
 * implement `name()`, `altitude_at()`, and `azimuth_at()`; the search
 * helpers also need overrides.
 */
class Target {
public:
  virtual ~Target() = default;

  // ------------------------------------------------------------------
  // Identity
  // ------------------------------------------------------------------

  /**
   * @brief Human-readable name for this target (e.g. "Sun", "Vega",
   * "ICRS(279.2°, 38.8°)").
   */
  virtual std::string name() const = 0;

  // ------------------------------------------------------------------
  // Altitude queries
  // ------------------------------------------------------------------

  /**
   * @brief Compute altitude (degrees) at a given MJD instant.
   */
  virtual qtty::Degree altitude_at(const Geodetic &obs,
                                   const MJD &mjd) const = 0;

  /**
   * @brief Find periods when the object is above a threshold altitude.
   */
  virtual std::vector<Period>
  above_threshold(const Geodetic &obs, const Period &window,
                  qtty::Degree threshold,
                  const SearchOptions &opts = {}) const = 0;

  /**
   * @brief Find periods when the object is below a threshold altitude.
   */
  virtual std::vector<Period>
  below_threshold(const Geodetic &obs, const Period &window,
                  qtty::Degree threshold,
                  const SearchOptions &opts = {}) const = 0;

  /**
   * @brief Find threshold-crossing events (rising / setting).
   */
  virtual std::vector<CrossingEvent>
  crossings(const Geodetic &obs, const Period &window, qtty::Degree threshold,
            const SearchOptions &opts = {}) const = 0;

  /**
   * @brief Find culmination (local altitude extremum) events.
   */
  virtual std::vector<CulminationEvent>
  culminations(const Geodetic &obs, const Period &window,
               const SearchOptions &opts = {}) const = 0;

  // ------------------------------------------------------------------
  // Azimuth queries
  // ------------------------------------------------------------------

  /**
   * @brief Compute azimuth (degrees, N-clockwise) at a given MJD instant.
   */
  virtual qtty::Degree azimuth_at(const Geodetic &obs,
                                  const MJD &mjd) const = 0;

  /**
   * @brief Find epochs when the object crosses a given azimuth bearing.
   */
  virtual std::vector<AzimuthCrossingEvent>
  azimuth_crossings(const Geodetic &obs, const Period &window,
                    qtty::Degree bearing,
                    const SearchOptions &opts = {}) const = 0;

  // Non-copyable, movable from base
  Target() = default;
  Target(const Target &) = delete;
  Target &operator=(const Target &) = delete;
  Target(Target &&) = default;
  Target &operator=(Target &&) = default;
};

/// @brief Backward-compatible alias.  Prefer `Target` in new code.
using Trackable = Target;

} // namespace siderust
