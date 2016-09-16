
#-----------------------------------------------------------------------------
# Configure files, include dirs, etc.
#-----------------------------------------------------------------------------

# Make sure that the correct paths separators are used on each platform
if(WIN32)
  set(DIR_SEP "\\\\")
  string(REPLACE "/" "\\\\" CMAKE_LIBRARY_OUTPUT_DIRECTORY_NATIVE ${CMAKE_LIBRARY_OUTPUT_DIRECTORY})
  string(REPLACE "/" "\\\\" CMAKE_RUNTIME_OUTPUT_DIRECTORY_NATIVE ${CMAKE_RUNTIME_OUTPUT_DIRECTORY})
else()
  set(DIR_SEP "/")
  set(CMAKE_LIBRARY_OUTPUT_DIRECTORY_NATIVE ${CMAKE_LIBRARY_OUTPUT_DIRECTORY})
  set(CMAKE_RUNTIME_OUTPUT_DIRECTORY_NATIVE ${CMAKE_RUNTIME_OUTPUT_DIRECTORY})
endif()

configure_file("${CMAKE_CURRENT_SOURCE_DIR}/TestingConfig.h.in" "${PROJECT_BINARY_DIR}/include/TestingConfig.h")

include_directories(${CMAKE_CURRENT_SOURCE_DIR})

#-----------------------------------------------------------------------------
# Create test bundles
#-----------------------------------------------------------------------------

include(usFunctionCreateTestBundle)

set(_us_test_bundle_libs "" CACHE INTERNAL "" FORCE)
add_subdirectory(bundles)

#-----------------------------------------------------------------------------
# Create non-bundle test libraries
#-----------------------------------------------------------------------------

add_subdirectory(singletonframework)

#-----------------------------------------------------------------------------
# Add unit tests
#-----------------------------------------------------------------------------

set(_tests
  AnyTest
  BundleRegistryPerformanceTest
  FrameworkEventTest
  FrameworkListenerTest
  FrameworkTest
  FrameworkFactoryTest
  HelgrindTest
  LDAPFilterTest
  LDAPQueryTest
  LogTest
  BundleEventTest
  BundleHooksTest
  BundleManifestTest
  BundleTest
  BundleResourceTest
  MemcheckTest
  ServiceFactoryTest
  ServiceHooksTest
  ServiceRegistryPerformanceTest
  ServiceRegistryTest
  ServiceTemplateTest
  ServiceTrackerTest
  StaticBundleResourceTest
  StaticBundleTest
)

if(US_BUILD_SHARED_LIBS)
  list(APPEND _tests
       ServiceListenerTest
       SharedLibraryTest
      )
endif()

if(US_ENABLE_THREADING_SUPPORT)
  list(APPEND _tests ConcurrencyTest)
endif()

set(_additional_srcs
  TestDriverActivator.cpp
  TestManager.cpp
  TestUtilBundleListener.cpp
  TestUtils.cpp
)

#-----------------------------------------------------------------------------
# Build the main test driver executable
#-----------------------------------------------------------------------------

set(_test_driver us${PROJECT_NAME}TestDriver)
set(_test_sourcelist_extra_args )
create_test_sourcelist(_srcs ${_test_driver}.cpp ${_tests} ${_test_sourcelist_extra_args})

# Generate a custom "bundle init" file for the test driver executable
usFunctionGenerateBundleInit(_srcs)

usFunctionGetResourceSource(TARGET ${_test_driver} OUT _srcs)
add_executable(${_test_driver} ${_srcs} ${_additional_srcs})
set_property(TARGET ${_test_driver} APPEND PROPERTY COMPILE_DEFINITIONS US_BUNDLE_NAME=main)
set_property(TARGET ${_test_driver} PROPERTY US_BUNDLE_NAME main)
if(NOT US_BUILD_SHARED_LIBS)
  target_link_libraries(${_test_driver} ${_us_test_bundle_libs})
endif()

target_link_libraries(${_test_driver} ${Framework_TARGET})

target_link_libraries(${_test_driver} singletonframework)

# Needed for clock_gettime with glibc < 2.17
if(UNIX AND NOT APPLE)
  target_link_libraries(${_test_driver} rt)
endif()

# Add resources
usFunctionEmbedResources(TARGET ${_test_driver}
                       FILES TestResource.txt manifest.json
                       ZIP_ARCHIVES ${Framework_TARGET} ${_us_test_bundle_libs})

# Register tests
us_add_tests(${_test_driver} ${_tests})

if(US_MEMCHECK_COMMAND)
  set_tests_properties(memcheck_MemcheckTest PROPERTIES WILL_FAIL 1)
  if(US_ENABLE_HELGRIND)
    set_tests_properties(helgrind_HelgrindTest PROPERTIES WILL_FAIL 1)
  endif()
endif()

if(US_ENABLE_TSAN)
  set_tests_properties(HelgrindTest PROPERTIES WILL_FAIL 1)
endif()

#-----------------------------------------------------------------------------
# Add dependencies for shared libraries
#-----------------------------------------------------------------------------

if(US_BUILD_SHARED_LIBS)
  foreach(_test_bundle ${_us_test_bundle_libs})
    add_dependencies(${_test_driver} ${_test_bundle})
  endforeach()
endif()
