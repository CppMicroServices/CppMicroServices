#-----------------------------------------------------------------------------
# Build and run the GTest Suite of tests
#-----------------------------------------------------------------------------
set(us_servicecomponent_test_exe_name usServiceComponentTests)

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
set(_servicecomponent_tests
  TestComponentInstance.cpp             
  suite_registration.cpp
)

include_directories(${CppMicroServices_SOURCE_DIR}/framework/include
  ${CppMicroServices_BINARY_DIR}/include
  ${CppMicroServices_BINARY_DIR}/framework/include
  ${CppMicroServices_BINARY_DIR}/compendium/ServiceComponent/include
  ${CppMicroServices_BINARY_DIR}/compendium/LogService/include
  ${CppMicroServices_SOURCE_DIR}/compendium/ServiceComponent/include
  ${CppMicroServices_SOURCE_DIR}/compendium/LogService/include
  ${CppMicroServices_SOURCE_DIR}/compendium/DeclarativeServices/src
  ${CppMicroServices_SOURCE_DIR}/third_party/googletest/googletest/include
  ${CppMicroServices_SOURCE_DIR}/third_party/googletest/googlemock/include
  )

add_executable(${us_servicecomponent_test_exe_name} ${_servicecomponent_tests})

if (US_COMPILER_MSVC AND BUILD_SHARED_LIBS)
  target_compile_options(${us_servicecomponent_test_exe_name} PRIVATE -DGTEST_LINKED_AS_SHARED_LIBRARY)
endif()

# Disable deprecation warnings.
if (("${CMAKE_CXX_COMPILER_ID}" STREQUAL "Clang") OR
    ("${CMAKE_CXX_COMPILER_ID}" STREQUAL "AppleClang") OR
    ("${CMAKE_CXX_COMPILER_ID}" STREQUAL "GNU"))
  set_source_files_properties(${_servicecomponent_tests} PROPERTIES COMPILE_FLAGS -Wno-error=deprecated-declarations)
endif()

target_link_libraries(${us_servicecomponent_test_exe_name} 
	PRIVATE
		${GTEST_BOTH_LIBRARIES}
		${GMOCK_BOTH_LIBRARIES}
        CppMicroServices
        usServiceComponent
)

# tests can be configured to run using add_test or GTEST_ADD_TEST (https://cmake.org/cmake/help/v3.0/module/FindGTest.html)
# GTEST_ADD_TEST will run a seprate EXE for each test and test fixture in the EXE.
# At this time, there isn't a need to use GTEST_ADD_TEST.

# Run the GTest EXE from ctest.
add_test(NAME ${us_servicecomponent_test_exe_name}
  COMMAND ${us_servicecomponent_test_exe_name}
  WORKING_DIRECTORY ${CppMicroServices_BINARY_DIR}
)
set_property(TEST ${us_servicecomponent_test_exe_name} PROPERTY LABELS regular)

# Run the GTest EXE from valgrind
if(US_MEMCHECK_COMMAND)
  add_test(
    NAME memcheck_${us_servicecomponent_test_exe_name}
    COMMAND ${US_MEMCHECK_COMMAND} --error-exitcode=1 ${US_RUNTIME_OUTPUT_DIRECTORY}/${us_servicecomponent_test_exe_name}
    WORKING_DIRECTORY ${CppMicroServices_BINARY_DIR}
    )
  set_property(TEST memcheck_${us_servicecomponent_test_exe_name} PROPERTY LABELS valgrind memcheck)
endif()

# Enable code coverage for GTest tests
if(US_COVERAGE_COMMAND)
  file(TO_NATIVE_PATH ${PROJECT_SOURCE_DIR} PROJECT_SOURCE_NATIVE_PATH)
  add_test(NAME coverage_${us_servicecomponent_test_exe_name}
    COMMAND ${US_COVERAGE_COMMAND} --verbose --cover_children --sources=${PROJECT_SOURCE_NATIVE_PATH} --export_type=binary:${us_servicecomponent_test_exe_name}.cov --continue_after_cpp_exception -- $<TARGET_FILE:${us_servicecomponent_test_exe_name}>
    WORKING_DIRECTORY ${CppMicroServices_BINARY_DIR}
    )
  set_property(TEST coverage_${us_servicecomponent_test_exe_name} PROPERTY LABELS opencppcoverage)
endif()

# Add a Preprocessor flag if code coverage is turned ON
if(US_ENABLE_COVERAGE)
  target_compile_definitions(${us_servicecomponent_test_exe_name} PUBLIC US_COVERAGE_ENABLED=1)
endif()

# Copy the Google Test libraries into the same folder as the
# executable so that they can be seen at runtime on Windows.
# Mac and Linux use RPATHs and do not need to do this.
if (WIN32 AND US_USE_SYSTEM_GTEST)
  foreach(lib_fullpath ${GTEST_BOTH_LIBRARIES})
    get_filename_component(dir ${lib_fullpath} DIRECTORY)
    get_filename_component(name_no_ext ${lib_fullpath} NAME_WE)
    set(dll_file "${dir}/${name_no_ext}${CMAKE_SHARED_LIBRARY_SUFFIX}")
    add_custom_command(TARGET ${us_servicecomponent_test_exe_name} POST_BUILD
    COMMAND ${CMAKE_COMMAND} -E copy_if_different
        "${dll_file}"
        $<TARGET_FILE_DIR:${us_servicecomponent_test_exe_name}>)
  endforeach(lib_fullpath)
endif()

#Compile-only tests addition
#Adding executables for compile-only test files
add_executable(usServiceComponentRefOrderTest TestCompInst_RefOrder.cpp)
add_executable(usServiceComponentRefOrderMultipleCardinalityTest TestCompInst_RefOrderMultipleCardinality.cpp)
add_executable(usServiceComponentInvalidInheritanceTest TestCompInst_InvalidInheritance.cpp)
add_executable(usServiceComponentNoDepsNoDefaultCtorTest TestCompInst_NoDepsNoDefaultCtor.cpp)
add_executable(usServiceComponentNoDefaultCtorTest TestCompInst_NoInjectNoDefaultCtor.cpp)
add_executable(usServiceComponentRefCountTest TestCompInst_RefCount.cpp)
add_executable(usServiceComponentModifiedMethod TestCompInst_IncorrectModifiedMethod.cpp)
add_executable(usServiceComponentActivateMethod TestCompInst_IncorrectActivateMethod.cpp)
add_executable(usServiceComponentDeactivateMethod TestCompInst_IncorrectDeactivateMethod.cpp)

set(compile_only_exe_names usServiceComponentRefOrderTest usServiceComponentRefOrderMultipleCardinalityTest usServiceComponentInvalidInheritanceTest usServiceComponentNoDepsNoDefaultCtorTest usServiceComponentNoDefaultCtorTest usServiceComponentRefCountTest usServiceComponentModifiedMethod usServiceComponentActivateMethod usServiceComponentDeactivateMethod)

foreach(exe_name ${compile_only_exe_names})
    
    if (US_COMPILER_MSVC AND BUILD_SHARED_LIBS)
        target_compile_options(${exe_name} PRIVATE -DGTEST_LINKED_AS_SHARED_LIBRARY)
    endif()

    target_link_libraries(${exe_name} 
	PRIVATE
		${GTEST_BOTH_LIBRARIES}
		${GMOCK_BOTH_LIBRARIES}
        CppMicroServices
        usServiceComponent
    )

    #Excluding compile-only tests from default build to avoid build failure
    set_target_properties(${exe_name} PROPERTIES EXCLUDE_FROM_ALL TRUE EXCLUDE_FROM_DEFAULT_BUILD TRUE)

    #For compile-only tests we add a test with Cmake command to build the target only as we expect a build failure
    add_test(NAME ${exe_name}
    COMMAND ${CMAKE_COMMAND} --build . --config ${CMAKE_BUILD_TYPE} --target ${exe_name}
    WORKING_DIRECTORY ${CppMicroServices_BINARY_DIR}
    )

endforeach()

#Set test properties for compile-only tests to check if compilation fails and corresponding errors are generated in the log
set_tests_properties(usServiceComponentRefOrderTest PROPERTIES PASS_REGULAR_EXPRESSION ".*An appropriate constructor was not found and/or the service implementation does not implement all of the service interface's methods. A constructor with service reference input parameters or a constructor with an AnyMap input parameter for configuration properties and service reference input parameters is required when inject-references is true.*")
set_tests_properties(usServiceComponentRefOrderMultipleCardinalityTest PROPERTIES PASS_REGULAR_EXPRESSION ".*An appropriate constructor was not found and/or the service implementation does not implement all of the service interface's methods. A constructor with service reference input parameters or a constructor with an AnyMap input parameter for configuration properties and service reference input parameters is required when inject-references is true.*")
set_tests_properties(usServiceComponentInvalidInheritanceTest PROPERTIES PASS_REGULAR_EXPRESSION ".*[error C2440: 'static_cast': cannot convert from].*")
set_tests_properties(usServiceComponentNoDepsNoDefaultCtorTest PROPERTIES PASS_REGULAR_EXPRESSION ".*An appropriate constructor was not found and/or the service implementation does not implement all of the service interface's methods. A default constructor or a constructor with an AnyMap input parameter for the configuration properties is required when inject-references is false..*")
set_tests_properties(usServiceComponentNoDefaultCtorTest PROPERTIES PASS_REGULAR_EXPRESSION ".*An appropriate constructor was not found and/or the service implementation does not implement all of the service interface's methods. A constructor with service reference input parameters or a constructor with an AnyMap input parameter for configuration properties and service reference input parameters is required when inject-references is true.*")
set_tests_properties(usServiceComponentRefCountTest PROPERTIES PASS_REGULAR_EXPRESSION ".*An appropriate constructor was not found and/or the service implementation does not implement all of the service interface's methods. A constructor with service reference input parameters or a constructor with an AnyMap input parameter for configuration properties and service reference input parameters is required when inject-references is true.*")
set_tests_properties(usServiceComponentModifiedMethod PROPERTIES PASS_REGULAR_EXPRESSION ".*Error: A Modified method was not found with the appropriate signature.*")
set_tests_properties(usServiceComponentActivateMethod PROPERTIES PASS_REGULAR_EXPRESSION ".*Error: An Activate method was not found with the appropriate signature.*")
set_tests_properties(usServiceComponentDeactivateMethod PROPERTIES PASS_REGULAR_EXPRESSION ".*Error: A Deactivate method was not found with the appropriate signature.*")