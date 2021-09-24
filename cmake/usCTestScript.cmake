
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

  if (NOT WIN32)
    if (WITH_ASAN)
      set(US_ENABLE_ASAN ON)
    endif()

    if (WITH_TSAN)
      set(US_ENABLE_TSAN ON)
    endif()

    if (WITH_UBSAN)
      set(US_ENABLE_UBSAN ON)
    endif()
  endif()

  ctest_configure(RETURN_VALUE res)
  if (res)
    message(FATAL_ERROR "CMake configure error")
  endif()
  
  if(APPLE AND ${US_CMAKE_GENERATOR} STREQUAL "Xcode")
    # When using Xcode on macOS, ctest -VV option generates compile log
    # lines that are too verbose that some travis-ci builds exceed the 4MB
    # log file size limit and terminate prematurely.
    #
    # Note: we no longer use travis-ci
    #
    # Since "ctest_build" currently does not support piping outputs of the
    # native tool builds, a shell script file is used to manually call "xcodebuild"
    # (with the same options that ctest_build would have generated), piping the
    # output through "xcpretty" (https://github.com/xcpretty/xcpretty).
    #
    # Instead of calling xcodebuild directly in "execute_process", a separate
    # shell script file was created, so RESULT_VARIABLE can properly be captured
    # by looking at PIPESTATUS[0].
    execute_process( COMMAND bash ${CTEST_SOURCE_DIRECTORY}/cmake/xcodebuild_pretty.sh ${CTEST_BINARY_DIRECTORY} ${CTEST_BUILD_CONFIGURATION}
                     RESULT_VARIABLE res )
  else()
    ctest_build(RETURN_VALUE res)
  endif()
  
  if (res)
    message(FATAL_ERROR "CMake build error")
  endif()

  ctest_test(RETURN_VALUE res PARALLEL_LEVEL ${CTEST_PARALLEL_LEVEL})
  if (res)
   message(FATAL_ERROR "CMake test error")
  endif()


  if(WITH_MEMCHECK AND CTEST_MEMORYCHECK_COMMAND AND NOT WIN32)
    ctest_memcheck()
  endif()

  if(WITH_COVERAGE AND NOT WIN32)
    if(CTEST_COVERAGE_COMMAND)
      ctest_coverage(CAPTURE_CMAKE_ERROR err_result QUIET)
    else()
      message(FATAL_ERROR "CMake could not find coverage tool")
    endif()
  endif()

  #ctest_submit()

endmacro()

function(create_initial_cache var _shared _threading)

  set(_initial_cache "
      US_BUILD_TESTING:BOOL=ON
      US_ENABLE_COVERAGE:BOOL=$ENV{WITH_COVERAGE}
      BUILD_SHARED_LIBS:BOOL=${_shared}
      US_ENABLE_THREADING_SUPPORT:BOOL=${_threading}
      US_ENABLE_TSAN:BOOL=$ENV{WITH_TSAN}
      US_BUILD_EXAMPLES:BOOL=ON
      ")

  set(${var} ${_initial_cache} PARENT_SCOPE)

  if(_shared)
    set(CTEST_DASHBOARD_NAME "shared")
  else()
    set(CTEST_DASHBOARD_NAME "static")
  endif()

  if(_threading)
    set(CTEST_DASHBOARD_NAME "${CTEST_DASHBOARD_NAME}-threading")
  endif()

  if (NOT WIN32)
    string(REPLACE " " "-" _fixedGenerator ${_generator})
  else()
    string(REPLACE " " "-" _fixedGenerator ${CMAKE_GENERATOR})
  endif()
  set(CTEST_DASHBOARD_NAME "${CTEST_DASHBOARD_NAME}-${_fixedGenerator}" PARENT_SCOPE)

endfunction()

#=========================================================

set(CTEST_PROJECT_NAME CppMicroServices)

if(NOT CTEST_PARALLEL_LEVEL)
  set(CTEST_PARALLEL_LEVEL 1)
endif()


#            SHARED THREADING

set(config0     1       1 )
set(config1     0       1 )
set(config2     1       0 )
set(config3     0       0 )

if(NOT US_CMAKE_GENERATOR)
  if(APPLE AND NOT WITH_COVERAGE)
    set(US_CMAKE_GENERATOR "Xcode")
  elseif (NOT WIN32)
    set(US_CMAKE_GENERATOR "Unix Makefiles")
  else()
    if ("$ENV{GITHUB_BUILD_OS}" STREQUAL "windows-2016")
      set(US_CMAKE_GENERATOR "Visual Studio 15 2017")
    elseif ("$ENV{GITHUB_BUILD_OS}" STREQUAL "windows-2019")
      set(US_CMAKE_GENERATOR "Visual Studio 16 2019")
    endif()
  endif()
endif()

foreach (_generator ${US_CMAKE_GENERATOR})
  set(CTEST_CMAKE_GENERATOR ${_generator})
  foreach(i ${US_BUILD_CONFIGURATION})
    create_initial_cache(CTEST_INITIAL_CACHE ${config${i}})
    message("Testing build configuration: ${CTEST_DASHBOARD_NAME}")
    build_and_test()
  endforeach()
endforeach()
