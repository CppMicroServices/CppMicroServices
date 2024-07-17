# sources and headers
set(_srcs
src/EMConstants.cpp
src/Event.cpp
)

set(_public_headers
include/cppmicroservices/em/EMConstants.hpp
include/cppmicroservices/em/Event.hpp
include/cppmicroservices/em/EventAdmin.hpp
include/cppmicroservices/em/EventHandler.hpp
)

set(_version "1.0.0")
project(em VERSION ${_version})

if(NOT ${PROJECT_NAME}_TARGET)
  set(${PROJECT_NAME}_TARGET usEM)
endif()
set(PROJECT_TARGET ${${PROJECT_NAME}_TARGET})

# link libraries for the EM lib
set(_link_libraries )
if(UNIX)
  list(APPEND _link_libraries dl)
endif()

if(CMAKE_THREAD_LIBS_INIT)
  list(APPEND _link_libraries ${CMAKE_THREAD_LIBS_INIT})
endif()

configure_file(${CppMicroServices_SOURCE_DIR}/cmake/Export.h.in
  ${CMAKE_CURRENT_BINARY_DIR}/include/cppmicroservices/em/EMExport.h
  )

add_library(${PROJECT_TARGET} ${_srcs} ${_public_headers} ${CMAKE_CURRENT_BINARY_DIR}/include/cppmicroservices/em/EMExport.h)
if (US_BUILD_TESTING)
  target_compile_definitions(${PROJECT_TARGET} PRIVATE USING_GTEST)
endif()

target_include_directories(usEM
  PUBLIC
    $<BUILD_INTERFACE:${CMAKE_CURRENT_SOURCE_DIR}/include>
    $<BUILD_INTERFACE:${CMAKE_CURRENT_BINARY_DIR}/include>
    $<BUILD_INTERFACE:${CppMicroServices_BINARY_DIR}/include>
    $<INSTALL_INTERFACE:${HEADER_INSTALL_DIR}>
  )

target_link_libraries(${PROJECT_TARGET} PUBLIC CppMicroServices)

# install support
install(TARGETS ${${PROJECT_NAME}_TARGET}
	  EXPORT us${PROJECT_NAME}Targets
	  RUNTIME DESTINATION ${RUNTIME_INSTALL_DIR} ${US_SDK_INSTALL_COMPONENT}
	  LIBRARY DESTINATION ${LIBRARY_INSTALL_DIR} ${US_SDK_INSTALL_COMPONENT}
	  ARCHIVE DESTINATION ${ARCHIVE_INSTALL_DIR} ${US_SDK_INSTALL_COMPONENT})

install(DIRECTORY ${CMAKE_CURRENT_SOURCE_DIR}/include/cppmicroservices
	DESTINATION ${HEADER_INSTALL_DIR}
	OPTIONAL)

install(DIRECTORY ${CMAKE_CURRENT_BINARY_DIR}/include/cppmicroservices
	DESTINATION ${HEADER_INSTALL_DIR}
	OPTIONAL)

if(US_BUILD_TESTING AND EXISTS "${CMAKE_CURRENT_SOURCE_DIR}/test/CMakeLists.txt")
  add_subdirectory(test)
endif()

# Version information
configure_file(
  ${US_CMAKE_DIR}/usBundleConfigVersion.cmake.in
  ${CppMicroServices_BINARY_DIR}/us${PROJECT_NAME}ConfigVersion.cmake
  @ONLY
  )

export(TARGETS ${${PROJECT_NAME}_TARGET} CppMicroServices
       FILE ${CppMicroServices_BINARY_DIR}/us${PROJECT_NAME}Targets.cmake)

if(NOT US_NO_INSTALL)
  install(EXPORT us${PROJECT_NAME}Targets
	  FILE us${PROJECT_NAME}Targets.cmake
	  DESTINATION ${AUXILIARY_CMAKE_INSTALL_DIR})
endif()

# Configure config file for the build tree

set(PACKAGE_CONFIG_RUNTIME_LIBRARY_DIR ${CMAKE_RUNTIME_OUTPUT_DIRECTORY})

configure_file(
  ${US_CMAKE_DIR}/usBundleConfig.cmake.in
  ${CppMicroServices_BINARY_DIR}/us${PROJECT_NAME}Config.cmake
  @ONLY
  )

# Configure config file for the install tree
if(NOT US_NO_INSTALL)
  set(CONFIG_INCLUDE_DIR ${HEADER_INSTALL_DIR})
  set(CONFIG_RUNTIME_LIBRARY_DIR ${RUNTIME_INSTALL_DIR})

  configure_package_config_file(
    ${US_CMAKE_DIR}/usBundleConfig.cmake.in
    ${CppMicroServices_BINARY_DIR}${CMAKE_FILES_DIRECTORY}/us${PROJECT_NAME}Config.cmake
    INSTALL_DESTINATION ${AUXILIARY_CMAKE_INSTALL_DIR}
    PATH_VARS CONFIG_INCLUDE_DIR CONFIG_RUNTIME_LIBRARY_DIR
    NO_SET_AND_CHECK_MACRO
    NO_CHECK_REQUIRED_COMPONENTS_MACRO
    )

  install(FILES ${CppMicroServices_BINARY_DIR}${CMAKE_FILES_DIRECTORY}/us${PROJECT_NAME}Config.cmake
		${CppMicroServices_BINARY_DIR}/us${PROJECT_NAME}ConfigVersion.cmake
	  DESTINATION ${AUXILIARY_CMAKE_INSTALL_DIR}
	  ${US_SDK_INSTALL_COMPONENT})
endif()
