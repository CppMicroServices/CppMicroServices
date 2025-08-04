#-----------------------------------------------------------------------------
# Build and run the GoogleBenchmark Suite of tests
#-----------------------------------------------------------------------------

set(us_declarativeservices_bench_test_exe_name usDeclarativeServicesBenchmarkTests)

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

configure_file("${CMAKE_CURRENT_SOURCE_DIR}/../TestingConfig.h.in" "${PROJECT_BINARY_DIR}/include/DSTestingConfig.h")

include_directories(
  ${GTEST_INCLUDE_DIRS}
  ${GMOCK_INCLUDE_DIRS}
  ${CMAKE_SOURCE_DIR}/third_party/benchmark/include
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

add_compile_definitions(BOOST_DATE_TIME_NO_LIB)
add_compile_definitions(BOOST_REGEX_NO_LIB)


#-----------------------------------------------------------------------------
# Add test source files
#-----------------------------------------------------------------------------
set(_declarativeservices_benchmark_tests
  GetDSServiceTest.cpp
)

include_directories(${PROJECT_BINARY_DIR}/include
  ${CppMicroServices_SOURCE_DIR}/compendium/CM/include
  ${CppMicroServices_BINARY_DIR}/compendium/CM/include
  )

set(_additional_srcs
  ../TestUtils.cpp
  ../../src/SCRActivator.cpp
  )

#-----------------------------------------------------------------------------
# Build the main test driver executable
#-----------------------------------------------------------------------------

# Generate a custom "bundle init" file for the test driver executable
usFunctionGenerateBundleInit(
  TARGET ${us_declarativeservices_bench_test_exe_name}
  OUT _additional_srcs
  )

usFunctionGetResourceSource(
  TARGET ${us_declarativeservices_bench_test_exe_name}
  OUT _additional_srcs)

add_executable(${us_declarativeservices_bench_test_exe_name}
  ${_declarativeservices_benchmark_tests}
  ${_additional_srcs})

if (US_COMPILER_MSVC AND BUILD_SHARED_LIBS)
  target_compile_options(${us_declarativeservices_bench_test_exe_name} PRIVATE -DGTEST_LINKED_AS_SHARED_LIBRARY)
endif()

set_property(
  TARGET ${us_declarativeservices_bench_test_exe_name}
  APPEND PROPERTY COMPILE_DEFINITIONS US_BUNDLE_NAME=main
  )

set_property(
  TARGET ${us_declarativeservices_bench_test_exe_name}
  PROPERTY US_BUNDLE_NAME main
  )

target_include_directories(${us_declarativeservices_bench_test_exe_name}
  PRIVATE $<TARGET_PROPERTY:util,INCLUDE_DIRECTORIES>)

# Disable deprecation warnings.
if (("${CMAKE_CXX_COMPILER_ID}" STREQUAL "Clang") OR
    ("${CMAKE_CXX_COMPILER_ID}" STREQUAL "AppleClang") OR
    ("${CMAKE_CXX_COMPILER_ID}" STREQUAL "GNU"))
  set_source_files_properties(${_declarativeservices_benchmark_tests}
    PROPERTIES
    COMPILE_FLAGS -Wno-error=deprecated-declarations
    COMPILE_FLAGS -Wno-sign-compare
    )
endif()

set(_test_bundles
  TestBundleDSTOI1
  )

target_link_libraries(${us_declarativeservices_bench_test_exe_name}
  PRIVATE
  ${${PROJECT_NAME}_LINK_LIBRARIES}
  ConfigurationAdmin
  DeclarativeServices
  DeclarativeServicesObjs
  usTestInterfaces
  usServiceComponent
  usLogService
  usAsyncWorkService
  gtest
  benchmark_main
  util
)

add_dependencies(${us_declarativeservices_bench_test_exe_name}
  DeclarativeServices
  ${_test_bundles}
)

# Needed for clock_gettime with glibc < 2.17
if((UNIX AND NOT APPLE) AND NOT "${CMAKE_SYSTEM_NAME}" STREQUAL "Android")
  target_link_libraries(${us_declarativeservices_bench_test_exe_name} PRIVATE rt)
endif()

# Needed for 'TestComponentLifecycle' test 
if(MINGW)
  target_link_libraries(${us_declarativeservices_bench_test_exe_name} PRIVATE -lpsapi)
endif()

#-----------------------------------------------------------------------------
# Add dependencies on test bundles if building shared libraries or
# link them if building static libraries
#-----------------------------------------------------------------------------

if(BUILD_SHARED_LIBS)
  # Add resources
  usFunctionEmbedResources(TARGET ${us_declarativeservices_bench_test_exe_name}
                           FILES manifest.json)
else()
  # Add resources
  usFunctionEmbedResources(TARGET ${us_declarativeservices_bench_test_exe_name}
                           FILES manifest.json
                           ZIP_ARCHIVES ${Framework_TARGET} ${_test_bundles})
endif()
