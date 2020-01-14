#-----------------------------------------------------------------------------
# Build and run the GTest Suite of tests
#-----------------------------------------------------------------------------

set(us_declarativeservices_test_exe_name usDeclarativeServicesTests)

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

configure_file("${CMAKE_CURRENT_SOURCE_DIR}/TestingConfig.h.in" "${PROJECT_BINARY_DIR}/include/DSTestingConfig.h")

include_directories(
  ${GTEST_INCLUDE_DIRS}
  ${GMOCK_INCLUDE_DIRS}
  )


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

#-----------------------------------------------------------------------------
# Add test source files
#-----------------------------------------------------------------------------
set(_declarativeservices_tests
  ActivatorTest.cpp
  SCRLoggerTest.cpp
  TestCCActiveState.cpp
  TestCCRegisteredState.cpp
  TestCCUnsatisfiedReferenceState.cpp
  TestComponentConfigurationImpl.cpp
  TestComponentContextImpl.cpp
  TestComponentManagerDisabledState.cpp
  TestComponentManagerEnabledState.cpp
  TestComponentManagerImpl.cpp
  TestComponentRegistry.cpp
  TestCounterLatch.cpp
  TestMetadataParserFactory.cpp
  TestMetadataParserImplV1.cpp
  TestReferenceManagerImpl.cpp
  TestReferenceMetadataParserV1.cpp
  TestReferenceSelfSatisfyDeadLock.cpp
  TestRegistrationManager.cpp
  TestSCRBundleExtension.cpp
  TestServiceComponentRuntimeImpl.cpp
  TestServiceMetadataParserV1.cpp
  TestSingletonComponentConfiguration.cpp
  TestBundleStartOrder.cpp
  TestComponentDescription.cpp
  TestComponentInitialState.cpp
  TestComponentLifecycle.cpp
  TestDictionary.cpp
  TestFixture.hpp
  TestServiceComponentRuntime.cpp
  TestServiceProvider.cpp
  ImportTestBundles.cpp
  main.cpp
)

include_directories(${PROJECT_BINARY_DIR}/include)

set(_additional_srcs
  TestUtils.cpp
  ../src/SCRActivator.cpp
  )

#-----------------------------------------------------------------------------
# Build the main test driver executable
#-----------------------------------------------------------------------------

# Generate a custom "bundle init" file for the test driver executable
usFunctionGenerateBundleInit(
  TARGET ${us_declarativeservices_test_exe_name}
  OUT _additional_srcs
  )

usFunctionGetResourceSource(
  TARGET ${us_declarativeservices_test_exe_name}
  OUT _additional_srcs)

add_executable(${us_declarativeservices_test_exe_name}
  ${_declarativeservices_tests}
  ${_additional_srcs})

set_property(
  TARGET ${us_declarativeservices_test_exe_name}
  APPEND PROPERTY COMPILE_DEFINITIONS US_BUNDLE_NAME=main
  )

set_property(
  TARGET ${us_declarativeservices_test_exe_name}
  PROPERTY US_BUNDLE_NAME main
  )

target_include_directories(${us_declarativeservices_test_exe_name}
  PRIVATE $<TARGET_PROPERTY:util,INCLUDE_DIRECTORIES>)

# Disable deprecation warnings.
if (("${CMAKE_CXX_COMPILER_ID}" STREQUAL "Clang") OR
    ("${CMAKE_CXX_COMPILER_ID}" STREQUAL "AppleClang") OR
    ("${CMAKE_CXX_COMPILER_ID}" STREQUAL "GNU"))
  set_source_files_properties(${_declarativeservices_tests}
    PROPERTIES
    COMPILE_FLAGS -Wno-error=deprecated-declarations
    COMPILE_FLAGS -Wno-sign-compare
    )
endif()

set(_test_bundles
  BenchmarkDS
  DSFrenchDictionary
  DSGraph01
  DSGraph02
  DSGraph03
  DSGraph04
  DSGraph05
  DSGraph06
  DSGraph07
  DSSpellChecker
  TestBundleDSTOI1
  TestBundleDSTOI10
  TestBundleDSTOI12
  TestBundleDSTOI14
  TestBundleDSTOI15
  TestBundleDSTOI16
  TestBundleDSTOI2
  TestBundleDSTOI3
  TestBundleDSTOI5
  TestBundleDSTOI6
  TestBundleDSTOI9
  EnglishDictionary
  )

target_link_libraries(${us_declarativeservices_test_exe_name} 
  PRIVATE
  ${GTEST_BOTH_LIBRARIES}
  ${GMOCK_BOTH_LIBRARIES}
  ${${PROJECT_NAME}_LINK_LIBRARIES}
  DeclarativeServicesObjs
  usTestInterfaces
  usServiceComponent
  usLogService
  usIDictionaryService
  usISpellCheckService
  gtest
  gmock
  util
  ${_test_bundles}
)

# Needed for clock_gettime with glibc < 2.17
if(UNIX AND NOT APPLE)
  target_link_libraries(${us_declarativeservices_test_exe_name} PRIVATE rt)
endif()


# tests can be configured to run using add_test or GTEST_ADD_TEST (https://cmake.org/cmake/help/v3.0/module/FindGTest.html)
# GTEST_ADD_TEST will run a seprate EXE for each test and test fixture in the EXE.
# At this time, there isn't a need to use GTEST_ADD_TEST.

# Run the GTest EXE from ctest.
add_test(NAME ${us_declarativeservices_test_exe_name}
  COMMAND ${us_declarativeservices_test_exe_name}
  WORKING_DIRECTORY ${CppMicroServices_BINARY_DIR}
)
set_property(TEST ${us_declarativeservices_test_exe_name} PROPERTY LABELS regular)

# Run the GTest EXE from valgrind
if(US_MEMCHECK_COMMAND)
  add_test(
    NAME memcheck_${us_declarativeservices_test_exe_name}
    COMMAND ${US_MEMCHECK_COMMAND} --max-threads=1000 --error-exitcode=1 ${US_RUNTIME_OUTPUT_DIRECTORY}/${us_declarativeservices_test_exe_name}
    WORKING_DIRECTORY ${CppMicroServices_BINARY_DIR}
    )
  set_property(TEST memcheck_${us_declarativeservices_test_exe_name} PROPERTY LABELS valgrind memcheck)
endif()

# Enable code coverage for GTest tests
if(US_COVERAGE_COMMAND)
  file(TO_NATIVE_PATH ${PROJECT_SOURCE_DIR} PROJECT_SOURCE_NATIVE_PATH)
  add_test(NAME coverage_${us_declarativeservices_test_exe_name}
    COMMAND ${US_COVERAGE_COMMAND} --verbose --cover_children --sources=${PROJECT_SOURCE_NATIVE_PATH} --export_type=binary:${us_declarativeservices_test_exe_name}.cov --continue_after_cpp_exception -- $<TARGET_FILE:${us_declarativeservices_test_exe_name}>
    WORKING_DIRECTORY ${CppMicroServices_BINARY_DIR}
    )
  set_property(TEST coverage_${us_declarativeservices_test_exe_name} PROPERTY LABELS opencppcoverage)
endif()

# Add a Preprocessor flag if code coverage is turned ON
if(US_ENABLE_COVERAGE)
  target_compile_definitions(${us_declarativeservices_test_exe_name} PUBLIC US_COVERAGE_ENABLED=1)
endif()

# Copy the Google Test libraries into the same folder as the
# executable so that they can be seen at runtime on Windows.
# Mac and Linux use RPATHs and do not need to do this.
if (WIN32 AND US_USE_SYSTEM_GTEST)
  foreach(lib_fullpath ${GTEST_BOTH_LIBRARIES})
    get_filename_component(dir ${lib_fullpath} DIRECTORY)
    get_filename_component(name_no_ext ${lib_fullpath} NAME_WE)
    set(dll_file "${dir}/${name_no_ext}${CMAKE_SHARED_LIBRARY_SUFFIX}")
    add_custom_command(TARGET ${us_declarativeservices_test_exe_name} POST_BUILD
    COMMAND ${CMAKE_COMMAND} -E copy_if_different
        "${dll_file}"
        $<TARGET_FILE_DIR:${us_declarativeservices_test_exe_name}>)
  endforeach(lib_fullpath)
endif()

#-----------------------------------------------------------------------------
# Add dependencies on test bundles if building shared libraries or 
# link them if building static libraries
#-----------------------------------------------------------------------------

if(BUILD_SHARED_LIBS)
  # Add resources
  usFunctionEmbedResources(TARGET ${us_declarativeservices_test_exe_name}
                           FILES manifest.json)
else()
  # Add resources
  usFunctionEmbedResources(TARGET ${us_declarativeservices_test_exe_name}
                           FILES manifest.json
                           ZIP_ARCHIVES ${Framework_TARGET} ${_test_bundles})
endif()

