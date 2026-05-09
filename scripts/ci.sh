#!/usr/bin/env bash
set -euo pipefail

SCRIPT_DIR="$(cd "$(dirname "${BASH_SOURCE[0]}")" && pwd)"
# shellcheck source=scripts/lib.sh
source "${SCRIPT_DIR}/lib.sh"

RUN_FMT=false
RUN_BUILD=false
RUN_TEST=false
RUN_DOCS=false
RUN_COVERAGE=false
FMT_MODE="check"
PARALLEL_LEVEL="${CMAKE_BUILD_PARALLEL_LEVEL:-2}"

usage() {
  cat <<EOF
Usage: $(basename "$0") [all|fmt|build|test|docs|coverage] [options]

Commands:
  all       Run fmt, build, test, docs, coverage (default)
  fmt       Run lint/format checks
  build     Configure and build test target
  test      Run ctest for build directory
  docs      Build documentation target
  coverage  Run coverage pipeline

Options:
  --fmt-mode MODE   check|fix (default: check)
  --parallel N      Set build parallelism (default: 2)
  --help            Show this help

Examples:
  $(basename "$0")
  $(basename "$0") all --fmt-mode check
  $(basename "$0") fmt --fmt-mode fix
  $(basename "$0") build test
EOF
}

if [[ $# -eq 0 ]]; then
  RUN_FMT=true
  RUN_BUILD=true
  RUN_TEST=true
  RUN_DOCS=true
  RUN_COVERAGE=true
fi

while [[ $# -gt 0 ]]; do
  case "$1" in
    all)
      RUN_FMT=true
      RUN_BUILD=true
      RUN_TEST=true
      RUN_DOCS=true
      RUN_COVERAGE=true
      shift
      ;;
    fmt)
      RUN_FMT=true
      shift
      ;;
    build)
      RUN_BUILD=true
      shift
      ;;
    test)
      RUN_TEST=true
      shift
      ;;
    docs)
      RUN_DOCS=true
      shift
      ;;
    coverage)
      RUN_COVERAGE=true
      shift
      ;;
    --fmt-mode)
      FMT_MODE="$2"
      if [[ "${FMT_MODE}" != "check" && "${FMT_MODE}" != "fix" ]]; then
        fail "--fmt-mode must be check|fix"
        exit 1
      fi
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

header "Submodule check"
git submodule status --recursive || true
if [[ ! -f tempoch/Cargo.toml ]]; then
  fail "tempoch submodule missing (run: git submodule update --init --recursive)"
  exit 1
fi
ok "tempoch submodule present"

if [[ "${RUN_FMT}" == "true" ]]; then
  "${SCRIPT_DIR}/fmt.sh" "--${FMT_MODE}"
fi

if [[ "${RUN_BUILD}" == "true" ]]; then
  "${SCRIPT_DIR}/build.sh" --parallel "${PARALLEL_LEVEL}"
fi

if [[ "${RUN_TEST}" == "true" ]]; then
  "${SCRIPT_DIR}/test.sh"
fi

if [[ "${RUN_DOCS}" == "true" ]]; then
  "${SCRIPT_DIR}/docs.sh"
fi

if [[ "${RUN_COVERAGE}" == "true" ]]; then
  "${SCRIPT_DIR}/coverage.sh" --parallel "${PARALLEL_LEVEL}"
fi

ok "CI scripts finished"
