#!/usr/bin/env bash
set -euo pipefail

SCRIPT_DIR="$(cd "$(dirname "${BASH_SOURCE[0]}")" && pwd)"
# shellcheck source=scripts/lib.sh
source "${SCRIPT_DIR}/lib.sh"

MODE="check"

usage() {
  cat <<EOF
Usage: $(basename "$0") [--check|--fix]

Runs clang-format over C++ sources (wrapper around repo clang_format.sh)
EOF
}

while [[ $# -gt 0 ]]; do
  case "$1" in
    --check)
      MODE="check"
      shift
      ;;
    --fix)
      MODE="fix"
      shift
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

if [[ "${MODE}" == "fix" ]]; then
  ./clang_format.sh --fix
else
  ./clang_format.sh --check
fi

ok "clang-format ${MODE} finished"
