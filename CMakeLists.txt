project(Traveler C CXX)
set(CMAKE_CXX_STANDARD 11)

cmake_minimum_required(VERSION 2.8.9) # for Qt5

set(Traveler_MAJOR_VERSION 0)
set(Traveler_MINOR_VERSION 1)
set(Traveler_PATCH_VERSION 0)
set(Traveler_VERSION
    "${Traveler_MAJOR_VERSION}.${Traveler_MINOR_VERSION}.${Traveler_PATCH_VERSION}")

set(CMAKE_MODULE_PATH ${CMAKE_CURRENT_SOURCE_DIR}/CMake-Modules
    ${CMAKE_MODULE_PATH})
include(FindPackageHandleStandardArgs)

add_subdirectory(src)
