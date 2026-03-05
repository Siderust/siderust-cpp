FROM ubuntu:22.04

ENV DEBIAN_FRONTEND=noninteractive
ENV RUSTUP_HOME=/usr/local/rustup
ENV CARGO_HOME=/usr/local/cargo
ENV PATH=/usr/local/cargo/bin:${PATH}
ENV DOXYGEN_VERSION=1.16.1

RUN apt-get update && apt-get install -y --no-install-recommends \
    build-essential \
    cmake \
    ninja-build \
    git \
    python3 \
    curl \
    ca-certificates \
    pkg-config \
    libssl-dev \
    graphviz \
    rsync \
    clang-format \
    clang-tidy \
    gcovr \
    && rm -rf /var/lib/apt/lists/*

RUN curl -fsSL "https://github.com/doxygen/doxygen/releases/download/Release_1_16_1/doxygen-${DOXYGEN_VERSION}.linux.bin.tar.gz" -o /tmp/doxygen.tar.gz && \
    tar -xzf /tmp/doxygen.tar.gz -C /opt && \
    ln -sf "/opt/doxygen-${DOXYGEN_VERSION}/bin/doxygen" /usr/local/bin/doxygen && \
    rm -f /tmp/doxygen.tar.gz && \
    doxygen --version

RUN curl --proto '=https' --tlsv1.2 -sSf https://sh.rustup.rs | sh -s -- -y --profile minimal --default-toolchain stable

WORKDIR /workspace
COPY . /workspace

# Fail early if required submodules are missing from the build context.
RUN test -f siderust/siderust-ffi/Cargo.toml && \
    test -f tempoch-cpp/tempoch/tempoch-ffi/Cargo.toml && \
    test -f qtty-cpp/CMakeLists.txt

# Validate the container toolchain by configuring, building, testing, and generating docs.
RUN rm -rf build && \
    cmake -S . -B build -G Ninja && \
    cmake --build build --target test_siderust -j"$(nproc)" && \
    ctest --test-dir build --output-on-failure -L siderust_cpp && \
    cmake --build build --target docs -j"$(nproc)"

CMD ["/bin/bash"]
