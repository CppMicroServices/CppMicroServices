
#-----------------------------------------------------------------------------
# Configure files, include dirs, etc.
#-----------------------------------------------------------------------------

configure_file("${CMAKE_CURRENT_SOURCE_DIR}/usTestingConfig.h.in" "${PROJECT_BINARY_DIR}/include/usTestingConfig.h")

include_directories(${US_INCLUDE_DIRS} ${CMAKE_CURRENT_SOURCE_DIR})
if(NOT US_ENABLE_SERVICE_FACTORY_SUPPORT)
  include_directories(${US_BASECLASS_INCLUDE_DIRS})
endif()

link_directories(${US_LINK_DIRS})
if(NOT US_ENABLE_SERVICE_FACTORY_SUPPORT)
  link_directories(${US_BASECLASS_LIBRARY_DIRS})
endif()

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
  usServiceRegistryTest
  usServiceTrackerTest
  usStaticModuleTest
)

if(US_BUILD_SHARED_LIBS)
  list(APPEND _tests usServiceListenerTest)
  if(US_ENABLE_AUTOLOADING_SUPPORT)
    list(APPEND _tests usModuleAutoLoadTest)
  endif()
endif()

set(_additional_srcs
  usTestManager.cpp
  usTestUtilModuleListener.cpp
  usTestUtilSharedLibrary.cpp
)

set(_test_driver ${PROJECT_NAME}TestDriver)
set(_test_sourcelist_extra_args )
create_test_sourcelist(_srcs ${_test_driver}.cpp ${_tests} ${_test_sourcelist_extra_args})

# Generate a custom "module init" file for the test driver executable
set(MODULE_DEPENDS_STR )
foreach(_dep ${US_LINK_LIBRARIES})
  set(MODULE_DEPENDS_STR "${MODULE_DEPENDS_STR} ${_dep}")
endforeach()

if(US_BUILD_SHARED_LIBS)
  usFunctionGenerateModuleInit(_srcs
                               NAME ${_test_driver}
                               DEPENDS "${MODULE_DEPENDS_STR}"
                               VERSION "0.1.0"
                               EXECUTABLE
                              )
endif()

add_executable(${_test_driver} ${_srcs} ${_additional_srcs})
if(NOT US_BUILD_SHARED_LIBS)
  target_link_libraries(${_test_driver} ${_us_test_module_libs})
endif()
target_link_libraries(${_test_driver} ${US_LINK_LIBRARIES})

if(NOT US_ENABLE_SERVICE_FACTORY_SUPPORT)
  target_link_libraries(${_test_driver} ${US_BASECLASS_LIBRARIES})
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
