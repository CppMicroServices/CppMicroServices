
add_subdirectory(bundles)
add_subdirectory(driver)
add_subdirectory(gtest)

# Add a Preprocessor flag if code coverage is turned ON
if(US_ENABLE_COVERAGE)
  target_compile_definitions(${_test_driver} PUBLIC US_COVERAGE_ENABLED=1)
endif()