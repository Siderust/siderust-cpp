#pragma once

/**
 * @file star_target.hpp
 * @brief Target implementation for Star catalog objects.
 *
 * `StarTarget` wraps a `const Star&` and implements the `Target`
 * interface by delegating to the `star_altitude::` namespace free functions.
 *
 * ### Example
 * @code
 * siderust::StarTarget vega_target(siderust::VEGA);
 * std::cout << vega_target.name() << "\n";  // "Vega"
 * auto alt = vega_target.altitude_at(obs, now);
 * @endcode
 */

#include "altitude.hpp"
#include "azimuth.hpp"
#include "bodies.hpp"
#include "trackable.hpp"

namespace siderust {

/**
 * @brief Target implementation wrapping a `const Star&`.
 *
 * The referenced `Star` must outlive the `StarTarget`. Typically used with
 * the pre-built catalog stars (e.g. `VEGA`, `SIRIUS`) which are `inline const`
 * globals and live for the entire program.
 */
class StarTarget : public Target {
public:
  /**
   * @brief Wrap a Star reference as a Target.
   * @param star Reference to a Star. Must outlive this adapter.
   */
  explicit StarTarget(const Star &star) : star_(star) {}

  // ------------------------------------------------------------------
  // Identity (implements Target)
  // ------------------------------------------------------------------

  /**
   * @brief Returns the star's catalog name (delegates to `Star::name()`).
   */
  std::string name() const override { return star_.name(); }

  // ------------------------------------------------------------------
  // Altitude queries
  // ------------------------------------------------------------------

  qtty::Degree altitude_at(const Geodetic &obs, const MJD &mjd) const override {
    // star_altitude::altitude_at returns Radian; convert to Degree
    auto rad = star_altitude::altitude_at(star_, obs, mjd);
    return qtty::Degree(rad.value() * 180.0 / 3.14159265358979323846);
  }

  std::vector<Period>
  above_threshold(const Geodetic &obs, const Period &window,
                  qtty::Degree threshold,
                  const SearchOptions &opts = {}) const override {
    return star_altitude::above_threshold(star_, obs, window, threshold, opts);
  }

  std::vector<Period>
  below_threshold(const Geodetic &obs, const Period &window,
                  qtty::Degree threshold,
                  const SearchOptions &opts = {}) const override {
    return star_altitude::below_threshold(star_, obs, window, threshold, opts);
  }

  std::vector<CrossingEvent>
  crossings(const Geodetic &obs, const Period &window, qtty::Degree threshold,
            const SearchOptions &opts = {}) const override {
    return star_altitude::crossings(star_, obs, window, threshold, opts);
  }

  std::vector<CulminationEvent>
  culminations(const Geodetic &obs, const Period &window,
               const SearchOptions &opts = {}) const override {
    return star_altitude::culminations(star_, obs, window, opts);
  }

  // ------------------------------------------------------------------
  // Azimuth queries
  // ------------------------------------------------------------------

  qtty::Degree azimuth_at(const Geodetic &obs, const MJD &mjd) const override {
    return star_altitude::azimuth_at(star_, obs, mjd);
  }

  std::vector<AzimuthCrossingEvent>
  azimuth_crossings(const Geodetic &obs, const Period &window,
                    qtty::Degree bearing,
                    const SearchOptions &opts = {}) const override {
    return star_altitude::azimuth_crossings(star_, obs, window, bearing, opts);
  }

  /// Access the underlying Star reference.
  const Star &star() const { return star_; }

private:
  const Star &star_;
};

} // namespace siderust
