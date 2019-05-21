
enable_language(CXX)

include_directories(../../third_party)

set(srcs main.cpp)

add_executable(jsonschemavalidator ${srcs})
set_property(TARGET jsonschemavalidator PROPERTY OUTPUT_NAME jsonschemavalidator)

if(NOT US_NO_INSTALL)
    install(TARGETS jsonschemavalidator
            EXPORT ${PROJECT_NAME}Targets
            FRAMEWORK DESTINATION . ${US_SDK_INSTALL_COMPONENT}
            RUNTIME DESTINATION ${TOOLS_INSTALL_DIR} ${US_SDK_INSTALL_COMPONENT})
endif()

# Add the tests
add_test(NAME Test_Schema_Validator_NoArgs
         COMMAND $<TARGET_FILE:jsonschemavalidator>
         WORKING_DIRECTORY ${CppMicroServices_BINARY_DIR})
add_test(NAME Test_Schema_Validator_Help
         COMMAND $<TARGET_FILE:jsonschemavalidator>
         WORKING_DIRECTORY ${CppMicroServices_BINARY_DIR})
add_test(NAME Test_Schema_Validator_ValidArgs
         COMMAND $<TARGET_FILE:jsonschemavalidator> -s ${CMAKE_CURRENT_SOURCE_DIR}/test_data/manifest_schema.json -j ${CMAKE_CURRENT_SOURCE_DIR}/test_data/manifest.json
         WORKING_DIRECTORY ${CppMicroServices_BINARY_DIR})
add_test(NAME NegativeTest_Schema_Validator_InvalidPaths
         COMMAND $<TARGET_FILE:jsonschemavalidator> -s schema.json -j data.json
         WORKING_DIRECTORY ${CppMicroServices_BINARY_DIR})
add_test(NAME NegativeTest_Schema_Validator_No_Data_Arg
         COMMAND $<TARGET_FILE:jsonschemavalidator> -s ${CMAKE_CURRENT_SOURCE_DIR}/test_data/manifest_schema.json
         WORKING_DIRECTORY ${CppMicroServices_BINARY_DIR})
add_test(NAME NegativeTest_Schema_Validator_No_Schema_Arg
         COMMAND $<TARGET_FILE:jsonschemavalidator> -j ${CMAKE_CURRENT_SOURCE_DIR}/test_data/manifest.json
         WORKING_DIRECTORY ${CppMicroServices_BINARY_DIR})
add_test(NAME NegativeTest_Schema_Validator_Manifest_Without_SymbolicName
         COMMAND $<TARGET_FILE:jsonschemavalidator> -s ${CMAKE_CURRENT_SOURCE_DIR}/test_data/manifest_schema.json -j ${CMAKE_CURRENT_SOURCE_DIR}/test_data/manifest_no_symbolicname.json
         WORKING_DIRECTORY ${CppMicroServices_BINARY_DIR})
add_test(NAME NegativeTest_Schema_Validator_Manifest_With_Invalid_SymbolicName
         COMMAND $<TARGET_FILE:jsonschemavalidator> -s ${CMAKE_CURRENT_SOURCE_DIR}/test_data/manifest_schema.json -j ${CMAKE_CURRENT_SOURCE_DIR}/test_data/manifest_invalid_symbolicname.json
         WORKING_DIRECTORY ${CppMicroServices_BINARY_DIR})
add_test(NAME NegativeTest_Schema_Validator_Manifest_With_Invalid_Version
         COMMAND $<TARGET_FILE:jsonschemavalidator> -s ${CMAKE_CURRENT_SOURCE_DIR}/test_data/manifest_schema.json -j ${CMAKE_CURRENT_SOURCE_DIR}/test_data/manifest_invalid_version.json
         WORKING_DIRECTORY ${CppMicroServices_BINARY_DIR})

# Expect negative tests to fail 
set_tests_properties(NegativeTest_Schema_Validator_InvalidPaths 
                     NegativeTest_Schema_Validator_No_Data_Arg
                     NegativeTest_Schema_Validator_No_Schema_Arg
                     NegativeTest_Schema_Validator_Manifest_Without_SymbolicName 
                     NegativeTest_Schema_Validator_Manifest_With_Invalid_SymbolicName 
                     NegativeTest_Schema_Validator_Manifest_With_Invalid_Version 
                     PROPERTIES WILL_FAIL TRUE)
