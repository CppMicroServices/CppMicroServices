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
  BenchTest.cpp
)


#-----------------------------------------------------------------------------
# Build the main test driver executable
#-----------------------------------------------------------------------------
add_executable(${us_bench_test_exe_name} ${_bench_src})

target_link_libraries(${us_bench_test_exe_name} benchmark_main)
target_link_libraries(${us_bench_test_exe_name} ${Framework_TARGET})

# Needed for clock_gettime with glibc < 2.17
if(UNIX AND NOT APPLE)
  target_link_libraries(${us_bench_test_exe_name} rt)
endif()

