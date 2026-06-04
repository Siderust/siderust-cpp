#!/usr/bin/env bash
set -euo pipefail

SCRIPT_DIR="$(cd "$(dirname "${BASH_SOURCE[0]}")" && pwd)"
# shellcheck source=scripts/lib.sh
source "${SCRIPT_DIR}/lib.sh"

PACKAGE_DIR="build/packages"

usage() {
  cat <<EOF
Usage: $(basename "$0") [--package-dir DIR]
EOF
}

while [[ $# -gt 0 ]]; do
  case "$1" in
    --package-dir)
      PACKAGE_DIR="$2"
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
require_cmd dpkg-deb

header "Validate package dependency metadata"

shopt -s nullglob
deb_packages=("${PACKAGE_DIR}"/siderust-cpp*.deb)
rpm_packages=("${PACKAGE_DIR}"/siderust-cpp*.rpm)
shopt -u nullglob

if [[ ${#deb_packages[@]} -ne 1 ]]; then
  fail "Expected exactly one siderust-cpp .deb in ${PACKAGE_DIR}, found ${#deb_packages[@]}"
  exit 1
fi

deb_depends="$(dpkg-deb -I "${deb_packages[0]}" | sed -n 's/^ Depends: //p')"
echo "DEB Depends: ${deb_depends}"

for bad in "qtty-cpp (>= 0.8)" "tempoch-cpp (>= 0.6)"; do
  if [[ "${deb_depends}" == *"${bad}"* ]]; then
    fail "DEB metadata contains stale Rust crate-derived dependency: ${bad}"
    exit 1
  fi
done

for required in "qtty-cpp (>= 0.4.5)" "tempoch-cpp (>= 0.5.2)"; do
  if [[ "${deb_depends}" != *"${required}"* ]]; then
    fail "DEB metadata is missing required dependency: ${required}"
    exit 1
  fi
done

if [[ ${#rpm_packages[@]} -gt 0 ]]; then
  require_cmd rpm
  rpm_requires="$(rpm -qp --requires "${rpm_packages[0]}")"
  echo "${rpm_requires}"

  for bad in "qtty-cpp >= 0.8" "tempoch-cpp >= 0.6"; do
    if grep -Fq "${bad}" <<<"${rpm_requires}"; then
      fail "RPM metadata contains stale Rust crate-derived dependency: ${bad}"
      exit 1
    fi
  done

  for required in "qtty-cpp >= 0.4.5" "tempoch-cpp >= 0.5.2"; do
    if ! grep -Fq "${required}" <<<"${rpm_requires}"; then
      fail "RPM metadata is missing required dependency: ${required}"
      exit 1
    fi
  done
else
  warn "No siderust-cpp RPM package found in ${PACKAGE_DIR}; skipped RPM metadata validation"
fi

ok "Package dependency metadata is valid"
