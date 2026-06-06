#!/usr/bin/env bash
set -euo pipefail

SCRIPT_DIR="$(cd "$(dirname "${BASH_SOURCE[0]}")" && pwd)"
# shellcheck source=scripts/lib.sh
source "${SCRIPT_DIR}/lib.sh"

APT_REPO_DIR="pages/apt"
PACKAGE_VERSION="0.7.0"

usage() {
  cat <<EOF
Usage: $(basename "$0") [--apt-repo-dir DIR] [--version VERSION]
EOF
}

while [[ $# -gt 0 ]]; do
  case "$1" in
    --apt-repo-dir)
      APT_REPO_DIR="$2"
      shift 2
      ;;
    --version)
      PACKAGE_VERSION="$2"
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
require_cmd docker

if [[ ! -d "${APT_REPO_DIR}" ]]; then
  fail "APT repository directory does not exist: ${APT_REPO_DIR}"
  exit 1
fi

header "Debian bookworm install smoke test"

docker run --rm \
  -v "$(cd "${APT_REPO_DIR}" && pwd):/repo:ro" \
  debian:bookworm \
  bash -euxo pipefail -c "
    apt-get update
    echo 'deb [trusted=yes] file:/repo ./' > /etc/apt/sources.list.d/siderust-local.list
    apt-get update
    apt-cache policy qtty-cpp tempoch-cpp siderust-cpp
    apt-get install -y siderust-cpp=${PACKAGE_VERSION}
  "

ok "Debian bookworm can install siderust-cpp=${PACKAGE_VERSION}"
