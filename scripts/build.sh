#!/usr/bin/env bash
set -euo pipefail

SCRIPT_DIR="$(cd "$(dirname "${BASH_SOURCE[0]}")" && pwd)"
# shellcheck source=scripts/lib.sh
source "${SCRIPT_DIR}/lib.sh"

BUILD_DIR="build"
BUILD_DOCS="ON"
TARGET="test_tempoch"
PARALLEL_LEVEL="${CMAKE_BUILD_PARALLEL_LEVEL:-2}"

usage() {
  cat <<EOF
Usage: $(basename "$0") [--build-dir DIR] [--docs on|off] [--target NAME] [--parallel N]
EOF
}

while [[ $# -gt 0 ]]; do
  case "$1" in
    --build-dir)
      BUILD_DIR="$2"
      shift 2
      ;;
    --docs)
      case "$2" in
        on|ON) BUILD_DOCS="ON" ;;
        off|OFF) BUILD_DOCS="OFF" ;;
        *) fail "--docs must be on|off"; exit 1 ;;
      esac
      shift 2
      ;;
    --target)
      TARGET="$2"
      shift 2
      ;;
    --parallel)
      PARALLEL_LEVEL="$2"
      shift 2
      ;;
    --help|-h)
      usage
      exit 0
      ;;
    *)
      fail "Unknown argument: $1"
      usage
      exit 1
      ;;
  esac
done

ensure_repo_root
require_cmd cmake
require_cmd ninja

header "Configure: CMake (${BUILD_DIR})"
ensure_fresh_cmake_build_dir "${BUILD_DIR}" "${REPO_ROOT}"
cmake -S . -B "${BUILD_DIR}" -G Ninja -DTEMPOCH_BUILD_DOCS="${BUILD_DOCS}"

header "Build: ${TARGET}"
CMAKE_BUILD_PARALLEL_LEVEL="${PARALLEL_LEVEL}" cmake --build "${BUILD_DIR}" --target "${TARGET}"
ok "Build completed"
