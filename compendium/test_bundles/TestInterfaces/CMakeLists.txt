set(PROJECT_TARGET usTestInterfaces)

project(TestInterfaces VERSION 1.0.0 LANGUAGES CXX)

add_library(usTestInterfaces
  src/Interfaces.cpp
  )

configure_file(
  ${CppMicroServices_SOURCE_DIR}/cmake/Export.h.in
  ${CMAKE_CURRENT_BINARY_DIR}/include/TestInterfaces/${PROJECT_NAME}Export.h
  )

# Include directories
target_include_directories(usTestInterfaces
  PUBLIC
    $<BUILD_INTERFACE:${CMAKE_CURRENT_SOURCE_DIR}/include>
    $<BUILD_INTERFACE:${CMAKE_CURRENT_BINARY_DIR}/include>
    $<BUILD_INTERFACE:${CppMicroServices_BINARY_DIR}/include>
    $<INSTALL_INTERFACE:${HEADER_INSTALL_DIR}>
  )

