#pragma once

/**
 * @file oem.hpp
 * @brief CCSDS OEM (Orbit Ephemeris Message) parser C++ wrapper.
 *
 * Wraps `siderust_oem_parse_str` to parse a CCSDS OEM v2/v3 document from a
 * `std::string` and return a `std::vector<StateVector>`.
 *
 * ### Typical usage
 * @code
 * #include <siderust/oem.hpp>
 * #include <fstream>
 * #include <sstream>
 *
 * std::ifstream f("orbit.oem");
 * std::ostringstream ss;
 * ss << f.rdbuf();
 * auto states = siderust::oem::parse(ss.str());
 * for (auto& sv : states) {
 *     std::cout << sv.epoch_jd << "  "
 *               << sv.pos_km[0] << " " << sv.pos_km[1] << " " << sv.pos_km[2] << '\n';
 * }
 * @endcode
 */

#include "ffi_core.hpp"

#include <array>
#include <string>
#include <string_view>
#include <vector>

namespace siderust {

/// @defgroup oem OEM parsing
/// @{
namespace oem {

/// A single spacecraft state vector from a CCSDS OEM file.
struct StateVector {
  double epoch_jd;             ///< Epoch as Julian Date.
  std::array<double, 3> pos_km;  ///< Position [x, y, z] in km.
  std::array<double, 3> vel_kms; ///< Velocity [vx, vy, vz] in km/s.
};

/**
 * @brief Parse a CCSDS OEM (KVN) document from a string.
 *
 * All state vectors from all OEM segments are returned in a flat vector,
 * in the order they appear in the document.
 *
 * @param text  OEM document text (null termination added internally).
 * @return std::vector<StateVector>  Parsed state vectors (may be empty).
 *
 * @throws siderust::InvalidArgumentError  if the OEM document is malformed.
 */
inline std::vector<StateVector> parse(std::string_view text) {
  const std::string buf{text};
  SiderustOemState *raw_ptr = nullptr;
  unsigned long     count   = 0;

  check_status(siderust_oem_parse_str(buf.c_str(), &raw_ptr, &count), "oem::parse");

  std::vector<StateVector> result;
  result.reserve(static_cast<std::size_t>(count));
  for (unsigned long i = 0; i < count; ++i) {
    const auto &s = raw_ptr[i];
    result.push_back({s.epoch_jd,
                      {s.pos_km[0], s.pos_km[1], s.pos_km[2]},
                      {s.vel_kms[0], s.vel_kms[1], s.vel_kms[2]}});
  }

  siderust_oem_states_free(raw_ptr, count);
  return result;
}

} // namespace oem
/// @}

} // namespace siderust
