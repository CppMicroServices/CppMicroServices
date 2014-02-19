
#-----------------------------------------------------------------------------
# Configure files, include dirs, etc.
#-----------------------------------------------------------------------------

configure_file("${CMAKE_CURRENT_SOURCE_DIR}/usTestingConfig.h.in" "${PROJECT_BINARY_DIR}/include/usTestingConfig.h")

include_directories(${CMAKE_CURRENT_SOURCE_DIR})

#-----------------------------------------------------------------------------
# Create test modules
#-----------------------------------------------------------------------------

include(usFunctionCreateTestModule)

set(_us_test_module_libs "" CACHE INTERNAL "" FORCE)
add_subdirectory(modules)

#-----------------------------------------------------------------------------
# Add unit tests
#-----------------------------------------------------------------------------

set(_tests
  usDebugOutputTest
  usLDAPFilterTest
  usModuleTest
  usModuleResourceTest
  usServiceFactoryTest
  usServiceHooksTest
  usServiceRegistryPerformanceTest
  usServiceRegistryTest
  usServiceTemplateTest
  usServiceTrackerTest
  usStaticModuleResourceTest
  usStaticModuleTest
)

if(US_BUILD_SHARED_LIBS)
  list(APPEND _tests
       usModuleHooksTest
       usModuleManifestTest
       usServiceListenerTest
       usSharedLibraryTest
      )
  if(US_ENABLE_AUTOLOADING_SUPPORT)
    list(APPEND _tests usModuleAutoLoadTest)
  endif()
endif()

set(_additional_srcs
  usTestManager.cpp
  usTestUtilModuleListener.cpp
)

set(_test_driver us${PROJECT_NAME}TestDriver)
set(_test_sourcelist_extra_args )
create_test_sourcelist(_srcs ${_test_driver}.cpp ${_tests} ${_test_sourcelist_extra_args})

usFunctionEmbedResources(_srcs EXECUTABLE_NAME ${_test_driver} FILES usTestResource.txt manifest.json)

# Generate a custom "module init" file for the test driver executable
if(US_BUILD_SHARED_LIBS)
  usFunctionGenerateExecutableInit(_srcs IDENTIFIER ${_test_driver})
endif()

add_executable(${_test_driver} ${_srcs} ${_additional_srcs})
if(NOT US_BUILD_SHARED_LIBS)
  target_link_libraries(${_test_driver} ${_us_test_module_libs})
endif()
target_link_libraries(${_test_driver} ${Core_TARGET})

if(UNIX AND NOT APPLE)
  target_link_libraries(${_test_driver} rt)
endif()

# Register tests
foreach(_test ${_tests})
  add_test(NAME ${_test} COMMAND ${_test_driver} ${_test})
endforeach()

if(US_TEST_LABELS)
  set_tests_properties(${_tests} PROPERTIES LABELS "${US_TEST_LABELS}")
endif()

#-----------------------------------------------------------------------------
# Add dependencies for shared libraries
#-----------------------------------------------------------------------------

if(US_BUILD_SHARED_LIBS)
  foreach(_test_module ${_us_test_module_libs})
    add_dependencies(${_test_driver} ${_test_module})
  endforeach()
endif()
