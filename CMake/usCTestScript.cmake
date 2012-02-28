
macro(build_and_test)

  set(CTEST_SOURCE_DIRECTORY ${US_SOURCE_DIR})
  set(CTEST_BINARY_DIRECTORY "${CTEST_DASHBOARD_ROOT}/${CTEST_PROJECT_NAME}_${CTEST_DASHBOARD_NAME}")
  
  #if(NOT CTEST_BUILD_NAME)
  #  set(CTEST_BUILD_NAME "${CMAKE_SYSTEM}_${CTEST_COMPILER}_${CTEST_DASHBOARD_NAME}")
  #endif()
  
  ctest_empty_binary_directory(${CTEST_BINARY_DIRECTORY})
  
  ctest_start("Experimental")
  
  if(NOT EXISTS "${CTEST_BINARY_DIRECTORY}/CMakeCache.txt")
    file(WRITE "${CTEST_BINARY_DIRECTORY}/CMakeCache.txt" "${CTEST_INITIAL_CACHE}")
  endif()
  
  ctest_configure()
  ctest_build()
  ctest_test()
  
  if(WITH_MEMCHECK AND CTEST_MEMORYCHECK_COMMAND)
    ctest_memcheck()
  endif()
  
  if(WITH_COVERAGE AND CTEST_COVERAGE_COMMAND)
    ctest_coverage()
  endif()
  
  #ctest_submit()
  
endmacro()

macro(_to_bool var out)
  if(${var})
    set(${out} "ON")
  else()
    set(${out} "OFF")
  endif()
endmacro()

function(create_initial_cache var)

  macro_parse_arguments(_cache "" "SHARED;THREADING;SERVICE_FACTORY;C++11" ${ARGN})
  
  _to_bool(_cache_SHARED _bool_shared)
  _to_bool(_cache_THREADING _bool_threading)
  _to_bool(_cache_SERVICE_FACTORY _bool_sf)
  _to_bool(_cache_C++11 _bool_c++11)
  
  set(_initial_cache "
      US_BUILD_TESTING:BOOL=ON
      US_BUILD_SHARED_LIBS:BOOL=${_bool_shared}
      US_ENABLE_THREADING_SUPPORT:BOOL=${_bool_threading}
      US_ENABLE_SERVICE_FACTORY_SUPPORT:BOOL=${_bool_sf}
      US_USE_C++11:BOOL=${_bool_c++11}
      ")
      
  set(${var} ${_initial_cache} PARENT_SCOPE)
  
endfunction()

#=========================================================

set(CTEST_PROJECT_NAME CppMicroServices)

include(${US_SOURCE_DIR}/CMake/MacroParseArguments.cmake)

#==========================================================
# Shared config
#==========================================================

if(US_TEST_SHARED)

# shared, no threading, no service factory, no C++11
set(CTEST_DASHBOARD_NAME "shared")
create_initial_cache(CTEST_INITIAL_CACHE SHARED)
build_and_test()

# shared, no threading, with service factory, no C++11
set(CTEST_DASHBOARD_NAME "shared-servicefactory")
create_initial_cache(CTEST_INITIAL_CACHE SHARED SERVICE_FACTORY)
build_and_test()

# shared, with threading, no service factory, no C++11
set(CTEST_DASHBOARD_NAME "shared-threading")
create_initial_cache(CTEST_INITIAL_CACHE SHARED THREADING)
build_and_test()

# shared, with threading, with service factory, no C++11
set(CTEST_DASHBOARD_NAME "shared-threading-servicefactory")
create_initial_cache(CTEST_INITIAL_CACHE SHARED THREADING SERVICE_FACTORY)
build_and_test()

# shared, no threading, no service factory, with C++11
set(CTEST_DASHBOARD_NAME "shared-cxx11")
create_initial_cache(CTEST_INITIAL_CACHE SHARED C++11)
build_and_test()

# shared, no threading, with service factory, with C++11
set(CTEST_DASHBOARD_NAME "shared-servicefactory-cxx11")
create_initial_cache(CTEST_INITIAL_CACHE SHARED SERVICE_FACTORY C++11)
build_and_test()

# shared, with threading, no service factory, with C++11
set(CTEST_DASHBOARD_NAME "shared-threading-cxx11")
create_initial_cache(CTEST_INITIAL_CACHE SHARED THREADING C++11)
build_and_test()

# shared, with threading, with service factory, with C++11
set(CTEST_DASHBOARD_NAME "shared-threading-servicefactory-cxx11")
create_initial_cache(CTEST_INITIAL_CACHE SHARED THREADING SERVICE_FACTORY C++11)
build_and_test()

endif()

#==========================================================
# Static config
#==========================================================

if(US_TEST_STATIC)

# static, no threading, no service factory, no C++11
set(CTEST_DASHBOARD_NAME "static")
create_initial_cache(CTEST_INITIAL_CACHE)
build_and_test()

# static, no threading, with service factory, no C++11
set(CTEST_DASHBOARD_NAME "static-servicefactory")
create_initial_cache(CTEST_INITIAL_CACHE SERVICE_FACTORY)
build_and_test()

# static, with threading, no service factory, no C++11
set(CTEST_DASHBOARD_NAME "static-threading")
create_initial_cache(CTEST_INITIAL_CACHE THREADING)
build_and_test()

# static, with threading, with service factory, no C++11
set(CTEST_DASHBOARD_NAME "static-threading-servicefactory")
create_initial_cache(CTEST_INITIAL_CACHE THREADING SERVICE_FACTORY)
build_and_test()

# static, no threading, no service factory, with C++11
set(CTEST_DASHBOARD_NAME "static-cxx11")
create_initial_cache(CTEST_INITIAL_CACHE C++11)
build_and_test()

# static, no threading, with service factory, with C++11
set(CTEST_DASHBOARD_NAME "static-servicefactory-cxx11")
create_initial_cache(CTEST_INITIAL_CACHE SERVICE_FACTORY C++11)
build_and_test()

# static, with threading, no service factory, with C++11
set(CTEST_DASHBOARD_NAME "static-threading-cxx11")
create_initial_cache(CTEST_INITIAL_CACHE THREADING C++11)
build_and_test()

# static, with threading, with service factory, with C++11
set(CTEST_DASHBOARD_NAME "static-threading-servicefactory-cxx11")
create_initial_cache(CTEST_INITIAL_CACHE THREADING SERVICE_FACTORY C++11)
build_and_test()

endif()
