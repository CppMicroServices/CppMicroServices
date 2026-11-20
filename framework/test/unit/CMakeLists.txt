#-----------------------------------------------------------------------------
# White-box unit tests for the core framework
#
# Unlike usFrameworkTests (which links the shipping CppMicroServices library
# and exercises it through its public API and real test bundles), this
# executable links the framework *object files* directly
# (CppMicroServicesObjs). That gives tests full access to private framework
# classes without exporting them from the shipping library.
#
# Ground rule: this executable must NOT link CppMicroServices, and must NOT
# install shared test bundles. Shared test bundles link CppMicroServices
# themselves, which would put a second copy of the framework in the process.
# Use mocks / in-memory fakes instead. Black-box tests that need real bundles
# belong in ../gtest.
#-----------------------------------------------------------------------------

set(us_fwk_unit_test_exe_name usFrameworkUnitTests)

set(_fwk_unit_tests
  BundleArchiveUnitTest.cpp
  BundleManifestUnitTest.cpp
  CoreBundleContextUnitTest.cpp
  PropertiesUnitTest.cpp
  SystemBundleUnitTest.cpp
  main.cpp
  )

set(_fwk_unit_test_headers
  mocks/MockBundleStorage.h
  )

add_executable(${us_fwk_unit_test_exe_name} ${_fwk_unit_tests} ${_fwk_unit_test_headers})

target_include_directories(${us_fwk_unit_test_exe_name}
  PRIVATE
    ${CMAKE_CURRENT_SOURCE_DIR}
    ${GTEST_INCLUDE_DIRS}
    ${GMOCK_INCLUDE_DIRS}
  )

target_compile_definitions(${us_fwk_unit_test_exe_name}
  PRIVATE US_BUNDLE_NAME=main
  )

if(US_COMPILER_MSVC AND BUILD_SHARED_LIBS)
  target_compile_definitions(${us_fwk_unit_test_exe_name} PRIVATE GTEST_LINKED_AS_SHARED_LIBRARY)
endif()

# CppMicroServicesObjs carries the framework include paths (public + private)
# and CppMicroServices_EXPORTS as usage requirements. 'util' supplies the
# remaining objects that the shipping library also links in.
target_link_libraries(${us_fwk_unit_test_exe_name}
  PRIVATE
    CppMicroServicesObjs
    util
    ${GTEST_BOTH_LIBRARIES}
    ${GMOCK_BOTH_LIBRARIES}
  )

if(UNIX AND NOT APPLE AND NOT "${CMAKE_SYSTEM_NAME}" STREQUAL "Android")
  target_link_libraries(${us_fwk_unit_test_exe_name} PRIVATE rt)
endif()

add_test(NAME ${us_fwk_unit_test_exe_name}
  COMMAND ${us_fwk_unit_test_exe_name}
  WORKING_DIRECTORY ${CppMicroServices_BINARY_DIR}
  )
set_property(TEST ${us_fwk_unit_test_exe_name} PROPERTY LABELS regular)

if(US_MEMCHECK_COMMAND)
  add_test(NAME memcheck_${us_fwk_unit_test_exe_name}
    COMMAND ${US_MEMCHECK_COMMAND} --error-exitcode=1 ${US_RUNTIME_OUTPUT_DIRECTORY}/${us_fwk_unit_test_exe_name}
    WORKING_DIRECTORY ${CppMicroServices_BINARY_DIR})
  set_property(TEST memcheck_${us_fwk_unit_test_exe_name} PROPERTY LABELS valgrind memcheck)
endif()

if(US_COVERAGE_COMMAND)
  file(TO_NATIVE_PATH ${PROJECT_SOURCE_DIR} PROJECT_SOURCE_NATIVE_PATH)
  add_test(NAME coverage_${us_fwk_unit_test_exe_name}
    COMMAND ${US_COVERAGE_COMMAND} --verbose --cover_children --sources=${PROJECT_SOURCE_NATIVE_PATH} --export_type=binary:${us_fwk_unit_test_exe_name}.cov --continue_after_cpp_exception -- $<TARGET_FILE:${us_fwk_unit_test_exe_name}>
    WORKING_DIRECTORY ${CppMicroServices_BINARY_DIR})
  set_property(TEST coverage_${us_fwk_unit_test_exe_name} PROPERTY LABELS opencppcoverage)
endif()

if(US_ENABLE_COVERAGE)
  target_compile_definitions(${us_fwk_unit_test_exe_name} PRIVATE US_COVERAGE_ENABLED=1)
endif()
