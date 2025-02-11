#-----------------------------------------------------------------------------
# Build and run the GTest Suite of tests
#-----------------------------------------------------------------------------

set(us_configurationadmin_test_exe_name usConfigurationAdminTests)

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

configure_file("${CMAKE_CURRENT_SOURCE_DIR}/TestingConfig.h.in" "${PROJECT_BINARY_DIR}/include/ConfigurationAdminTestingConfig.h")

include_directories(
  ${GTEST_INCLUDE_DIRS}
  ${GMOCK_INCLUDE_DIRS}
  )

usFunctionBoostPath(BOOST_SYSTEM ${US_USE_SYSTEM_BOOST} CPPMS_SOURCE_DIR ${CppMicroServices_SOURCE_DIR} BOOST_DIR ${BOOST_INCLUDEDIR})

# There are warnings in the boost asio headers which are flagged as errors. Include the boost
# asio headers as system headers to ignore these warnings and not treat them as errors.
include_directories(SYSTEM ${_boost_library})

if (US_COMPILER_CLANG OR US_COMPILER_APPLE_CLANG)
  check_cxx_compiler_flag(-Wno-inconsistent-missing-override HAS_MISSING_OVERRIDE_FLAG)
  if (HAS_MISSING_OVERRIDE_FLAG)
    add_compile_options(-Wno-inconsistent-missing-override)
  endif()
  check_cxx_compiler_flag(-Wno-deprecated-declarations HAS_MISSING_DEPRECATED_DECLARATIONS_FLAG)
  if (HAS_MISSING_OVERRIDE_FLAG)
    add_compile_options(-Wno-deprecated-declarations)
  endif()
endif()

if(MSVC)
  add_compile_definitions(GTEST_HAS_STD_TUPLE_=1)
  add_compile_definitions(GTEST_HAS_TR1_TUPLE=0)
  add_compile_definitions(GTEST_LANG_CXX11=1)
endif()

set(_configurationadmin_tests
  TestAsyncWorkService.cpp
  TestCMActivator.cpp
  TestCMBundleExtension.cpp
  TestCMLogger.cpp
  TestConfigAdmin.cpp
  TestConfigurationAdminImpl.cpp
  TestConfigurationImpl.cpp
  TestMetadataParserFactory.cpp
  TestMetadataParserImplV1.cpp
  main.cpp
  TestFixtures.hpp
  )

include_directories(${PROJECT_BINARY_DIR}/include
  ${CppMicroServices_SOURCE_DIR}/compendium/CM/include
  ${CppMicroServices_BINARY_DIR}/compendium/CM/include
  )

set(_additional_srcs
  ../src/CMActivator.cpp
  )

#-----------------------------------------------------------------------------
# Build the main test driver executable
#-----------------------------------------------------------------------------

# Generate a custom "bundle init" file for the test driver executable
usFunctionGenerateBundleInit(
  TARGET ${us_configurationadmin_test_exe_name}
  OUT _additional_srcs
  )

usFunctionGetResourceSource(
  TARGET ${us_configurationadmin_test_exe_name}
  OUT _additional_srcs)

add_executable(${us_configurationadmin_test_exe_name}
  ${_configurationadmin_tests}
  ${_additional_srcs})

if (US_COMPILER_MSVC AND BUILD_SHARED_LIBS)
  target_compile_options(${us_configurationadmin_test_exe_name} PRIVATE -DGTEST_LINKED_AS_SHARED_LIBRARY)
endif()

set_property(
  TARGET ${us_configurationadmin_test_exe_name}
  APPEND PROPERTY COMPILE_DEFINITIONS US_BUNDLE_NAME=main
  )

set_property(
  TARGET ${us_configurationadmin_test_exe_name}
  PROPERTY US_BUNDLE_NAME main
  )

target_include_directories(${us_configurationadmin_test_exe_name}
  PRIVATE $<TARGET_PROPERTY:util,INCLUDE_DIRECTORIES>)

# See https://github.com/google/googletest/blob/master/googlemock/docs/gmock_faq.md#msvc-gives-me-warning-c4301-or-c4373-when-i-define-a-mock-method-with-a-const-parameter-why
# MSVC_VERSION 1900 = MS Visual Studio 2015
if (MSVC AND MSVC_VERSION EQUAL 1900)
  target_compile_options(${us_configurationadmin_test_exe_name} PRIVATE "/wd4373")
endif()

# Disable deprecation warnings.
if (("${CMAKE_CXX_COMPILER_ID}" STREQUAL "Clang") OR
    ("${CMAKE_CXX_COMPILER_ID}" STREQUAL "AppleClang") OR
    ("${CMAKE_CXX_COMPILER_ID}" STREQUAL "GNU"))
  set_source_files_properties(${_configurationadmin_tests}
    PROPERTIES
    COMPILE_FLAGS -Wno-error=deprecated-declarations
    COMPILE_FLAGS -Wno-sign-compare
    )
endif()

set(_test_bundles
  ManagedServiceAndFactoryBundle
  TestBundleManagedServiceFactory
  TestBundleMultipleManagedServiceFactory
  )

target_link_libraries(${us_configurationadmin_test_exe_name}
  PRIVATE
  ${GTEST_BOTH_LIBRARIES}
  ${GMOCK_BOTH_LIBRARIES}
  ${${PROJECT_NAME}_LINK_LIBRARIES}
  CppMicroServices
  cm
  ConfigurationAdminObjs
  ConfigurationAdmin
  usTestInterfaces
  usLogService
  usAsyncWorkService
  usServiceComponent
  gtest
  gmock
  util
)

add_dependencies(${us_configurationadmin_test_exe_name}
  ConfigurationAdmin
  DeclarativeServices
  ${_test_bundles}
)

# Needed for clock_gettime with glibc < 2.17
if((UNIX AND NOT APPLE) AND NOT "${CMAKE_SYSTEM_NAME}" STREQUAL "Android")
  target_link_libraries(${us_configurationadmin_test_exe_name} PRIVATE rt)
endif()

# Run the GTest EXE from ctest.
add_test(NAME ${us_configurationadmin_test_exe_name}
  COMMAND ${us_configurationadmin_test_exe_name}
  WORKING_DIRECTORY ${CppMicroServices_BINARY_DIR}
)
set_property(TEST ${us_configurationadmin_test_exe_name} PROPERTY LABELS regular)
set_tests_properties(${us_configurationadmin_test_exe_name} PROPERTIES TIMEOUT 1200)

# Note: The memcheck test for usConfigurationAdminTests is tepmorarily disabled due to
#       a sporadic test failure which occurs when the test executable is ran with valgrind.
#       We currently do not have time to investigate the issue further but the test passes
#       when not subjected to valgrind; this doesn't mean there isn't an issue since this
#       appears to be caused by some kind of data race within ConfigAdmin or the test itself.
#
#       TODO: Investigate and fix the issue, re-enable the memcheck test

# Run the GTest EXE from valgrind
#[[
if(US_MEMCHECK_COMMAND)
  add_test(
    NAME memcheck_${us_configurationadmin_test_exe_name}
    COMMAND ${US_MEMCHECK_COMMAND} --max-threads=1000 --error-exitcode=1 ${US_RUNTIME_OUTPUT_DIRECTORY}/${us_configurationadmin_test_exe_name}
    WORKING_DIRECTORY ${CppMicroServices_BINARY_DIR}
    )
  set_property(TEST memcheck_${us_configurationadmin_test_exe_name} PROPERTY LABELS valgrind memcheck)
  # The test, when ran with Valgrind, executes extremely slow and doesn't allow for ConfigAdmin
  # notifications to properly be received so a higher timeout in the code is necessary. Due to this,
  # the test times out and we need to increase the timeout time.
  set_tests_properties(memcheck_${us_configurationadmin_test_exe_name} PROPERTIES TIMEOUT 1200)
endif()
]]

# Enable code coverage for GTest tests
if(US_COVERAGE_COMMAND)
  file(TO_NATIVE_PATH ${PROJECT_SOURCE_DIR} PROJECT_SOURCE_NATIVE_PATH)
  add_test(NAME coverage_${us_configurationadmin_test_exe_name}
    COMMAND ${US_COVERAGE_COMMAND} --verbose --cover_children --sources=${PROJECT_SOURCE_NATIVE_PATH} --export_type=binary:${us_configurationadmin_test_exe_name}.cov --continue_after_cpp_exception -- $<TARGET_FILE:${us_configurationadmin_test_exe_name}>
    WORKING_DIRECTORY ${CppMicroServices_BINARY_DIR}
    )
  set_property(TEST coverage_${us_configurationadmin_test_exe_name} PROPERTY LABELS opencppcoverage)
endif()

# Add a Preprocessor flag if code coverage is turned ON
if(US_ENABLE_COVERAGE)
  target_compile_definitions(${us_configurationadmin_test_exe_name} PUBLIC US_COVERAGE_ENABLED=1)
endif()

# Copy the Google Test libraries into the same folder as the
# executable so that they can be seen at runtime on Windows.
# Mac and Linux use RPATHs and do not need to do this.
if (WIN32 AND US_USE_SYSTEM_GTEST)
  foreach(lib_fullpath ${GTEST_BOTH_LIBRARIES})
    get_filename_component(dir ${lib_fullpath} DIRECTORY)
    get_filename_component(name_no_ext ${lib_fullpath} NAME_WE)
    set(dll_file "${dir}/${name_no_ext}${CMAKE_SHARED_LIBRARY_SUFFIX}")
    add_custom_command(TARGET ${us_configurationadmin_test_exe_name} POST_BUILD
    COMMAND ${CMAKE_COMMAND} -E copy_if_different
	"${dll_file}"
	$<TARGET_FILE_DIR:${us_configurationadmin_test_exe_name}>)
  endforeach(lib_fullpath)
endif()

#-----------------------------------------------------------------------------
# Add dependencies on test bundles if building shared libraries or
# link them if building static libraries
#-----------------------------------------------------------------------------

if(BUILD_SHARED_LIBS)
  # Add resources
  usFunctionEmbedResources(TARGET ${us_configurationadmin_test_exe_name}
			   FILES manifest.json)
else()
  # Add resources
  usFunctionEmbedResources(TARGET ${us_configurationadmin_test_exe_name}
			   FILES manifest.json
			   ZIP_ARCHIVES ${Framework_TARGET})
endif()
