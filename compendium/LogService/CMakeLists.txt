set(_public_headers
  include/cppmicroservices/logservice/LogService.hpp
  include/cppmicroservices/logservice/LoggerFactory.hpp
  include/cppmicroservices/logservice/Logger.hpp
)

set(_version "1.0.0")
project(usLogService VERSION ${_version})

if(NOT ${PROJECT_NAME}_TARGET)
  set(${PROJECT_NAME}_TARGET usLogService)
endif()
set(PROJECT_TARGET ${${PROJECT_NAME}_TARGET})

configure_file(${CppMicroServices_SOURCE_DIR}/cmake/Export.h.in
  ${CMAKE_CURRENT_BINARY_DIR}/include/cppmicroservices/logservice/LogServiceExport.h
  )

add_library(usLogService INTERFACE)
target_include_directories(usLogService INTERFACE ${CMAKE_CURRENT_SOURCE_DIR}/include/)

# install support
install(DIRECTORY ${CMAKE_CURRENT_SOURCE_DIR}/include/cppmicroservices
	DESTINATION ${HEADER_INSTALL_DIR}
	OPTIONAL)

install(DIRECTORY ${CMAKE_CURRENT_BINARY_DIR}/include/cppmicroservices
	DESTINATION ${HEADER_INSTALL_DIR}
	OPTIONAL)

# Version information
configure_file(
  ${US_CMAKE_DIR}/usBundleConfigVersion.cmake.in
  ${CppMicroServices_BINARY_DIR}/us${PROJECT_NAME}ConfigVersion.cmake
  @ONLY
  )

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
