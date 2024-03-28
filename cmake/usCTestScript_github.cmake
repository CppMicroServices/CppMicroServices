
if(APPLE)
  find_program(CTEST_COVERAGE_COMMAND NAMES gcov)
elseif(NOT WIN32)
  find_program(CTEST_COVERAGE_COMMAND NAMES $ENV{MY_COVERAGE})
endif()
find_program(CTEST_MEMORYCHECK_COMMAND NAMES valgrind)
find_program(CTEST_GIT_COMMAND NAMES git)

if(DEFINED ENV{BUILD_DIR})
  set(CTEST_DASHBOARD_ROOT $ENV{BUILD_DIR})
else()
  message(FATAL_ERROR "The BUILD_DIR environment variable is not set. Please set BUILD_DIR to a valid, writeable path.")
endif()

if(NOT ${CMAKE_CXX_COMPILER_ID} STREQUAL "GNU" AND NOT WIN32)
  # gcc in combination with gcov seems to consume a lot of memory
  # and may lead to errors when building in CI. Hence we compile
  # with -j for non-GNU compilers only.
  set(CTEST_BUILD_FLAGS "-j")
elseif(WIN32 AND NOT "$ENV{BUILD_OS}" STREQUAL "mingw-w64")
  set(CTEST_BUILD_FLAGS "/maxcpucount")
endif()

set(CTEST_CONFIGURATION_TYPE $ENV{BUILD_TYPE})
set(CTEST_BUILD_CONFIGURATION $ENV{BUILD_TYPE})

set(WITH_DETERMINISTIC $ENV{BUILD_WITH_DETERMINISTIC})

set(US_SOURCE_DIR "${CMAKE_CURRENT_LIST_DIR}/../")

set(US_BUILD_CONFIGURATION $ENV{BUILD_CONFIGURATION})
set(WITH_COVERAGE $ENV{WITH_COVERAGE})
include(${US_SOURCE_DIR}/cmake/usCTestScript.cmake)
