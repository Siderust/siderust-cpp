cmake_minimum_required(VERSION 3.21)
project(siderust_cpp VERSION 0.10.0 LANGUAGES CXX)

# ---------------------------------------------------------------------------
# CMake 3.21+ provides PROJECT_IS_TOP_LEVEL; emulate for older CMake if needed.
# ---------------------------------------------------------------------------
if(NOT DEFINED PROJECT_IS_TOP_LEVEL)
    if(CMAKE_SOURCE_DIR STREQUAL CMAKE_CURRENT_SOURCE_DIR)
        set(PROJECT_IS_TOP_LEVEL TRUE)
    else()
        set(PROJECT_IS_TOP_LEVEL FALSE)
    endif()
endif()

set(CMAKE_CXX_STANDARD 17)
set(CMAKE_CXX_STANDARD_REQUIRED ON)
set(CMAKE_CXX_EXTENSIONS OFF)

# ---------------------------------------------------------------------------
# Top-level options (safe defaults when consumed via add_subdirectory)
# ---------------------------------------------------------------------------
option(SIDERUST_BUILD_DOCS         "Enable Doxygen documentation target."          ${PROJECT_IS_TOP_LEVEL})
option(SIDERUST_CPP_BUILD_TESTS    "Build siderust-cpp unit tests."                ${PROJECT_IS_TOP_LEVEL})
option(SIDERUST_CPP_BUILD_BENCHES  "Build siderust-cpp Google Benchmark suites."   ${PROJECT_IS_TOP_LEVEL})
option(SIDERUST_CPP_BUILD_EXAMPLES "Build siderust-cpp example programs."          ${PROJECT_IS_TOP_LEVEL})
option(SIDERUST_CPP_INSTALL        "Generate install rules for siderust-cpp."      ${PROJECT_IS_TOP_LEVEL})
option(SIDERUST_CPP_ENABLE_PACKAGING "Enable CPack (.deb / .rpm) packaging."       ${PROJECT_IS_TOP_LEVEL})

# Find Cargo for building Rust libraries
find_program(CARGO_BIN cargo REQUIRED)

# ---------------------------------------------------------------------------
# Paths to siderust-ffi. In the products monorepo, prefer the sibling Rust
# checkout so siderust-cpp builds against local Rust changes. Standalone
# checkouts keep using the nested siderust submodule by default.
# ---------------------------------------------------------------------------
set(_SIDERUST_DEFAULT_RUST_DIR ${CMAKE_CURRENT_SOURCE_DIR}/siderust)
if(EXISTS "${CMAKE_CURRENT_SOURCE_DIR}/../../rust/siderust/siderust-ffi/Cargo.toml")
    set(_SIDERUST_DEFAULT_RUST_DIR ${CMAKE_CURRENT_SOURCE_DIR}/../../rust/siderust)
endif()
set(SIDERUST_RUST_DIR ${_SIDERUST_DEFAULT_RUST_DIR}
    CACHE PATH "Path to the siderust Rust workspace used to build siderust-ffi")
set(SIDERUST_FFI_DIR ${SIDERUST_RUST_DIR}/siderust-ffi)
set(SIDERUST_FFI_INCLUDE_DIR ${SIDERUST_FFI_DIR}/include)

# Cargo features for siderust-ffi (e.g., "serde,de441")
set(SIDERUST_FFI_FEATURES "" CACHE STRING "Cargo features for siderust-ffi")
set(_SIDERUST_FEATURES_ARGS "")
string(TOUPPER "${SIDERUST_FFI_FEATURES}" _SIDERUST_FEATURES_UP)
if("${_SIDERUST_FEATURES_UP}" STREQUAL "OFF" OR "${_SIDERUST_FEATURES_UP}" STREQUAL "FALSE" OR "${_SIDERUST_FEATURES_UP}" STREQUAL "0")
    # No features
elseif(NOT "${SIDERUST_FFI_FEATURES}" STREQUAL "")
    set(_SIDERUST_FEATURES_ARGS --features ${SIDERUST_FFI_FEATURES})
endif()

if(SIDERUST_FFI_FEATURES MATCHES "(^|,)satellite(,|$)")
    message(FATAL_ERROR
        "The satellite feature and C++ dynamics adapter have been removed from siderust-cpp. "
        "Remove 'satellite' from SIDERUST_FFI_FEATURES."
    )
endif()

# ---------------------------------------------------------------------------
# Platform-specific library paths
# ---------------------------------------------------------------------------
set(SIDERUST_ARTIFACT_DIR ${SIDERUST_RUST_DIR}/target/release)

if(APPLE)
    set(SIDERUST_LIBRARY_NAME libsiderust_ffi.dylib)
elseif(WIN32)
    set(SIDERUST_LIBRARY_NAME siderust_ffi.dll)
    set(SIDERUST_IMPORT_LIBRARY ${SIDERUST_ARTIFACT_DIR}/siderust_ffi.dll.lib)
else()
    set(SIDERUST_LIBRARY_NAME libsiderust_ffi.so)
endif()
set(SIDERUST_LIBRARY_PATH ${SIDERUST_ARTIFACT_DIR}/${SIDERUST_LIBRARY_NAME})

# ---------------------------------------------------------------------------
# Build siderust-ffi via Cargo
# ---------------------------------------------------------------------------
add_custom_target(
    build_siderust_ffi
    # Use `cargo rustc --crate-type cdylib` so that the shared library is
    # produced even though Cargo.toml only lists rlib (which keeps coverage
    # instrumentation clean during `cargo test`/`cargo llvm-cov`).
    COMMAND ${CARGO_BIN} rustc --release --crate-type cdylib ${_SIDERUST_FEATURES_ARGS}
    WORKING_DIRECTORY ${SIDERUST_FFI_DIR}
    BYPRODUCTS ${SIDERUST_LIBRARY_PATH}
    DEPENDS build_tempoch_ffi
    COMMENT "Building siderust-ffi via Cargo (cdylib override)"
    VERBATIM
)

# Import siderust_ffi (build-tree imported target)
add_library(siderust_ffi SHARED IMPORTED GLOBAL)
set_target_properties(siderust_ffi PROPERTIES IMPORTED_LOCATION ${SIDERUST_LIBRARY_PATH})
if(WIN32)
    set_target_properties(siderust_ffi PROPERTIES IMPORTED_IMPLIB ${SIDERUST_IMPORT_LIBRARY})
endif()
add_dependencies(siderust_ffi build_siderust_ffi)

# ---------------------------------------------------------------------------
# Header-only C++ wrapper library
# ---------------------------------------------------------------------------
# Pull in qtty-cpp (unit-safe quantities) via the nested tempoch-cpp checkout
set(QTTY_FFI_FEATURES "" CACHE STRING "Cargo features for qtty-ffi" FORCE)
add_subdirectory(tempoch-cpp/qtty-cpp)

# Pull in tempoch-cpp (time types) as a subdirectory
set(TEMPOCH_FFI_FEATURES "" CACHE STRING "Cargo features for tempoch-ffi" FORCE)
add_subdirectory(tempoch-cpp)

# Paths to qtty-ffi shared library (for RPATH)
set(QTTY_SUBMODULE_DIR ${CMAKE_CURRENT_SOURCE_DIR}/tempoch-cpp/qtty-cpp/qtty)
set(QTTY_ARTIFACT_DIR  ${QTTY_SUBMODULE_DIR}/target/release)

# Paths to tempoch-ffi shared library (for RPATH)
set(TEMPOCH_SUBMODULE_DIR_PARENT ${CMAKE_CURRENT_SOURCE_DIR}/tempoch-cpp/tempoch)
set(TEMPOCH_ARTIFACT_DIR  ${TEMPOCH_SUBMODULE_DIR_PARENT}/tempoch-ffi/target/release)

add_library(siderust_cpp INTERFACE)
add_library(siderust::siderust_cpp ALIAS siderust_cpp)
target_include_directories(siderust_cpp INTERFACE
    $<BUILD_INTERFACE:${CMAKE_CURRENT_SOURCE_DIR}/include>
    $<BUILD_INTERFACE:${SIDERUST_FFI_INCLUDE_DIR}>
    $<INSTALL_INTERFACE:include>
)
# Build-tree linking uses the in-tree imported targets. Install-tree linking
# uses the imported targets defined in siderust_cppConfig.cmake (which point
# at the installed shared library) plus the qtty / tempoch package configs.
target_link_libraries(siderust_cpp INTERFACE
    $<BUILD_INTERFACE:siderust_ffi>
    $<BUILD_INTERFACE:tempoch_cpp>
    $<BUILD_INTERFACE:qtty_cpp>
    $<INSTALL_INTERFACE:siderust::siderust_ffi>
    $<INSTALL_INTERFACE:tempoch::tempoch_cpp>
    $<INSTALL_INTERFACE:qtty::qtty_cpp>
)
add_dependencies(siderust_cpp build_siderust_ffi)

# ---------------------------------------------------------------------------
# Doxygen documentation
# ---------------------------------------------------------------------------
if(SIDERUST_BUILD_DOCS)
    find_package(Doxygen QUIET)
    if(DOXYGEN_FOUND)
        set(SIDERUST_DOXYFILE_IN  ${CMAKE_CURRENT_SOURCE_DIR}/docs/Doxyfile.in)
        set(SIDERUST_DOXYFILE_OUT ${CMAKE_CURRENT_BINARY_DIR}/Doxyfile.siderust_cpp)
        configure_file(${SIDERUST_DOXYFILE_IN} ${SIDERUST_DOXYFILE_OUT} @ONLY)

        if(NOT TARGET docs)
            add_custom_target(docs
                COMMAND ${CMAKE_COMMAND} -E make_directory ${CMAKE_CURRENT_BINARY_DIR}/docs/doxygen
                COMMAND ${DOXYGEN_EXECUTABLE} ${SIDERUST_DOXYFILE_OUT}
                WORKING_DIRECTORY ${CMAKE_CURRENT_SOURCE_DIR}
                COMMENT "Generating API documentation with Doxygen"
                VERBATIM
            )
        else()
            message(STATUS "Top-level 'docs' target already exists; skipping creation to avoid conflict with subprojects")
        endif()
    else()
        message(STATUS "Doxygen not found; 'docs' target will not be available")
    endif()
endif()

# RPATH for shared library lookup at runtime (build tree).
if(APPLE)
    set(_siderust_rpath "${SIDERUST_ARTIFACT_DIR};@loader_path/../tempoch-cpp/tempoch/tempoch-ffi/target/release;@loader_path/../tempoch-cpp/qtty-cpp/qtty/target/release")
elseif(UNIX)
    set(_siderust_rpath "${SIDERUST_ARTIFACT_DIR}:$ORIGIN/../tempoch-cpp/tempoch/tempoch-ffi/target/release:$ORIGIN/../tempoch-cpp/qtty-cpp/qtty/target/release")
endif()

# ---------------------------------------------------------------------------
# Google Benchmark (benches only)
# ---------------------------------------------------------------------------
if(SIDERUST_CPP_BUILD_BENCHES)
    include(FetchContent)
    FetchContent_Declare(
        benchmark
        GIT_REPOSITORY https://github.com/google/benchmark.git
        GIT_TAG v1.8.5
    )
    set(BENCHMARK_ENABLE_TESTING OFF CACHE BOOL "" FORCE)
    set(BENCHMARK_ENABLE_INSTALL OFF CACHE BOOL "" FORCE)
    FetchContent_MakeAvailable(benchmark)
endif()

# ---------------------------------------------------------------------------
# Google Test (tests only)
# ---------------------------------------------------------------------------
if(SIDERUST_CPP_BUILD_TESTS)
    include(FetchContent)
    FetchContent_Declare(
        googletest
        GIT_REPOSITORY https://github.com/google/googletest.git
        GIT_TAG v1.14.0
    )
    set(gtest_force_shared_crt ON CACHE BOOL "" FORCE)
    # Prevent GoogleTest artifacts from being picked up by cmake --install / CPack.
    set(INSTALL_GTEST OFF CACHE BOOL "" FORCE)
    set(INSTALL_GMOCK OFF CACHE BOOL "" FORCE)
    FetchContent_MakeAvailable(googletest)

    enable_testing()
endif()

# ---------------------------------------------------------------------------
# Example helper: add an example only if its source file exists.
# ---------------------------------------------------------------------------
macro(siderust_add_example target_name source_file)
    if(EXISTS "${CMAKE_CURRENT_SOURCE_DIR}/${source_file}")
        add_executable(${target_name} ${source_file})
        target_link_libraries(${target_name} PRIVATE siderust_cpp)
        if(DEFINED _siderust_rpath)
            set_target_properties(${target_name} PROPERTIES
                BUILD_RPATH ${_siderust_rpath}
                INSTALL_RPATH ${_siderust_rpath}
            )
        endif()
    endif()
endmacro()

# ---------------------------------------------------------------------------
# Examples
# ---------------------------------------------------------------------------
if(SIDERUST_CPP_BUILD_EXAMPLES)
    # Numbered mirror examples (01–21, mirroring siderust Rust examples)
    siderust_add_example(01_basic_coordinates_example          examples/01_basic_coordinates.cpp)
    siderust_add_example(02_coordinate_transformations_example examples/02_coordinate_transformations.cpp)
    siderust_add_example(03_all_frame_conversions_example   examples/03_all_frames_conversions.cpp)
    siderust_add_example(04_all_center_conversions_example  examples/04_all_center_conversions.cpp)
    siderust_add_example(05_target_tracking_example         examples/05_target_tracking.cpp)
    siderust_add_example(06_night_events_example            examples/06_night_events.cpp)
    siderust_add_example(07_moon_properties_example         examples/07_moon_properties.cpp)
    siderust_add_example(08_solar_system_example            examples/08_solar_system.cpp)
    siderust_add_example(09_star_observability_example      examples/09_star_observability.cpp)
    siderust_add_example(10_time_periods_example            examples/10_time_periods.cpp)
    siderust_add_example(11_serde_serialization_example     examples/11_serde_serialization.cpp)
    siderust_add_example(12_runtime_ephemeris_example       examples/12_runtime_ephemeris.cpp)
    siderust_add_example(13_coordinate_operations_example   examples/13_coordinate_operations.cpp)
    siderust_add_example(14_nutation_models_example         examples/14_nutation_models.cpp)
    siderust_add_example(15_orbit_models_example            examples/15_orbit_models.cpp)
    siderust_add_example(16_lambert_earth_to_mars_example   examples/16_lambert_earth_to_mars.cpp)
    siderust_add_example(17_sgp4_from_tle_example           examples/17_sgp4_from_tle.cpp)
    siderust_add_example(18_lisa_pod_example                examples/18_lisa_pod.cpp)
    siderust_add_example(19_sky_grid_sampling_example       examples/19_sky_grid_sampling.cpp)
    if(TARGET 18_lisa_pod_example)
        target_compile_definitions(18_lisa_pod_example PRIVATE
            SIDERUST_TEST_DATA_DIR="${SIDERUST_RUST_DIR}/tests/test-data/lisa"
        )
    endif()
endif()

# ---------------------------------------------------------------------------
# Benchmarks
# ---------------------------------------------------------------------------
if(SIDERUST_CPP_BUILD_BENCHES)
    add_executable(bench_night_periods benches/bench_night_periods.cpp)
    target_link_libraries(bench_night_periods PRIVATE siderust_cpp benchmark::benchmark)

    add_executable(bench_icrs_altitude_periods benches/bench_icrs_altitude_periods.cpp)
    target_link_libraries(bench_icrs_altitude_periods PRIVATE siderust_cpp benchmark::benchmark)

    if(DEFINED _siderust_rpath)
        set_target_properties(bench_night_periods PROPERTIES
            BUILD_RPATH ${_siderust_rpath}
            INSTALL_RPATH ${_siderust_rpath}
        )
        set_target_properties(bench_icrs_altitude_periods PROPERTIES
            BUILD_RPATH ${_siderust_rpath}
            INSTALL_RPATH ${_siderust_rpath}
        )
    endif()
endif()

# ---------------------------------------------------------------------------
# Tests
# ---------------------------------------------------------------------------
if(SIDERUST_CPP_BUILD_TESTS)
    set(TEST_SOURCES
        tests/main.cpp
        tests/test_time.cpp
        tests/test_observatories.cpp
        tests/test_coordinates.cpp
        tests/test_bodies.cpp
        tests/test_altitude.cpp
        tests/test_ephemeris.cpp
        tests/test_bodycentric.cpp
        tests/test_orbits.cpp
        tests/test_subject.cpp
        tests/test_context.cpp
        tests/test_twilight.cpp
        tests/test_lambert.cpp
        tests/test_sgp4.cpp
        tests/test_sky_grid.cpp
        tests/test_oem.cpp
        tests/test_stream.cpp
    )

    add_executable(test_siderust ${TEST_SOURCES})
    target_compile_definitions(test_siderust PRIVATE
        SIDERUST_SOURCE_DIR="${CMAKE_CURRENT_SOURCE_DIR}"
    )
    target_link_libraries(test_siderust PRIVATE siderust_cpp GTest::gtest)
    if(DEFINED _siderust_rpath)
        set_target_properties(test_siderust PROPERTIES
            BUILD_RPATH ${_siderust_rpath}
            INSTALL_RPATH ${_siderust_rpath}
        )
    endif()

    include(GoogleTest)
    gtest_discover_tests(test_siderust
        PROPERTIES LABELS "siderust_cpp"
    )
endif()

# ---------------------------------------------------------------------------
# Installation
# ---------------------------------------------------------------------------
if(SIDERUST_CPP_INSTALL)
    include(GNUInstallDirs)

    install(DIRECTORY include/siderust
        DESTINATION ${CMAKE_INSTALL_INCLUDEDIR}
        COMPONENT siderust-cpp
        FILES_MATCHING PATTERN "*.hpp"
    )

    install(DIRECTORY ${SIDERUST_FFI_INCLUDE_DIR}/
        DESTINATION ${CMAKE_INSTALL_INCLUDEDIR}
        COMPONENT siderust-cpp
        FILES_MATCHING PATTERN "*.h"
    )

    get_target_property(_siderust_qtty_ffi_library qtty_ffi IMPORTED_LOCATION)
    get_target_property(_siderust_tempoch_ffi_library tempoch_ffi IMPORTED_LOCATION)

    if(WIN32)
        get_target_property(_siderust_qtty_ffi_implib qtty_ffi IMPORTED_IMPLIB)
        get_target_property(_siderust_tempoch_ffi_implib tempoch_ffi IMPORTED_IMPLIB)

        install(FILES ${SIDERUST_LIBRARY_PATH}
            DESTINATION ${CMAKE_INSTALL_BINDIR}
            COMPONENT siderust-cpp
        )
        install(FILES ${SIDERUST_IMPORT_LIBRARY}
            DESTINATION ${CMAKE_INSTALL_LIBDIR}
            COMPONENT siderust-cpp
        )
        install(FILES ${_siderust_qtty_ffi_library} ${_siderust_tempoch_ffi_library}
            DESTINATION ${CMAKE_INSTALL_BINDIR}
            COMPONENT siderust-cpp
        )
        install(FILES ${_siderust_qtty_ffi_implib} ${_siderust_tempoch_ffi_implib}
            DESTINATION ${CMAKE_INSTALL_LIBDIR}
            COMPONENT siderust-cpp
        )
        set(SIDERUST_FFI_INSTALLED_IMPLIB_NAME "siderust_ffi.dll.lib")
    else()
        install(FILES ${SIDERUST_LIBRARY_PATH}
            DESTINATION ${CMAKE_INSTALL_LIBDIR}
            COMPONENT siderust-cpp
        )
        install(FILES ${_siderust_qtty_ffi_library} ${_siderust_tempoch_ffi_library}
            DESTINATION ${CMAKE_INSTALL_LIBDIR}
            COMPONENT siderust-cpp
        )
    endif()

    include(CMakePackageConfigHelpers)

    # Expose installed artifact names so the package config can reconstruct paths.
    set(SIDERUST_FFI_INSTALLED_LIB_NAME "${SIDERUST_LIBRARY_NAME}")

    configure_package_config_file(
        ${CMAKE_CURRENT_SOURCE_DIR}/cmake/siderust_cppConfig.cmake.in
        ${CMAKE_CURRENT_BINARY_DIR}/siderust_cppConfig.cmake
        INSTALL_DESTINATION ${CMAKE_INSTALL_LIBDIR}/cmake/siderust_cpp
        PATH_VARS CMAKE_INSTALL_INCLUDEDIR CMAKE_INSTALL_LIBDIR CMAKE_INSTALL_BINDIR
    )

    write_basic_package_version_file(
        ${CMAKE_CURRENT_BINARY_DIR}/siderust_cppConfigVersion.cmake
        VERSION ${PROJECT_VERSION}
        COMPATIBILITY SameMajorVersion
    )

    install(FILES
        ${CMAKE_CURRENT_BINARY_DIR}/siderust_cppConfig.cmake
        ${CMAKE_CURRENT_BINARY_DIR}/siderust_cppConfigVersion.cmake
        DESTINATION ${CMAKE_INSTALL_LIBDIR}/cmake/siderust_cpp
        COMPONENT siderust-cpp
    )

    install(TARGETS siderust_cpp
        EXPORT siderust_cppTargets
        INCLUDES DESTINATION ${CMAKE_INSTALL_INCLUDEDIR}
    )

    install(EXPORT siderust_cppTargets
        FILE siderust_cppTargets.cmake
        NAMESPACE siderust::
        DESTINATION ${CMAKE_INSTALL_LIBDIR}/cmake/siderust_cpp
        COMPONENT siderust-cpp
    )
endif()

# ---------------------------------------------------------------------------
# Packaging (CPack — DEB and RPM)
# ---------------------------------------------------------------------------
if(SIDERUST_CPP_ENABLE_PACKAGING AND SIDERUST_CPP_INSTALL)
    set(CPACK_PACKAGE_NAME             "siderust-cpp")
    set(CPACK_PACKAGE_VERSION          "${PROJECT_VERSION}")
    set(CPACK_PACKAGE_VENDOR           "Siderust")
    set(CPACK_PACKAGE_CONTACT          "VPRamon <vallespuigramon@gmail.com>")
    set(CPACK_PACKAGE_DESCRIPTION_SUMMARY
        "C++ wrapper for the siderust high-precision astronomy library")
    set(CPACK_PACKAGE_DESCRIPTION
        "siderust-cpp provides a header-only C++17 API over the siderust Rust\n"
        "astronomy library via the siderust-ffi C ABI.  It bundles the pre-built\n"
        "siderust_ffi shared library together with the C++ headers and CMake\n"
        "package config. Requires qtty-cpp and tempoch-cpp.")
    set(CPACK_RESOURCE_FILE_LICENSE    "${CMAKE_CURRENT_SOURCE_DIR}/LICENSE")
    set(CPACK_PACKAGE_HOMEPAGE_URL     "https://github.com/Siderust/siderust-cpp")

    set(CPACK_COMPONENTS_ALL "siderust-cpp")

    # -- DEB ---------------------------------------------------------------
    # OS package dependency versions must track the packaged C++ bindings.
    # Rust crate compatibility is owned by Cargo/submodules and must not be
    # copied into Debian/RPM metadata.
    set(CPACK_DEBIAN_PACKAGE_MAINTAINER "VPRamon <vallespuigramon@gmail.com>")
    set(CPACK_DEBIAN_PACKAGE_SECTION    "libs")
    set(CPACK_DEBIAN_PACKAGE_DEPENDS    "libc6 (>= 2.17), libstdc++6 (>= 9), qtty-cpp (>= 0.4.5), tempoch-cpp (>= 0.5.2)")
    set(CPACK_DEBIAN_FILE_NAME          DEB-DEFAULT)

    # -- RPM ---------------------------------------------------------------
    set(CPACK_RPM_PACKAGE_LICENSE    "AGPL-3.0")
    set(CPACK_RPM_PACKAGE_GROUP      "Development/Libraries")
    set(CPACK_RPM_PACKAGE_REQUIRES   "glibc >= 2.17, libstdc++ >= 9, qtty-cpp >= 0.4.5, tempoch-cpp >= 0.5.2")
    set(CPACK_RPM_FILE_NAME          RPM-DEFAULT)

    set(CPACK_GENERATOR "DEB;RPM")

    include(CPack)
endif()
