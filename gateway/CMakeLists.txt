cmake_minimum_required(VERSION 3.16)
project(gateway CXX)

set(CMAKE_CXX_STANDARD 17)
set(CMAKE_CXX_STANDARD_REQUIRED ON)
if(NOT CMAKE_BUILD_TYPE)
  set(CMAKE_BUILD_TYPE Release)
endif()

include(FetchContent)

# Header-only HTTP server + client used for the reverse proxy.
FetchContent_Declare(httplib
  GIT_REPOSITORY https://github.com/yhirose/cpp-httplib.git
  GIT_TAG v0.15.3)
FetchContent_MakeAvailable(httplib)

# YAML config parsing (also used to parse decoded JWT JSON segments).
set(YAML_CPP_BUILD_TESTS OFF CACHE BOOL "" FORCE)
set(YAML_CPP_BUILD_TOOLS OFF CACHE BOOL "" FORCE)
FetchContent_Declare(yaml-cpp
  GIT_REPOSITORY https://github.com/jbeder/yaml-cpp.git
  GIT_TAG 0.8.0)
FetchContent_MakeAvailable(yaml-cpp)

find_package(Threads REQUIRED)

add_executable(gateway
  src/main.cpp
  src/jwt.cpp
  src/config.cpp)

target_include_directories(gateway PRIVATE include)
target_link_libraries(gateway PRIVATE httplib::httplib yaml-cpp::yaml-cpp Threads::Threads)

option(APIWAY_BUILD_TESTS "Build the gateway unit tests" OFF)
if(APIWAY_BUILD_TESTS)
  enable_testing()
  add_executable(apiway_tests
    tests/test_main.cpp
    src/jwt.cpp)
  target_include_directories(apiway_tests PRIVATE include)
  target_link_libraries(apiway_tests PRIVATE yaml-cpp::yaml-cpp Threads::Threads)
  add_test(NAME apiway_tests COMMAND apiway_tests)
endif()
