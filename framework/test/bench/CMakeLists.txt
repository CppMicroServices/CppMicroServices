#-----------------------------------------------------------------------------
# Build and run the GTest Suite of tests
#-----------------------------------------------------------------------------

set(us_bench_test_exe_name usFrameworkBenchTests)

include_directories(
  ${CMAKE_SOURCE_DIR}/third_party/benchmark/include
  ${CMAKE_CURRENT_SOURCE_DIR}/../util
  )

#-----------------------------------------------------------------------------
# Add test source files
#-----------------------------------------------------------------------------
set(_bench_src 
  ServiceRegistryTest.cpp
  ServiceTrackerTest.cpp
  AnyMapPerfTest.cpp
  bundleinstall.cpp
  ldapfilter.cpp
  ldappropexpr.cpp
  servicequery.cpp
  BundleTrackerTest.cpp
)

set(_additional_srcs
  ../util/TestUtilBundleListener.cpp
  ../util/TestUtils.cpp
  ../util/ImportTestBundles.cpp
  $<TARGET_OBJECTS:util>
  )

#-----------------------------------------------------------------------------
# Build the main test driver executable
#-----------------------------------------------------------------------------
# Generate a custom "bundle init" file for the test driver executable
usFunctionGenerateBundleInit(TARGET ${us_bench_test_exe_name} OUT _additional_srcs)
usFunctionGetResourceSource(TARGET ${us_bench_test_exe_name} OUT _additional_srcs)

add_executable(${us_bench_test_exe_name} ${_bench_src} ${_additional_srcs} )

target_include_directories(${us_bench_test_exe_name} PRIVATE $<TARGET_PROPERTY:util,INCLUDE_DIRECTORIES>)

target_link_libraries(${us_bench_test_exe_name} benchmark_main usLogService)
target_link_libraries(${us_bench_test_exe_name} ${Framework_TARGET})

set_property(TARGET ${us_bench_test_exe_name} APPEND PROPERTY COMPILE_DEFINITIONS US_BUNDLE_NAME=main)
set_property(TARGET ${us_bench_test_exe_name} PROPERTY US_BUNDLE_NAME main)



# Needed for clock_gettime with glibc < 2.17
if((UNIX AND NOT APPLE) AND NOT "${CMAKE_SYSTEM_NAME}" STREQUAL "Android")
  target_link_libraries(${us_bench_test_exe_name} rt)
endif()


if(BUILD_SHARED_LIBS)
    add_dependencies(${us_bench_test_exe_name} ${_us_test_bundle_libs})
    usFunctionEmbedResources(TARGET ${us_bench_test_exe_name}
                             FILES manifest.json)
else()
    target_link_libraries(${us_bench_test_exe_name} ${_us_test_bundle_libs})
    # Add resources
    usFunctionEmbedResources(TARGET ${us_bench_test_exe_name}
                             FILES manifest.json
                             ZIP_ARCHIVES ${Framework_TARGET} ${_us_test_bundle_libs})
endif()
