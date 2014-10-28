
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
  usAnyTest
  usLDAPFilterTest
  usLogTest
  usModuleHooksTest
  usModuleManifestTest
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
       usServiceListenerTest
       usSharedLibraryTest
      )
  if(US_ENABLE_AUTOLOADING_SUPPORT)
    list(APPEND _tests usModuleAutoLoadTest)
  endif()
endif()

set(_additional_srcs
  usTestDriverActivator.cpp
  usTestManager.cpp
  usTestUtilModuleListener.cpp
)

set(_test_driver us${PROJECT_NAME}TestDriver)
set(_test_sourcelist_extra_args )
create_test_sourcelist(_srcs ${_test_driver}.cpp ${_tests} ${_test_sourcelist_extra_args})

# Generate a custom "module init" file for the test driver executable
usFunctionGenerateModuleInit(_srcs)

usFunctionGetResourceSource(TARGET ${_test_driver} OUT _srcs)
add_executable(${_test_driver} ${_srcs} ${_additional_srcs})
set_property(TARGET ${_test_driver} APPEND PROPERTY COMPILE_DEFINITIONS US_MODULE_NAME=main)
set_property(TARGET ${_test_driver} PROPERTY US_MODULE_NAME main)
if(NOT US_BUILD_SHARED_LIBS)
  set_property(TARGET ${_test_driver} APPEND PROPERTY COMPILE_DEFINITIONS US_STATIC_MODULE)
  target_link_libraries(${_test_driver} ${_us_test_module_libs})
endif()

target_link_libraries(${_test_driver} ${Core_TARGET})

if(UNIX AND NOT APPLE)
  target_link_libraries(${_test_driver} rt)
endif()

# Add resources
usFunctionEmbedResources(TARGET ${_test_driver}
                         FILES usTestResource.txt manifest.json
                         ZIP_ARCHIVES ${Core_TARGET} ${_us_test_module_libs})

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
