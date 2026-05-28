cmake_minimum_required(VERSION 3.21)
project(siderust_cpp_installed_consumer LANGUAGES CXX)

set(CMAKE_CXX_STANDARD 17)
set(CMAKE_CXX_STANDARD_REQUIRED ON)
set(CMAKE_CXX_EXTENSIONS OFF)

find_package(siderust_cpp REQUIRED)

add_executable(smoke smoke.cpp)
target_link_libraries(smoke PRIVATE siderust::siderust_cpp)
