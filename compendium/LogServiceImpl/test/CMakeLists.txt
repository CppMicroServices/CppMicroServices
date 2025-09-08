#-----------------------------------------------------------------------------
# Build and run the GTest Suite of tests
#-----------------------------------------------------------------------------

set(us_logservice_test_exe_name usLogServiceTests)

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

configure_file("${CMAKE_CURRENT_SOURCE_DIR}/TestingConfig.h.in" "${PROJECT_BINARY_DIR}/include/LogServiceTestingConfig.h")

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

if(MINGW)
  # silence dangling references warning due to false positives
  add_compile_options(-Wno-dangling-reference)
endif()

if(MSVC)
  add_compile_definitions(GTEST_HAS_STD_TUPLE_=1)
  add_compile_definitions(GTEST_HAS_TR1_TUPLE=0)
  add_compile_definitions(GTEST_LANG_CXX11=1)
endif()

set(_logservice_tests
  TestLogService.cpp
  TestLoggerFactory.cpp
  main.cpp
  )

set(_logservice_additional_srcs
  ${CppMicroServices_SOURCE_DIR}/compendium/LogServiceImpl/src/LogServiceImpl.cpp
  ${CppMicroServices_SOURCE_DIR}/compendium/LogServiceImpl/src/LoggerFactoryImpl.cpp
  ${CppMicroServices_SOURCE_DIR}/compendium/LogServiceImpl/src/LoggerImpl.cpp
  )

set(_logservice_additional_hdrs
  ${CppMicroServices_SOURCE_DIR}/compendium/LogServiceImpl/src/LogServiceImpl.hpp
  ${CppMicroServices_SOURCE_DIR}/compendium/LogServiceImpl/src/LoggerFactoryImpl.hpp
  ${CppMicroServices_SOURCE_DIR}/compendium/LogServiceImpl/src/LoggerImpl.hpp
  )

#-----------------------------------------------------------------------------
# Build the main test driver executable
#-----------------------------------------------------------------------------
add_executable(${us_logservice_test_exe_name}
  ${_logservice_tests} ${_logservice_additional_srcs} ${_logservice_additional_hdrs})

if (US_COMPILER_MSVC AND BUILD_SHARED_LIBS)
  target_compile_options(${us_logservice_test_exe_name} PRIVATE -DGTEST_LINKED_AS_SHARED_LIBRARY)
endif()

if(APPLE)
  if(CMAKE_CXX_COMPILER_ID STREQUAL "AppleClang" AND CMAKE_CXX_COMPILER_VERSION VERSION_LESS "8.0.0")
    target_compile_definitions(${us_logservice_test_exe_name} PRIVATE SPDLOG_NO_TLS)
  endif()
endif()

# Disable deprecation warnings.
if (("${CMAKE_CXX_COMPILER_ID}" STREQUAL "Clang") OR
    ("${CMAKE_CXX_COMPILER_ID}" STREQUAL "AppleClang") OR
    ("${CMAKE_CXX_COMPILER_ID}" STREQUAL "GNU"))
  set_source_files_properties(${_logservice_tests}
    PROPERTIES
    COMPILE_FLAGS -Wno-error=deprecated-declarations
    COMPILE_FLAGS -Wno-sign-compare
    )
endif()

target_link_libraries(${us_logservice_test_exe_name}
  PRIVATE
  ${GTEST_BOTH_LIBRARIES}
  ${GMOCK_BOTH_LIBRARIES}
  ${${PROJECT_NAME}_LINK_LIBRARIES}
  CppMicroServices
  usLogService
  gtest
  gmock
  util
  )

target_include_directories(${us_logservice_test_exe_name} PRIVATE
  ${CMAKE_CURRENT_SOURCE_DIR}/src
  ${CppMicroServices_BINARY_DIR}/include
  ${CppMicroServices_SOURCE_DIR}/framework/include
  ${CppMicroServices_BINARY_DIR}/framework/include
  ${CppMicroServices_SOURCE_DIR}/compendium/LogService/include
  ${CppMicroServices_BINARY_DIR}/compendium/LogService/include
  ${CppMicroServices_SOURCE_DIR}/compendium/LogServiceImpl/src
  ${CppMicroServices_SOURCE_DIR}/third_party/googletest/googletest/include
  ${CppMicroServices_SOURCE_DIR}/third_party/googletest/googlemock/include
  ${CppMicroServices_SOURCE_DIR}/third_party/spdlog/include
  )

# Needed for clock_gettime with glibc < 2.17
if((UNIX AND NOT APPLE) AND NOT "${CMAKE_SYSTEM_NAME}" STREQUAL "Android")
  target_link_libraries(${us_logservice_test_exe_name} PRIVATE rt)
endif()

# Run the GTest EXE from ctest.
add_test(NAME ${us_logservice_test_exe_name}
  COMMAND ${us_logservice_test_exe_name}
  WORKING_DIRECTORY ${CppMicroServices_BINARY_DIR}
)
set_property(TEST ${us_logservice_test_exe_name} PROPERTY LABELS regular)
set_tests_properties(${us_logservice_test_exe_name} PROPERTIES TIMEOUT 1200)

# Run the GTest EXE from valgrind
if(US_MEMCHECK_COMMAND)
  add_test(
    NAME memcheck_${us_logservice_test_exe_name}
    COMMAND ${US_MEMCHECK_COMMAND} --max-threads=1000 --error-exitcode=1 ${US_RUNTIME_OUTPUT_DIRECTORY}/${us_logservice_test_exe_name}
    WORKING_DIRECTORY ${CppMicroServices_BINARY_DIR}
    )
  set_property(TEST memcheck_${us_logservice_test_exe_name} PROPERTY LABELS valgrind memcheck)
endif()

# Enable code coverage for GTest tests
if(US_COVERAGE_COMMAND)
  file(TO_NATIVE_PATH ${PROJECT_SOURCE_DIR} PROJECT_SOURCE_NATIVE_PATH)
  add_test(NAME coverage_${us_logservice_test_exe_name}
    COMMAND ${US_COVERAGE_COMMAND} --verbose --cover_children --sources=${PROJECT_SOURCE_NATIVE_PATH} --export_type=binary:${us_logservice_test_exe_name}.cov --continue_after_cpp_exception -- $<TARGET_FILE:${us_logservice_test_exe_name}>
    WORKING_DIRECTORY ${CppMicroServices_BINARY_DIR}
    )
  set_property(TEST coverage_${us_logservice_test_exe_name} PROPERTY LABELS opencppcoverage)
endif()

# Add a Preprocessor flag if code coverage is turned ON
if(US_ENABLE_COVERAGE)
  target_compile_definitions(${us_logservice_test_exe_name} PUBLIC US_COVERAGE_ENABLED=1)
endif()

# Copy the Google Test libraries into the same folder as the
# executable so that they can be seen at runtime on Windows.
# Mac and Linux use RPATHs and do not need to do this.
if (WIN32 AND US_USE_SYSTEM_GTEST)
  foreach(lib_fullpath ${GTEST_BOTH_LIBRARIES})
    get_filename_component(dir ${lib_fullpath} DIRECTORY)
    get_filename_component(name_no_ext ${lib_fullpath} NAME_WE)
    set(dll_file "${dir}/${name_no_ext}${CMAKE_SHARED_LIBRARY_SUFFIX}")
    add_custom_command(TARGET ${us_logservice_test_exe_name} POST_BUILD
    COMMAND ${CMAKE_COMMAND} -E copy_if_different
	"${dll_file}"
	$<TARGET_FILE_DIR:${us_logservice_test_exe_name}>)
  endforeach(lib_fullpath)
endif()
