cmake_minimum_required(VERSION 3.4)

set(CMAKE_CXX_STANDARD 17)
set(CMAKE_CXX_STANDARD_REQUIRED ON)

option(BUILD_COVERAGE "Build coverage" OFF)

set(
        HUNTER_CACHE_SERVERS
        "https://github.com/bmstu-iu8-cpp-sem-3/hunter-cache"
        CACHE STRING "Default cache server"
)

include("tools/gate/cmake/HunterGate.cmake")
include_directories(process/include/boost)
include_directories(process/include)

HunterGate(
        URL "https://github.com/cpp-pm/hunter/archive/v0.23.218.tar.gz"
        SHA1 "9a3594a61227967fb92159339ba326701b287b19"
)

project(lab_11)

hunter_add_package(Boost COMPONENTS program_options filesystem system regex)
hunter_add_package(Async++)
find_package(Async++ CONFIG REQUIRED)
find_package(Boost CONFIG REQUIRED program_options filesystem system regex)

add_library(header INTERFACE)
add_library(Log INTERFACE)

add_executable(lab_11
        ${CMAKE_CURRENT_SOURCE_DIR}/sources/source.cpp
        )

target_include_directories(header
        INTERFACE $<BUILD_INTERFACE:${${PROJECT_NAME}_SOURCE_DIR}/include>
        $<INSTALL_INTERFACE:${CMAKE_INSTALL_INCLUDEDIR}>)
target_include_directories(Log
        INTERFACE $<BUILD_INTERFACE:${${PROJECT_NAME}_SOURCE_DIR}/include>
        $<INSTALL_INTERFACE:${CMAKE_INSTALL_INCLUDEDIR}>)

target_link_libraries(lab_11 Boost::program_options Async++::Async++ Boost::system Boost::filesystem Boost::regex)
target_link_libraries(lab_11 header)

enable_testing()
add_test(NAME unit_tests COMMAND lab_11)

if(BUILD_COVERAGE)
  set(ENABLE_COVERAGE ON CACHE BOOL "Enable coverage build." FORCE)
  list(APPEND CMAKE_MODULE_PATH "${CMAKE_CURRENT_SOURCE_DIR}/cmake")
  find_package(codecov)
  
  add_coverage(lab_11)
  list(APPEND LCOV_REMOVE_PATTERNS "'${PROJECT_SOURCE_DIR}/sources/*'")
  coverage_evaluate()
endif()
