#-----------------------------------------------------------------------------
# Build and run the GTest Suite of tests
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

set(us_gtest_test_exe_name usFrameworkTests)

if(US_COMPILER_CLANG OR US_COMPILER_APPLE_CLANG)
  add_compile_options(-Wno-error=unneeded-internal-declaration)
endif()

include_directories(
  ${GTEST_INCLUDE_DIRS}
  ${GMOCK_INCLUDE_DIRS}
  ${CMAKE_CURRENT_SOURCE_DIR}/../util
  ${CMAKE_CURRENT_SOURCE_DIR}/../../../third_party
  )

#-----------------------------------------------------------------------------
# Add test source files
#-----------------------------------------------------------------------------
set(_gtest_tests
  AnyTest.cpp
  AnyMapTest.cpp
  BundleActivatorTest.cpp
  BundleContextTest.cpp
  BundleDeadLockTest.cpp
  BundleManifestTest.cpp
  BundleValidationTest.cpp
  BundleVersionTest.cpp
  CFRLoggerTest.cpp
  InvalidBundleTest.cpp
  GlobalServiceTrackerTest.cpp
  LDAPExprTest.cpp
  LDAPFilterTest.cpp
  OpenFileHandleTest.cpp
  UtilsTest.cpp
  FrameworkTest.cpp
  BundleObjFileTest.cpp
  BundleGetSymbolTest.cpp
  SecurityExceptionTest.cpp
  ServiceExceptionTest.cpp
  ServiceObjectsTest.cpp
  ServiceReferenceTest.cpp
  ServiceFactoryTest.cpp
  ServiceTrackerTest.cpp
  SharedLibraryExceptionTest.cpp
  ShrinkableVectorTest.cpp
  BundleEventTest.cpp
  BundleResourceTest.cpp
  BundleStreamOperatorTest.cpp
  ServiceEventStreamOperatorTest.cpp
  ServiceRegistryTest.cpp
  LDAPQueryTest.cpp
  FrameworkFactoryTest.cpp
  LogTest.cpp
  StaticBundleResourceTest.cpp
  FrameworkEventTest.cpp
  ServiceTemplateTest.cpp
  BundleRegistryConcurrencyTest.cpp
  StaticBundleTest.cpp
  ShrinkableMapTest.cpp
  FrameworkListenerTest.cpp
  BundleHooksTest.cpp
  ServiceHooksTest.cpp
  TestCounterLatch.cpp
  ResourceCompilerTest.cpp
  MultipleListenersTest.cpp
  BundleTest.cpp
  BundleTrackerMethodTest.cpp
  BundleTrackerCustomCallbackTest.cpp
  BundleTrackerConcurrencyTest.cpp
  main.cpp
)

if(BUILD_SHARED_LIBS)
  list(APPEND _gtest_tests
       SharedLibraryTest.cpp
       ServiceListenerTest.cpp
      )
endif()

set(_additional_srcs
  FrameworkTestActivator.cpp
  ../util/TestUtilBundleListener.cpp
  ../util/TestUtilFrameworkListener.cpp
  ../util/TestUtils.cpp
  ../util/ImportTestBundles.cpp
  ../../src/util/CFRLogger.cpp
  $<TARGET_OBJECTS:util>
  )

#-----------------------------------------------------------------------------
# Build the main test driver executable
#-----------------------------------------------------------------------------

set(_third_party_srcs 
  ../../../third_party/miniz.c
  ../../../third_party/jsoncpp.cpp
)

if(MINGW)
  # Suppress warnings for specific files
  set_source_files_properties(
    ../../../third_party/jsoncpp.cpp
    PROPERTIES COMPILE_FLAGS -Wno-maybe-uninitialized
  )
endif()

if(NOT APPLE AND US_COMPILER_GNU_ARM AND  CMAKE_SYSTEM_PROCESSOR MATCHES "aarch64|ARM64|arm64")
  add_compile_options(-Wno-error=maybe-uninitialized)
endif()

# Generate a custom "bundle init" file for the test driver executable
usFunctionGenerateBundleInit(TARGET ${us_gtest_test_exe_name} OUT _additional_srcs)

usFunctionGetResourceSource(TARGET ${us_gtest_test_exe_name} OUT _additional_srcs)

add_executable(${us_gtest_test_exe_name} ${_gtest_tests} ${_additional_srcs} ${_third_party_srcs})

if (US_COMPILER_MSVC AND BUILD_SHARED_LIBS)
  target_compile_options(${us_gtest_test_exe_name} PRIVATE -DGTEST_LINKED_AS_SHARED_LIBRARY)
endif()

# Needed for ResourceCompilerTest
add_dependencies(${us_gtest_test_exe_name} ${US_RCC_EXECUTABLE_TARGET})
set_property(TARGET ${us_gtest_test_exe_name} APPEND PROPERTY COMPILE_DEFINITIONS US_BUNDLE_NAME=main)
set_property(TARGET ${us_gtest_test_exe_name} PROPERTY US_BUNDLE_NAME main)

target_include_directories(${us_gtest_test_exe_name} PRIVATE $<TARGET_PROPERTY:util,INCLUDE_DIRECTORIES>)

# Disable deprecation warnings.
if (("${CMAKE_CXX_COMPILER_ID}" STREQUAL "Clang") OR
    ("${CMAKE_CXX_COMPILER_ID}" STREQUAL "AppleClang") OR
    ("${CMAKE_CXX_COMPILER_ID}" STREQUAL "GNU"))
  set_source_files_properties(${_gtest_tests}
                                PROPERTIES COMPILE_FLAGS -Wno-error=deprecated-declarations
                                            COMPILE_FLAGS -Wno-deprecated
                                            COMPILE_FLAGS -Wno-deprecated-declarations)
endif()

target_link_libraries(${us_gtest_test_exe_name} ${GTEST_BOTH_LIBRARIES})
target_link_libraries(${us_gtest_test_exe_name} ${GMOCK_BOTH_LIBRARIES})
target_link_libraries(${us_gtest_test_exe_name} ${Framework_TARGET} usLogService)

# Needed for clock_gettime with glibc < 2.17
if(UNIX AND NOT APPLE)
  target_link_libraries(${us_gtest_test_exe_name} rt)
endif()

# tests can be configured to run using add_test or GTEST_ADD_TEST (https://cmake.org/cmake/help/v3.0/module/FindGTest.html)
# GTEST_ADD_TEST will run a seprate EXE for each test and test fixture in the EXE.
# At this time, there isn't a need to use GTEST_ADD_TEST.

# Run the GTest EXE from ctest.
add_test(NAME ${us_gtest_test_exe_name}
  COMMAND ${us_gtest_test_exe_name}
  WORKING_DIRECTORY ${CppMicroServices_BINARY_DIR}
)
set_property(TEST ${us_gtest_test_exe_name} PROPERTY LABELS regular)

# Run the GTest EXE from valgrind
if(US_MEMCHECK_COMMAND)
  add_test(NAME memcheck_${us_gtest_test_exe_name} COMMAND ${US_MEMCHECK_COMMAND} --max-threads=2000 --error-exitcode=1 ${US_RUNTIME_OUTPUT_DIRECTORY}/${us_gtest_test_exe_name}
    WORKING_DIRECTORY ${CppMicroServices_BINARY_DIR})
  set_property(TEST memcheck_${us_gtest_test_exe_name} PROPERTY LABELS valgrind memcheck)
endif()

# Enable code coverage for GTest tests
if(US_COVERAGE_COMMAND)
  file(TO_NATIVE_PATH ${PROJECT_SOURCE_DIR} PROJECT_SOURCE_NATIVE_PATH)
  add_test(NAME coverage_${us_gtest_test_exe_name} COMMAND ${US_COVERAGE_COMMAND} --verbose --cover_children --sources=${PROJECT_SOURCE_NATIVE_PATH} --export_type=binary:${us_gtest_test_exe_name}.cov --continue_after_cpp_exception -- $<TARGET_FILE:${us_gtest_test_exe_name}>
      WORKING_DIRECTORY ${CppMicroServices_BINARY_DIR})
  set_property(TEST coverage_${us_gtest_test_exe_name} PROPERTY LABELS opencppcoverage)
endif()

# Add a Preprocessor flag if code coverage is turned ON
if(US_ENABLE_COVERAGE)
  target_compile_definitions(${us_gtest_test_exe_name} PUBLIC US_COVERAGE_ENABLED=1)
endif()

# Copy the Google Test libraries into the same folder as the
# executable so that they can be seen at runtime on Windows.
# Mac and Linux use RPATHs and do not need to do this.
if (WIN32 AND US_USE_SYSTEM_GTEST)
  foreach(lib_fullpath ${GTEST_BOTH_LIBRARIES})
    get_filename_component(dir ${lib_fullpath} DIRECTORY)
    get_filename_component(name_no_ext ${lib_fullpath} NAME_WE)
    set(dll_file "${dir}/${name_no_ext}${CMAKE_SHARED_LIBRARY_SUFFIX}")
    add_custom_command(TARGET ${us_gtest_test_exe_name} POST_BUILD
    COMMAND ${CMAKE_COMMAND} -E copy_if_different
        "${dll_file}"
        $<TARGET_FILE_DIR:${us_gtest_test_exe_name}>)
  endforeach(lib_fullpath)
endif()

#-----------------------------------------------------------------------------
# Add dependencies on test bundles if building shared libraries or 
# link them if building static libraries
#-----------------------------------------------------------------------------

if(BUILD_SHARED_LIBS)
    add_dependencies(${us_gtest_test_exe_name} ${_us_test_bundle_libs})
    # Add resources
    usFunctionEmbedResources(TARGET ${us_gtest_test_exe_name}
                             FILES TestResource.ptxt manifest.json)
else()
    target_link_libraries(${us_gtest_test_exe_name} ${_us_test_bundle_libs})
    # Add resources
    usFunctionEmbedResources(TARGET ${us_gtest_test_exe_name}
                             FILES TestResource.ptxt manifest.json
                             ZIP_ARCHIVES ${Framework_TARGET} ${_us_test_bundle_libs})
endif()
