cmake_minimum_required(VERSION 3.15)
project(siderust_cpp VERSION 0.1.0 LANGUAGES CXX)

set(CMAKE_CXX_STANDARD 17)
set(CMAKE_CXX_STANDARD_REQUIRED ON)
set(CMAKE_CXX_EXTENSIONS OFF)
option(SIDERUST_BUILD_DOCS "Enable Doxygen documentation target." ON)

# Find Cargo for building Rust libraries
find_program(CARGO_BIN cargo REQUIRED)

# ---------------------------------------------------------------------------
# Paths to siderust-ffi (inside the siderust submodule)
# ---------------------------------------------------------------------------
set(SIDERUST_FFI_DIR       ${CMAKE_CURRENT_SOURCE_DIR}/siderust/siderust-ffi)

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

# ---------------------------------------------------------------------------
# Platform-specific library paths
# ---------------------------------------------------------------------------
set(SIDERUST_ARTIFACT_DIR ${CMAKE_CURRENT_SOURCE_DIR}/siderust/target/release)

if(APPLE)
    set(SIDERUST_LIBRARY_PATH ${SIDERUST_ARTIFACT_DIR}/libsiderust_ffi.dylib)
elseif(WIN32)
    set(SIDERUST_LIBRARY_PATH ${SIDERUST_ARTIFACT_DIR}/siderust_ffi.dll)
    set(SIDERUST_IMPORT_LIBRARY ${SIDERUST_ARTIFACT_DIR}/siderust_ffi.dll.lib)
else()
    set(SIDERUST_LIBRARY_PATH ${SIDERUST_ARTIFACT_DIR}/libsiderust_ffi.so)
endif()

# ---------------------------------------------------------------------------
# Build siderust-ffi via Cargo
# ---------------------------------------------------------------------------

# Build siderust-ffi (depends on tempoch-ffi via tempoch-cpp subdirectory)
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

# Import siderust_ffi
add_library(siderust_ffi SHARED IMPORTED GLOBAL)
set_target_properties(siderust_ffi PROPERTIES IMPORTED_LOCATION ${SIDERUST_LIBRARY_PATH})
if(WIN32)
    set_target_properties(siderust_ffi PROPERTIES IMPORTED_IMPLIB ${SIDERUST_IMPORT_LIBRARY})
endif()
add_dependencies(siderust_ffi build_siderust_ffi)

# ---------------------------------------------------------------------------
# Header-only C++ wrapper library
# ---------------------------------------------------------------------------
# Pull in qtty-cpp (unit-safe quantities) as a subdirectory
set(QTTY_FFI_FEATURES "" CACHE STRING "Cargo features for qtty-ffi" FORCE)
add_subdirectory(qtty-cpp)

# Pull in tempoch-cpp (time types) as a subdirectory
set(TEMPOCH_FFI_FEATURES "" CACHE STRING "Cargo features for tempoch-ffi" FORCE)
add_subdirectory(tempoch-cpp)

# Paths to qtty-ffi shared library (for RPATH)
set(QTTY_SUBMODULE_DIR ${CMAKE_CURRENT_SOURCE_DIR}/qtty-cpp/qtty)
set(QTTY_ARTIFACT_DIR  ${QTTY_SUBMODULE_DIR}/target/release)

# Paths to tempoch-ffi shared library (for RPATH)
set(TEMPOCH_SUBMODULE_DIR_PARENT ${CMAKE_CURRENT_SOURCE_DIR}/tempoch-cpp/tempoch)
set(TEMPOCH_ARTIFACT_DIR  ${TEMPOCH_SUBMODULE_DIR_PARENT}/tempoch-ffi/target/release)

add_library(siderust_cpp INTERFACE)
target_include_directories(siderust_cpp INTERFACE
    $<BUILD_INTERFACE:${CMAKE_CURRENT_SOURCE_DIR}/include>
    $<BUILD_INTERFACE:${SIDERUST_FFI_INCLUDE_DIR}>
    $<INSTALL_INTERFACE:include>
)
target_link_libraries(siderust_cpp INTERFACE siderust_ffi tempoch_cpp qtty_cpp)
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

# RPATH for shared library lookup at runtime
if(APPLE)
    set(_siderust_rpath "@loader_path/../siderust/siderust-ffi/target/release;@loader_path/../tempoch-cpp/tempoch/tempoch-ffi/target/release;@loader_path/../qtty-cpp/qtty/target/release")
elseif(UNIX)
    set(_siderust_rpath "$ORIGIN/../siderust/siderust-ffi/target/release:$ORIGIN/../tempoch-cpp/tempoch/tempoch-ffi/target/release:$ORIGIN/../qtty-cpp/qtty/target/release")
endif()

# ---------------------------------------------------------------------------
# Google Test
# ---------------------------------------------------------------------------
include(FetchContent)
FetchContent_Declare(
    googletest
    GIT_REPOSITORY https://github.com/google/googletest.git
    GIT_TAG v1.14.0
)
set(gtest_force_shared_crt ON CACHE BOOL "" FORCE)
FetchContent_MakeAvailable(googletest)

enable_testing()

# ---------------------------------------------------------------------------
# Example
# ---------------------------------------------------------------------------
add_executable(siderust_demo examples/demo.cpp)
target_link_libraries(siderust_demo PRIVATE siderust_cpp)
if(DEFINED _siderust_rpath)
    set_target_properties(siderust_demo PROPERTIES
        BUILD_RPATH ${_siderust_rpath}
        INSTALL_RPATH ${_siderust_rpath}
    )
endif()

add_executable(coordinates_examples examples/coordinates_examples.cpp)
target_link_libraries(coordinates_examples PRIVATE siderust_cpp)
if(DEFINED _siderust_rpath)
    set_target_properties(coordinates_examples PROPERTIES
        BUILD_RPATH ${_siderust_rpath}
        INSTALL_RPATH ${_siderust_rpath}
    )
endif()

add_executable(coordinate_systems_example examples/coordinate_systems_example.cpp)
target_link_libraries(coordinate_systems_example PRIVATE siderust_cpp)
if(DEFINED _siderust_rpath)
    set_target_properties(coordinate_systems_example PROPERTIES
        BUILD_RPATH ${_siderust_rpath}
        INSTALL_RPATH ${_siderust_rpath}
    )
endif()

add_executable(solar_system_bodies_example examples/solar_system_bodies_example.cpp)
target_link_libraries(solar_system_bodies_example PRIVATE siderust_cpp)
if(DEFINED _siderust_rpath)
    set_target_properties(solar_system_bodies_example PROPERTIES
        BUILD_RPATH ${_siderust_rpath}
        INSTALL_RPATH ${_siderust_rpath}
    )
endif()

add_executable(altitude_events_example examples/altitude_events_example.cpp)
target_link_libraries(altitude_events_example PRIVATE siderust_cpp)
if(DEFINED _siderust_rpath)
    set_target_properties(altitude_events_example PROPERTIES
        BUILD_RPATH ${_siderust_rpath}
        INSTALL_RPATH ${_siderust_rpath}
    )
endif()

add_executable(trackable_targets_example examples/trackable_targets_example.cpp)
target_link_libraries(trackable_targets_example PRIVATE siderust_cpp)
if(DEFINED _siderust_rpath)
    set_target_properties(trackable_targets_example PROPERTIES
        BUILD_RPATH ${_siderust_rpath}
        INSTALL_RPATH ${_siderust_rpath}
    )
endif()

add_executable(azimuth_lunar_phase_example examples/azimuth_lunar_phase_example.cpp)
target_link_libraries(azimuth_lunar_phase_example PRIVATE siderust_cpp)
if(DEFINED _siderust_rpath)
    set_target_properties(azimuth_lunar_phase_example PROPERTIES
        BUILD_RPATH ${_siderust_rpath}
        INSTALL_RPATH ${_siderust_rpath}
    )
endif()

# ---------------------------------------------------------------------------
# Tests
# ---------------------------------------------------------------------------
set(TEST_SOURCES
    tests/main.cpp
    tests/test_time.cpp
    tests/test_observatories.cpp
    tests/test_coordinates.cpp
    tests/test_bodies.cpp
    tests/test_altitude.cpp
    tests/test_ephemeris.cpp
)

add_executable(test_siderust ${TEST_SOURCES})
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

# ---------------------------------------------------------------------------
# Installation
# ---------------------------------------------------------------------------
install(DIRECTORY include/siderust
    DESTINATION include
    FILES_MATCHING PATTERN "*.hpp"
)

install(DIRECTORY ${SIDERUST_FFI_INCLUDE_DIR}/
    DESTINATION include
    FILES_MATCHING PATTERN "*.h"
)

install(FILES ${SIDERUST_LIBRARY_PATH}
    DESTINATION lib
)

include(CMakePackageConfigHelpers)

configure_package_config_file(
    ${CMAKE_CURRENT_SOURCE_DIR}/cmake/siderust_cppConfig.cmake.in
    ${CMAKE_CURRENT_BINARY_DIR}/siderust_cppConfig.cmake
    INSTALL_DESTINATION lib/cmake/siderust_cpp
)

write_basic_package_version_file(
    ${CMAKE_CURRENT_BINARY_DIR}/siderust_cppConfigVersion.cmake
    VERSION ${PROJECT_VERSION}
    COMPATIBILITY SameMajorVersion
)

install(FILES
    ${CMAKE_CURRENT_BINARY_DIR}/siderust_cppConfig.cmake
    ${CMAKE_CURRENT_BINARY_DIR}/siderust_cppConfigVersion.cmake
    DESTINATION lib/cmake/siderust_cpp
)

install(TARGETS siderust_cpp
    EXPORT siderust_cppTargets
    INCLUDES DESTINATION include
)

install(EXPORT siderust_cppTargets
    FILE siderust_cppTargets.cmake
    NAMESPACE siderust::
    DESTINATION lib/cmake/siderust_cpp
)
