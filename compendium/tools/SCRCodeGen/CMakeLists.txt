
set(_srcs 
    ComponentInfo.cpp
    Main.cpp
    ManifestParserImpl.cpp
    Util.cpp
      ../../../third_party/jsoncpp.cpp)
  
set(_private_headers
    ComponentCallbackGenerator.hpp
    ComponentInfo.hpp
    ManifestParser.hpp
    ManifestParserFactory.hpp
    ManifestParserImpl.hpp
    Util.hpp)
    
include_directories(../../../third_party
                    ${CppMicroServices_SOURCE_DIR}/third_party/googletest/googletest/include
                    ${CppMicroServices_SOURCE_DIR}/third_party/googletest/googlemock/include)

set(US_SCRCODEGEN_EXECUTABLE_TARGET SCRCodeGen)
set(US_SCRCODEGEN_EXECUTABLE_OUTPUT_NAME ${US_SCRCODEGEN_EXECUTABLE_TARGET}${US_GLOBAL_VERSION_SUFFIX})
    
add_executable(${US_SCRCODEGEN_EXECUTABLE_TARGET} ${_srcs} ${_private_headers})

set_property(TARGET ${US_SCRCODEGEN_EXECUTABLE_TARGET} PROPERTY OUTPUT_NAME ${US_SCRCODEGEN_EXECUTABLE_OUTPUT_NAME})

if(NOT US_NO_INSTALL)
    install(TARGETS ${US_SCRCODEGEN_EXECUTABLE_TARGET}
            EXPORT ${PROJECT_NAME}Targets
            FRAMEWORK DESTINATION . ${US_SDK_INSTALL_COMPONENT}
            RUNTIME DESTINATION ${TOOLS_INSTALL_DIR} ${US_SDK_INSTALL_COMPONENT})
endif()

if(CMAKE_CROSSCOMPILING)
  # When cross-compiling, import the host machine executable from a file
  # See https://gitlab.kitware.com/cmake/community/wikis/doc/cmake/CrossCompiling
  include(${IMPORT_EXECUTABLES})
else()
  # Export the host scr code gen tool target to a file so it can be imported when cross-compiling.
  # Use the NAMESPACE option of EXPORT() to get a different target name for ${US_SCRCODEGEN_EXECUTABLE_TARGET}
  # when exporting.
  # Doing this allows the host built SCRCodeGen to be used when cross-compiling and allows
  # the scr code gen tool to be built and installed for the target machine.
  export( TARGETS ${US_SCRCODEGEN_EXECUTABLE_TARGET} APPEND FILE ${IMPORT_EXECUTABLES} NAMESPACE native-)
endif()

if(US_BUILD_TESTING AND EXISTS "${CMAKE_CURRENT_SOURCE_DIR}/test/CMakeLists.txt")
  add_subdirectory(test)
endif()

export(TARGETS ${US_SCRCODEGEN_EXECUTABLE_TARGET}
       FILE ${CMAKE_CURRENT_BINARY_DIR}/${PROJECT_NAME}Targets.cmake)
