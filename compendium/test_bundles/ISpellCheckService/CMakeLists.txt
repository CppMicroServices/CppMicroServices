set(PROJECT_TARGET usISpellCheckService)

project(ISpellCheckService VERSION 1.0.0 LANGUAGES CXX)

add_library(usISpellCheckService
  src/ISpellCheckService.cpp
  )

configure_file(
  ${CppMicroServices_SOURCE_DIR}/cmake/Export.h.in
  ${CMAKE_CURRENT_BINARY_DIR}/include/ISpellCheckService/${PROJECT_NAME}Export.h
  )

include_directories(${CppMicroServices_SOURCE_DIR}/framework/include
  ${CppMicroServices_SOURCE_DIR}/include
  ${CppMicroServices_SOURCE_DIR}/compendium/test_bundles/ISpellCheckService/include
  ${CppMicroServices_BINARY_DIR}/framework/include
  ${CppMicroServices_BINARY_DIR}/compendium/test_bundles/ISpellCheckService/include
  )

# Include directories
target_include_directories(usISpellCheckService
  PUBLIC
    $<BUILD_INTERFACE:${CMAKE_CURRENT_SOURCE_DIR}/include>
    $<BUILD_INTERFACE:${CMAKE_CURRENT_BINARY_DIR}/include>
    $<BUILD_INTERFACE:${CppMicroServices_BINARY_DIR}/include>
    $<INSTALL_INTERFACE:${HEADER_INSTALL_DIR}>
  )
