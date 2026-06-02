cmake_minimum_required(VERSION 3.21)
project(siderust_cpp_installed_consumer LANGUAGES CXX)

set(CMAKE_CXX_STANDARD 17)
set(CMAKE_CXX_STANDARD_REQUIRED ON)
set(CMAKE_CXX_EXTENSIONS OFF)

find_package(siderust_cpp REQUIRED)

add_executable(smoke smoke.cpp)
target_link_libraries(smoke PRIVATE siderust::siderust_cpp)

# Link/run against the staged install prefix without LD_LIBRARY_PATH when possible.
if(CMAKE_PREFIX_PATH)
  list(GET CMAKE_PREFIX_PATH 0 _siderust_install_prefix)
  if(_siderust_install_prefix)
    file(TO_CMAKE_PATH "${_siderust_install_prefix}/lib" _siderust_install_lib)
    if(EXISTS "${_siderust_install_lib}")
      if(APPLE)
        set_target_properties(smoke PROPERTIES
          BUILD_RPATH "${_siderust_install_lib}"
          INSTALL_RPATH "@loader_path/../lib"
        )
      elseif(UNIX)
        set_target_properties(smoke PROPERTIES
          BUILD_RPATH "${_siderust_install_lib}"
          INSTALL_RPATH "$ORIGIN/../lib"
        )
      endif()
    endif()
  endif()
endif()
