#!/usr/bin/env bash
set -euo pipefail

SCRIPT_DIR="$(cd "$(dirname "${BASH_SOURCE[0]}")" && pwd)"
REPO_ROOT="$(cd "${SCRIPT_DIR}/.." && pwd)"

blue='\033[0;34m'
green='\033[0;32m'
yellow='\033[1;33m'
red='\033[0;31m'
reset='\033[0m'

header() {
  echo -e "\n${blue}================================================${reset}"
  echo -e "${blue}$*${reset}"
  echo -e "${blue}================================================${reset}\n"
}

ok() {
  echo -e "${green}[ok]${reset} $*"
}

warn() {
  echo -e "${yellow}[warn]${reset} $*"
}

fail() {
  echo -e "${red}[fail]${reset} $*"
}

ensure_repo_root() {
  cd "${REPO_ROOT}"
}

ensure_fresh_cmake_build_dir() {
  local build_dir="$1"
  local expected_source_dir="$2"
  local cache_file="${build_dir}/CMakeCache.txt"

  if [[ ! -f "${cache_file}" ]]; then
    return 0
  fi

  local cached_source_dir
  cached_source_dir="$(sed -n 's#^CMAKE_HOME_DIRECTORY:INTERNAL=##p' "${cache_file}" | head -1)"

  if [[ -n "${cached_source_dir}" && "${cached_source_dir}" != "${expected_source_dir}" ]]; then
    warn "Removing stale CMake build directory: ${build_dir}"
    rm -rf "${build_dir}"
  fi
}

require_cmd() {
  local cmd="$1"
  if ! command -v "${cmd}" >/dev/null 2>&1; then
    fail "Missing required command: ${cmd}"
    return 1
  fi
}
