#!/usr/bin/env bash
set -euo pipefail

SCRIPT_DIR="$(cd "$(dirname "${BASH_SOURCE[0]}")" && pwd)"
# shellcheck source=scripts/lib.sh
source "${SCRIPT_DIR}/lib.sh"

BUILD_DIR="build"
LABEL="tempoch_cpp"

usage() {
  cat <<EOF
Usage: $(basename "$0") [--build-dir DIR] [--label LABEL]
EOF
}

while [[ $# -gt 0 ]]; do
  case "$1" in
    --build-dir)
      BUILD_DIR="$2"
      shift 2
      ;;
    --label)
      LABEL="$2"
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
require_cmd ctest

header "Test: ctest (${BUILD_DIR})"
ctest --test-dir "${BUILD_DIR}" --output-on-failure -L "${LABEL}"
ok "Tests passed"
