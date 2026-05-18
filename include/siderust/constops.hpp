#pragma once

#include <cstddef>
#include <cstdint>
#include <stdexcept>
#include <string>
#include <utility>

extern "C" {
#include "constops.h"
}

namespace siderust::constops {

inline constexpr const char *kRoutePassesGenerate = "/v1/passes/generate";
inline constexpr const char *kRouteLinksScore = "/v1/links/score";
inline constexpr const char *kRouteSchedulesSolve = "/v1/schedules/solve";
inline constexpr const char *kRouteSchedulesValidate = "/v1/schedules/validate";
inline constexpr const char *kRouteOrbitStatesIngest = "/v1/orbit-states/ingest";

class ConstopsError : public std::runtime_error {
public:
  explicit ConstopsError(const std::string &message) : std::runtime_error(message) {}
};

inline std::string format_ground_asset_id(const std::string &provider, const std::string &station) {
  if (provider.empty() || station.empty()) {
    throw ConstopsError("ground asset id parts must be non-empty");
  }
  if (provider.find(':') != std::string::npos) {
    throw ConstopsError("ground asset provider must not contain ':'");
  }
  return provider + ":" + station;
}

inline std::pair<std::string, std::string> parse_ground_asset_id(const std::string &id) {
  const auto split = id.find(':');
  if (split == std::string::npos || split == 0 || split == id.size() - 1) {
    throw ConstopsError("invalid ground asset id; expected 'provider:station'");
  }
  return {id.substr(0, split), id.substr(split + 1)};
}

using ConstopsEntryPoint = int32_t (*)(const uint8_t *, size_t, ConstopsBuffer *);

inline std::string call_json(ConstopsEntryPoint entrypoint, const std::string &request_json,
                             const char *operation_name) {
  ConstopsBuffer out{nullptr, 0};
  const auto *req = reinterpret_cast<const uint8_t *>(request_json.data());
  const auto rc = entrypoint(req, request_json.size(), &out);
  if (rc != CONSTOPS_OK) {
    const char *last = constops_last_error();
    std::string detail = (last == nullptr) ? "unknown constops error" : last;
    throw ConstopsError(std::string(operation_name) + " failed (" + std::to_string(rc) + "): " +
                        detail);
  }

  std::string response;
  if (out.data != nullptr && out.len > 0) {
    response.assign(reinterpret_cast<const char *>(out.data), out.len);
  }
  constops_buffer_free(out);
  return response;
}

inline std::string passes_generate_json(const std::string &request_json) {
  return call_json(constops_passes_generate, request_json, "constops_passes_generate");
}

inline std::string links_score_json(const std::string &request_json) {
  return call_json(constops_links_score, request_json, "constops_links_score");
}

inline std::string schedule_solve_json(const std::string &request_json) {
  return call_json(constops_schedule_solve, request_json, "constops_schedule_solve");
}

inline std::string schedule_validate_json(const std::string &request_json) {
  return call_json(constops_schedule_validate, request_json, "constops_schedule_validate");
}

inline std::string orbit_ingest_json(const std::string &request_json) {
  return call_json(constops_orbit_ingest, request_json, "constops_orbit_ingest");
}

} // namespace siderust::constops
