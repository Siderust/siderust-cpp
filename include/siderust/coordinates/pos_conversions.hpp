#pragma once

/**
 * @file pos_conversions.hpp
 * @brief Out-of-line template definitions for cartesian<->spherical conversions.
 */

#include "cartesian.hpp"
#include "spherical.hpp"

namespace siderust {

// cartesian::Position::to_spherical implementation
template <typename C, typename F, typename U>
spherical::Position<C, F, U> cartesian::Position<C, F, U>::to_spherical() const {
  const double x = comp_x.value();
  const double y = comp_y.value();
  const double z = comp_z.value();
  const double r = std::sqrt(x * x + y * y + z * z);
  const double lon = std::atan2(y, x) * 180.0 / M_PI;
  const double lat = std::atan2(z, std::sqrt(x * x + y * y)) * 180.0 / M_PI;
  return spherical::Position<C, F, U>(qtty::Degree(lon), qtty::Degree(lat), U(r));
}

// spherical::Position::to_cartesian implementation
template <typename C, typename F, typename U>
cartesian::Position<C, F, U> spherical::Position<C, F, U>::to_cartesian() const {
  double lon_deg = 0.0;
  double lat_deg = 0.0;
  if constexpr (frames::has_ra_dec_v<F>) {
    lon_deg = azimuth_.value();
    lat_deg = polar_.value();
  } else if constexpr (frames::has_lon_lat_v<F>) {
    lon_deg = azimuth_.value();
    lat_deg = polar_.value();
  } else if constexpr (frames::has_az_alt_v<F>) {
    lon_deg = azimuth_.value();
    lat_deg = polar_.value();
  }

  const double lon = lon_deg * M_PI / 180.0;
  const double lat = lat_deg * M_PI / 180.0;
  const double r = dist_.value();

  const double cx = r * std::cos(lat) * std::cos(lon);
  const double cy = r * std::cos(lat) * std::sin(lon);
  const double cz = r * std::sin(lat);
  return cartesian::Position<C, F, U>(U(cx), U(cy), U(cz));
}

// spherical::Position::to_frame implementation
// Performs the transformation via a cartesian round-trip:
//   spherical(C,F) → cartesian(C,F) → cartesian(C,Target) → spherical(C,Target)
template <typename C, typename F, typename U>
template <typename Target>
std::enable_if_t<siderust::frames::has_frame_transform_v<F, Target>,
                 siderust::spherical::Position<C, Target, U>>
spherical::Position<C, F, U>::to_frame(const JulianDate &jd) const {
  if constexpr (std::is_same_v<F, Target>) {
    return *this;
  } else {
    return to_cartesian().template to_frame<Target>(jd).to_spherical();
  }
}

} // namespace siderust
