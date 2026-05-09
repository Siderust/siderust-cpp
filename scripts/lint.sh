#!/usr/bin/env bash
set -euo pipefail

SCRIPT_DIR="$(cd "$(dirname "${BASH_SOURCE[0]}")" && pwd)"
# shellcheck source=scripts/lib.sh
source "${SCRIPT_DIR}/lib.sh"

MODE="check"

usage() {
  cat <<EOF
Usage: $(basename "$0") [--check|--fix]

Options:
  --check   Verify formatting and run clang-tidy checks (default)
  --fix     Apply clang-format changes in place, then run clang-tidy checks
  --help    Show this help
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
require_cmd git
require_cmd cmake
require_cmd clang-format
require_cmd clang-tidy

header "Lint: clang-format (${MODE})"
mapfile -t cpp_like_files < <(git ls-files '*.hpp' '*.cpp')

if [[ ${#cpp_like_files[@]} -eq 0 ]]; then
  warn "No C++ files found"
else
  if [[ "${MODE}" == "fix" ]]; then
    clang-format -i "${cpp_like_files[@]}"
    ok "Applied clang-format fixes"
  else
    clang-format --dry-run --Werror "${cpp_like_files[@]}"
    ok "clang-format check passed"
  fi
fi

header "Lint: clang-tidy"
ensure_fresh_cmake_build_dir "build" "${REPO_ROOT}"
cmake -S . -B build -G Ninja -DTEMPOCH_BUILD_DOCS=OFF -DCMAKE_EXPORT_COMPILE_COMMANDS=ON >/dev/null

mapfile -t cpp_files < <(git ls-files '*.cpp')
if [[ ${#cpp_files[@]} -eq 0 ]]; then
  warn "No C++ source files found"
  exit 0
fi

for file in "${cpp_files[@]}"; do
  echo "Running clang-tidy on ${file}"
  clang-tidy -p build --warnings-as-errors='*' "${file}"
done
ok "clang-tidy checks passed"
