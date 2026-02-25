#!/usr/bin/env bash
set -euo pipefail

# Local CI runner that mirrors .github/workflows/ci.yml inside Docker

ROOT_DIR="$(cd "$(dirname "${BASH_SOURCE[0]}")" && pwd)"
IMAGE_NAME="siderust-cpp-ci"
IMAGE_TAG="local"
IMAGE="${IMAGE_NAME}:${IMAGE_TAG}"

bold() { printf '\e[1m%s\e[0m\n' "$*"; }

build_image() {
    bold "Building Docker image (${IMAGE})"
    docker build -t "${IMAGE}" "${ROOT_DIR}"
}

run_container() {
    local cmd=$1
    local flags=(--rm --init -v "${ROOT_DIR}:/workspace")
    if [ -t 1 ]; then
        flags+=(-it)
    fi
    docker run "${flags[@]}" "${IMAGE}" bash -lc "${cmd}"
}

container_script() {
    cat <<'EOS'
set -euo pipefail
export CARGO_TERM_COLOR=always
export CMAKE_BUILD_PARALLEL_LEVEL=${CMAKE_BUILD_PARALLEL_LEVEL:-2}

cd /workspace

run_lint() {
    echo "==> Lint: configure + clang-format + clang-tidy"
    rm -rf build
    cmake -S . -B build -G Ninja -DSIDERUST_BUILD_DOCS=OFF -DCMAKE_EXPORT_COMPILE_COMMANDS=ON

    mapfile -t format_files < <(git ls-files '*.hpp' '*.cpp')
    if [ ${#format_files[@]} -gt 0 ]; then
        clang-format --dry-run --Werror "${format_files[@]}"
    else
        echo "No C++ files to format"
    fi

    mapfile -t tidy_files < <(git ls-files '*.cpp')
    if [ ${#tidy_files[@]} -gt 0 ]; then
        for f in "${tidy_files[@]}"; do
            echo "clang-tidy: ${f}"
            clang-tidy -p build --warnings-as-errors='*' "${f}"
        done
    else
        echo "No C++ source files for clang-tidy"
    fi
}

run_build_test_docs() {
    echo "==> Build + Test + Docs"
    rm -rf build
    cmake -S . -B build -G Ninja -DSIDERUST_BUILD_DOCS=ON
    cmake --build build --target test_siderust
    ctest --test-dir build --output-on-failure -L siderust_cpp
    cmake --build build --target docs
}

run_coverage() {
    echo "==> Coverage"
    rm -rf build-coverage coverage.xml coverage_html code-coverage-results.md
    cmake -S . -B build-coverage -G Ninja \
        -DSIDERUST_BUILD_DOCS=OFF \
        -DCMAKE_BUILD_TYPE=Debug \
        -DCMAKE_CXX_FLAGS="--coverage" \
        -DCMAKE_EXE_LINKER_FLAGS="--coverage"

    cmake --build build-coverage --target test_siderust
    ctest --test-dir build-coverage --output-on-failure -L siderust_cpp

    mkdir -p coverage_html
    gcovr \
        --root . \
        --exclude 'build-coverage/.*' \
        --exclude 'siderust/.*' \
        --exclude 'qtty-cpp/.*' \
        --exclude 'tempoch-cpp/.*' \
        --exclude 'tests/.*' \
        --exclude 'examples/.*' \
        --xml \
        --output coverage.xml

    gcovr \
        --root . \
        --exclude 'build-coverage/.*' \
        --exclude 'siderust/.*' \
        --exclude 'qtty-cpp/.*' \
        --exclude 'tempoch-cpp/.*' \
        --exclude 'tests/.*' \
        --exclude 'examples/.*' \
        --html-details \
        --output coverage_html/index.html
}

run_lint
run_build_test_docs
run_coverage

echo "==> All CI steps completed"
EOS
}

build_image
run_container "$(container_script)"
