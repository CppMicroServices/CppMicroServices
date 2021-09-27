
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
  set(CTEST_DASHBOARD_ROOT "/tmp/us_builds")
endif()

if(NOT ${CMAKE_CXX_COMPILER_ID} STREQUAL "GNU" AND NOT WIN32)
  # gcc in combination with gcov seems to consume a lot of memory
  # and may lead to errors when building in CI. Hence we compile
  # with -j for non-GNU compilers only.
set(CTEST_BUILD_FLAGS "-j")
endif()

if ($ENV{US_BUILD_DEBUG})
  set(CTEST_CONFIGURATION_TYPE Debug)
  set(CTEST_BUILD_CONFIGURATION Debug)
else()
  set(CTEST_CONFIGURATION_TYPE Release)
  set(CTEST_BUILD_CONFIGURATION Release)
endif()

set(US_SOURCE_DIR "${CMAKE_CURRENT_LIST_DIR}/../")

set(US_BUILD_CONFIGURATION $ENV{BUILD_CONFIGURATION})
set(WITH_COVERAGE $ENV{WITH_COVERAGE})
include(${US_SOURCE_DIR}/cmake/usCTestScript.cmake)
