#-----------------------------------------------------------------------------
# Build and run the GTest Suite of tests
#-----------------------------------------------------------------------------

set(scrcodegen_test_exe_name SCRCodeGenTests)

include_directories(
  ${GTEST_INCLUDE_DIRS}
  ${GMOCK_INCLUDE_DIRS}
  )


if(MSVC)
  add_compile_definitions(GTEST_HAS_STD_TUPLE_=1)
  add_compile_definitions(GTEST_HAS_TR1_TUPLE=0)
  add_compile_definitions(GTEST_LANG_CXX11=1)
endif()

#-----------------------------------------------------------------------------
# Add test source files
#-----------------------------------------------------------------------------
set(_scrcodegen_tests
  ../ComponentInfo.cpp
  ../ManifestParserImpl.cpp
  ../../../../third_party/jsoncpp.cpp
  ../Util.cpp
  main.cpp
  TestCodeGenerator.cpp
)

include_directories(${CppMicroServices_SOURCE_DIR}/framework/include
  ${CppMicroServices_BINARY_DIR}/include
  ${CppMicroServices_BINARY_DIR}/framework/include
  ${CppMicroServices_SOURCE_DIR}/compendium/tools/SCRCodeGen
  ${CppMicroServices_SOURCE_DIR}/third_party/googletest/googletest/include
  ${CppMicroServices_SOURCE_DIR}/third_party/googletest/googlemock/include
  ${CppMicroServices_SOURCE_DIR}/third_party
  )

add_executable(${scrcodegen_test_exe_name} ${_scrcodegen_tests})

# Disable deprecation warnings.
if (("${CMAKE_CXX_COMPILER_ID}" STREQUAL "Clang") OR
    ("${CMAKE_CXX_COMPILER_ID}" STREQUAL "AppleClang") OR
    ("${CMAKE_CXX_COMPILER_ID}" STREQUAL "GNU"))
  set_source_files_properties(${_scrcodegen_tests} PROPERTIES COMPILE_FLAGS -Wno-error=deprecated-declarations)
endif()

target_link_libraries(${scrcodegen_test_exe_name} 
	PRIVATE
		${GTEST_BOTH_LIBRARIES}
		${GMOCK_BOTH_LIBRARIES}
                CppMicroServices
)

# tests can be configured to run using add_test or GTEST_ADD_TEST (https://cmake.org/cmake/help/v3.0/module/FindGTest.html)
# GTEST_ADD_TEST will run a seprate EXE for each test and test fixture in the EXE.
# At this time, there isn't a need to use GTEST_ADD_TEST.

# Run the GTest EXE from ctest.
add_test(NAME ${scrcodegen_test_exe_name}
  COMMAND ${scrcodegen_test_exe_name}
  WORKING_DIRECTORY ${CppMicroServices_BINARY_DIR}
)
set_property(TEST ${scrcodegen_test_exe_name} PROPERTY LABELS regular)

# Run the GTest EXE from valgrind
if(US_MEMCHECK_COMMAND)
  add_test(
    NAME memcheck_${scrcodegen_test_exe_name}
    COMMAND ${US_MEMCHECK_COMMAND} --error-exitcode=1 ${US_RUNTIME_OUTPUT_DIRECTORY}/${scrcodegen_test_exe_name}
    WORKING_DIRECTORY ${CppMicroServices_BINARY_DIR}
    )
  set_property(TEST memcheck_${scrcodegen_test_exe_name} PROPERTY LABELS valgrind memcheck)
endif()

# Enable code coverage for GTest tests
if(US_COVERAGE_COMMAND)
  file(TO_NATIVE_PATH ${PROJECT_SOURCE_DIR} PROJECT_SOURCE_NATIVE_PATH)
  add_test(NAME coverage_${scrcodegen_test_exe_name}
    COMMAND ${US_COVERAGE_COMMAND} --verbose --cover_children --sources=${PROJECT_SOURCE_NATIVE_PATH} --export_type=binary:${scrcodegen_test_exe_name}.cov --continue_after_cpp_exception -- $<TARGET_FILE:${scrcodegen_test_exe_name}>
    WORKING_DIRECTORY ${CppMicroServices_BINARY_DIR}
    )
  set_property(TEST coverage_${scrcodegen_test_exe_name} PROPERTY LABELS opencppcoverage)
endif()

# Add a Preprocessor flag if code coverage is turned ON
if(US_ENABLE_COVERAGE)
  target_compile_definitions(${scrcodegen_test_exe_name} PUBLIC US_COVERAGE_ENABLED=1)
endif()

# Copy the Google Test libraries into the same folder as the
# executable so that they can be seen at runtime on Windows.
# Mac and Linux use RPATHs and do not need to do this.
if (WIN32 AND US_USE_SYSTEM_GTEST)
  foreach(lib_fullpath ${GTEST_BOTH_LIBRARIES})
    get_filename_component(dir ${lib_fullpath} DIRECTORY)
    get_filename_component(name_no_ext ${lib_fullpath} NAME_WE)
    set(dll_file "${dir}/${name_no_ext}${CMAKE_SHARED_LIBRARY_SUFFIX}")
    add_custom_command(TARGET ${scrcodegen_test_exe_name} POST_BUILD
    COMMAND ${CMAKE_COMMAND} -E copy_if_different
        "${dll_file}"
        $<TARGET_FILE_DIR:${scrcodegen_test_exe_name}>)
  endforeach(lib_fullpath)
endif()
