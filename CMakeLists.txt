cmake_minimum_required(VERSION 3.15)
project(siderust_cpp VERSION 0.1.0 LANGUAGES CXX)

set(CMAKE_CXX_STANDARD 17)
set(CMAKE_CXX_STANDARD_REQUIRED ON)
set(CMAKE_CXX_EXTENSIONS OFF)

# Find Cargo for building Rust libraries
find_program(CARGO_BIN cargo REQUIRED)

# ---------------------------------------------------------------------------
# Paths to siderust / siderust-ffi / tempoch-ffi (submodules or siblings)
# ---------------------------------------------------------------------------
set(SIDERUST_SUBMODULE_DIR ${CMAKE_CURRENT_SOURCE_DIR}/siderust)
set(SIDERUST_FFI_DIR       ${CMAKE_CURRENT_SOURCE_DIR}/siderust-ffi)
set(TEMPOCH_FFI_DIR        ${SIDERUST_SUBMODULE_DIR}/tempoch/tempoch-ffi)

set(SIDERUST_FFI_INCLUDE_DIR ${SIDERUST_FFI_DIR}/include)
set(TEMPOCH_FFI_INCLUDE_DIR  ${TEMPOCH_FFI_DIR}/include)

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
set(SIDERUST_ARTIFACT_DIR ${SIDERUST_FFI_DIR}/target/release)
set(TEMPOCH_ARTIFACT_DIR  ${TEMPOCH_FFI_DIR}/target/release)

if(APPLE)
    set(SIDERUST_LIBRARY_PATH ${SIDERUST_ARTIFACT_DIR}/libsiderust_ffi.dylib)
    set(TEMPOCH_LIBRARY_PATH  ${TEMPOCH_ARTIFACT_DIR}/libtempoch_ffi.dylib)
elseif(WIN32)
    set(SIDERUST_LIBRARY_PATH ${SIDERUST_ARTIFACT_DIR}/siderust_ffi.dll)
    set(SIDERUST_IMPORT_LIBRARY ${SIDERUST_ARTIFACT_DIR}/siderust_ffi.dll.lib)
    set(TEMPOCH_LIBRARY_PATH  ${TEMPOCH_ARTIFACT_DIR}/tempoch_ffi.dll)
    set(TEMPOCH_IMPORT_LIBRARY ${TEMPOCH_ARTIFACT_DIR}/tempoch_ffi.dll.lib)
else()
    set(SIDERUST_LIBRARY_PATH ${SIDERUST_ARTIFACT_DIR}/libsiderust_ffi.so)
    set(TEMPOCH_LIBRARY_PATH  ${TEMPOCH_ARTIFACT_DIR}/libtempoch_ffi.so)
endif()

# ---------------------------------------------------------------------------
# Build siderust-ffi and tempoch-ffi via Cargo
# ---------------------------------------------------------------------------

# Build tempoch-ffi first (separate target dir)
add_custom_target(
    build_tempoch_ffi
    COMMAND ${CARGO_BIN} build --release
    WORKING_DIRECTORY ${TEMPOCH_FFI_DIR}
    BYPRODUCTS ${TEMPOCH_LIBRARY_PATH}
    COMMENT "Building tempoch-ffi via Cargo"
    VERBATIM
)

# Build siderust-ffi (depends on tempoch-ffi rlib, but has its own cdylib)
add_custom_target(
    build_siderust_ffi
    COMMAND ${CARGO_BIN} build --release ${_SIDERUST_FEATURES_ARGS}
    WORKING_DIRECTORY ${SIDERUST_FFI_DIR}
    BYPRODUCTS ${SIDERUST_LIBRARY_PATH}
    DEPENDS build_tempoch_ffi
    COMMENT "Building siderust-ffi via Cargo"
    VERBATIM
)

# Import siderust_ffi
add_library(siderust_ffi SHARED IMPORTED GLOBAL)
set_target_properties(siderust_ffi PROPERTIES IMPORTED_LOCATION ${SIDERUST_LIBRARY_PATH})
if(WIN32)
    set_target_properties(siderust_ffi PROPERTIES IMPORTED_IMPLIB ${SIDERUST_IMPORT_LIBRARY})
endif()
add_dependencies(siderust_ffi build_siderust_ffi)

# Import tempoch_ffi
add_library(tempoch_ffi SHARED IMPORTED GLOBAL)
set_target_properties(tempoch_ffi PROPERTIES IMPORTED_LOCATION ${TEMPOCH_LIBRARY_PATH})
if(WIN32)
    set_target_properties(tempoch_ffi PROPERTIES IMPORTED_IMPLIB ${TEMPOCH_IMPORT_LIBRARY})
endif()
add_dependencies(tempoch_ffi build_tempoch_ffi)

# ---------------------------------------------------------------------------
# Header-only C++ wrapper library
# ---------------------------------------------------------------------------
# Pull in qtty-cpp (unit-safe quantities) as a subdirectory
set(QTTY_FFI_FEATURES "" CACHE STRING "Cargo features for qtty-ffi" FORCE)
add_subdirectory(qtty-cpp)

# Paths to qtty-ffi shared library (for RPATH)
set(QTTY_SUBMODULE_DIR ${CMAKE_CURRENT_SOURCE_DIR}/qtty-cpp/qtty)
set(QTTY_ARTIFACT_DIR  ${QTTY_SUBMODULE_DIR}/target/release)

add_library(siderust_cpp INTERFACE)
target_include_directories(siderust_cpp INTERFACE
    $<BUILD_INTERFACE:${CMAKE_CURRENT_SOURCE_DIR}/include>
    $<BUILD_INTERFACE:${SIDERUST_FFI_INCLUDE_DIR}>
    $<BUILD_INTERFACE:${TEMPOCH_FFI_INCLUDE_DIR}>
    $<INSTALL_INTERFACE:include>
)
target_link_libraries(siderust_cpp INTERFACE siderust_ffi tempoch_ffi qtty_cpp)
add_dependencies(siderust_cpp build_siderust_ffi)

# RPATH for shared library lookup at runtime
if(APPLE)
    set(_siderust_rpath "@loader_path/../siderust-ffi/target/release;@loader_path/../siderust/tempoch/tempoch-ffi/target/release;@loader_path/../qtty-cpp/qtty/target/release")
elseif(UNIX)
    set(_siderust_rpath "$ORIGIN/../siderust-ffi/target/release:$ORIGIN/../siderust/tempoch/tempoch-ffi/target/release:$ORIGIN/../qtty-cpp/qtty/target/release")
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
add_executable(demo examples/demo.cpp)
target_link_libraries(demo PRIVATE siderust_cpp)
if(DEFINED _siderust_rpath)
    set_target_properties(demo PROPERTIES
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
gtest_discover_tests(test_siderust)

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

install(DIRECTORY ${TEMPOCH_FFI_INCLUDE_DIR}/
    DESTINATION include
    FILES_MATCHING PATTERN "*.h"
)

install(FILES ${SIDERUST_LIBRARY_PATH} ${TEMPOCH_LIBRARY_PATH}
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
