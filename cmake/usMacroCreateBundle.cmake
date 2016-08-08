# For internal use only

macro(usMacroCreateBundle _project_name)

project(${_project_name})

cmake_parse_arguments(${PROJECT_NAME}
  "SKIP_EXAMPLES;SKIP_INIT"
  "VERSION;TARGET;SYMBOLIC_NAME"
  "DEPENDS;INTERNAL_INCLUDE_DIRS;LINK_LIBRARIES;INTERNAL_LINK_LIBRARIES;SOURCES;PRIVATE_HEADERS;PUBLIC_HEADERS;RESOURCES;BINARY_RESOURCES"
  ${ARGN}
)

if(NOT ${PROJECT_NAME}_VERSION MATCHES "^[0-9]+\\.[0-9]+\\.[0-9]+$")
  message(SEND_ERROR "VERSION argument invalid: ${${PROJECT_NAME}_VERSION}")
endif()

string(REPLACE "." ";" _version_numbers ${${PROJECT_NAME}_VERSION})
list(GET _version_numbers 0 ${PROJECT_NAME}_MAJOR_VERSION)
list(GET _version_numbers 1 ${PROJECT_NAME}_MINOR_VERSION)
list(GET _version_numbers 2 ${PROJECT_NAME}_PATCH_VERSION)

if(NOT ${PROJECT_NAME}_TARGET)
  set(${PROJECT_NAME}_TARGET us${PROJECT_NAME})
endif()
set(PROJECT_TARGET ${${PROJECT_NAME}_TARGET})

if(NOT ${PROJECT_NAME}_SYMBOLIC_NAME)
  set(${PROJECT_NAME}_SYMBOLIC_NAME ${${PROJECT_NAME}_TARGET})
endif()

if(${PROJECT_NAME}_DEPENDS)
  find_package(CppMicroServices REQUIRED ${${PROJECT_NAME}_DEPENDS} QUIET
    HINTS ${CppMicroServices_BINARY_DIR}
    NO_DEFAULT_PATH
  )
endif()

#-----------------------------------------------------------------------------
# Include dirs and libraries
#-----------------------------------------------------------------------------

set(${PROJECT_NAME}_INCLUDE_DIRS
  ${CMAKE_CURRENT_SOURCE_DIR}/include
  ${CMAKE_CURRENT_BINARY_DIR}/include
)

set(${PROJECT_NAME}_INCLUDE_SUBDIR "cppmicroservices")
if(NOT ${PROJECT_NAME} STREQUAL "Framework")
  string(TOLOWER ${PROJECT_NAME} PROJECT_NAME_LOWER)
  set(${PROJECT_NAME}_INCLUDE_SUBDIR "${${PROJECT_NAME}_INCLUDE_SUBDIR}/${PROJECT_NAME_LOWER}")
endif()

configure_file(${CppMicroServices_SOURCE_DIR}/cmake/Export.h.in
               ${CMAKE_CURRENT_BINARY_DIR}/include/${${PROJECT_NAME}_INCLUDE_SUBDIR}/${PROJECT_NAME}Export.h)
list(APPEND ${PROJECT_NAME}_PUBLIC_HEADERS
  ${CMAKE_CURRENT_BINARY_DIR}/include/${${PROJECT_NAME}_INCLUDE_SUBDIR}/${PROJECT_NAME}Export.h)

if(EXISTS ${CMAKE_CURRENT_SOURCE_DIR}/include/${PROJECT_NAME}Config.h.in)
  configure_file(${CMAKE_CURRENT_SOURCE_DIR}/include/${PROJECT_NAME}Config.h.in
                 ${CMAKE_CURRENT_BINARY_DIR}/include/${${PROJECT_NAME}_INCLUDE_SUBDIR}/${PROJECT_NAME}Config.h)
  list(APPEND ${PROJECT_NAME}_PUBLIC_HEADERS
    ${CMAKE_CURRENT_BINARY_DIR}/include/${${PROJECT_NAME}_INCLUDE_SUBDIR}/${PROJECT_NAME}Config.h)
endif()

include_directories(
  ${US_INCLUDE_DIRS}
  ${${PROJECT_NAME}_INCLUDE_DIRS}
)

set(_internal_include_dirs ${${PROJECT_NAME}_INTERNAL_INCLUDE_DIRS})
set(${PROJECT_NAME}_INTERNAL_INCLUDE_DIRS )
if(_internal_include_dirs)
  foreach(_internal_include_dir ${_internal_include_dirs})
    if(IS_ABSOLUTE "${_internal_include_dir}")
      list(APPEND ${PROJECT_NAME}_INTERNAL_INCLUDE_DIRS ${_internal_include_dir})
    else()
      list(APPEND ${PROJECT_NAME}_INTERNAL_INCLUDE_DIRS ${CMAKE_CURRENT_SOURCE_DIR}/${_internal_include_dir})
    endif()
  endforeach()
  include_directories(${${PROJECT_NAME}_INTERNAL_INCLUDE_DIRS})
endif()

#-----------------------------------------------------------------------------
# Create library
#-----------------------------------------------------------------------------

# Generate the bundle init file
if(NOT ${PROJECT_NAME}_SKIP_INIT)
  usFunctionGenerateBundleInit(${PROJECT_NAME}_SOURCES)
endif()

if(${PROJECT_NAME}_RESOURCES OR ${PROJECT_NAME}_BINARY_RESOURCES)
  usFunctionGetResourceSource(TARGET ${${PROJECT_NAME}_TARGET} OUT ${PROJECT_NAME}_SOURCES)
endif()

# Create the bundle library
add_library(${${PROJECT_NAME}_TARGET} ${${PROJECT_NAME}_SOURCES}
            ${${PROJECT_NAME}_PRIVATE_HEADERS} ${${PROJECT_NAME}_PUBLIC_HEADERS})

# Compile definitions
set_property(TARGET ${${PROJECT_NAME}_TARGET} APPEND PROPERTY COMPILE_DEFINITIONS US_BUNDLE_NAME=${${PROJECT_NAME}_SYMBOLIC_NAME})
set_property(TARGET ${${PROJECT_NAME}_TARGET} PROPERTY US_BUNDLE_NAME ${${PROJECT_NAME}_SYMBOLIC_NAME})

# Link flags
if(${PROJECT_NAME}_LINK_FLAGS OR US_LINK_FLAGS)
  set_target_properties(${${PROJECT_NAME}_TARGET} PROPERTIES
    LINK_FLAGS "${US_LINK_FLAGS} ${${PROJECT_NAME}_LINK_FLAGS}"
  )
endif()

set_target_properties(${${PROJECT_NAME}_TARGET} PROPERTIES
  SOVERSION ${${PROJECT_NAME}_VERSION}
)

# Link additional libraries
if(${PROJECT_NAME}_LINK_LIBRARIES OR ${PROJECT_NAME}_INTERNAL_LINK_LIBRARIES OR US_LIBRARIES)
  target_link_libraries(${${PROJECT_NAME}_TARGET} ${US_LIBRARIES} ${${PROJECT_NAME}_LINK_LIBRARIES}
                        ${${PROJECT_NAME}_INTERNAL_LINK_LIBRARIES})
endif()

if(${PROJECT_NAME}_INTERNAL_LINK_LIBRARIES)
  set_target_properties(${${PROJECT_NAME}_TARGET} PROPERTIES
    INTERFACE_LINK_LIBRARIES ${US_LIBRARIES} ${${PROJECT_NAME}_LINK_LIBRARIES}
  )
endif()

# Embed bundle resources

if(${PROJECT_NAME}_RESOURCES)
  set(_wd ${CMAKE_CURRENT_SOURCE_DIR})
  if(${PROJECT_NAME}_RESOURCES)
    set(_wd ${CMAKE_CURRENT_SOURCE_DIR}/resources)
  endif()
  usFunctionAddResources(TARGET ${${PROJECT_NAME}_TARGET}
                         WORKING_DIRECTORY ${_wd}
                         FILES ${${PROJECT_NAME}_RESOURCES}
                        )
endif()
if(${PROJECT_NAME}_BINARY_RESOURCES)
  usFunctionAddResources(TARGET ${${PROJECT_NAME}_TARGET}
                         WORKING_DIRECTORY ${CMAKE_CURRENT_BINARY_DIR}/resources
                         FILES ${${PROJECT_NAME}_BINARY_RESOURCES}
                        )
endif()
usFunctionEmbedResources(TARGET ${${PROJECT_NAME}_TARGET})

#-----------------------------------------------------------------------------
# Install support
#-----------------------------------------------------------------------------

if(NOT US_NO_INSTALL)
  install(TARGETS ${${PROJECT_NAME}_TARGET}
          EXPORT us${PROJECT_NAME}Targets
          RUNTIME DESTINATION ${RUNTIME_INSTALL_DIR} ${US_SDK_INSTALL_COMPONENT}
          LIBRARY DESTINATION ${LIBRARY_INSTALL_DIR} ${US_SDK_INSTALL_COMPONENT}
          ARCHIVE DESTINATION ${ARCHIVE_INSTALL_DIR} ${US_SDK_INSTALL_COMPONENT})

  install(DIRECTORY ${CMAKE_CURRENT_SOURCE_DIR}/include/cppmicroservices DESTINATION ${HEADER_INSTALL_DIR})
  install(DIRECTORY ${CMAKE_CURRENT_BINARY_DIR}/include/cppmicroservices DESTINATION ${HEADER_INSTALL_DIR})
endif()

#-----------------------------------------------------------------------------
# US testing
#-----------------------------------------------------------------------------

if(US_BUILD_TESTING AND EXISTS "${CMAKE_CURRENT_SOURCE_DIR}/test/CMakeLists.txt")
  add_subdirectory(test)
endif()

#-----------------------------------------------------------------------------
# Documentation
#-----------------------------------------------------------------------------

if(US_BUILD_TESTING AND EXISTS "${CMAKE_CURRENT_SOURCE_DIR}/doc/snippets/CMakeLists.txt")
  # Compile source code snippets
  add_subdirectory(${CMAKE_CURRENT_SOURCE_DIR}/doc/snippets)
endif()

#-----------------------------------------------------------------------------
# Last configuration and install steps
#-----------------------------------------------------------------------------

# Version information
configure_file(
  ${US_CMAKE_DIR}/usBundleConfigVersion.cmake.in
  ${CppMicroServices_BINARY_DIR}/us${PROJECT_NAME}ConfigVersion.cmake
  @ONLY
  )

export(TARGETS ${${PROJECT_NAME}_TARGET} ${US_LIBRARIES}
       FILE ${CppMicroServices_BINARY_DIR}/us${PROJECT_NAME}Targets.cmake)
if(NOT US_NO_INSTALL)
  install(EXPORT us${PROJECT_NAME}Targets
          FILE us${PROJECT_NAME}Targets.cmake
          DESTINATION ${AUXILIARY_CMAKE_INSTALL_DIR})
endif()

# Configure config file for the build tree

set(PACKAGE_CONFIG_INCLUDE_DIR
  ${${PROJECT_NAME}_INCLUDE_DIRS}
  ${${PROJECT_NAME}_INTERNAL_INCLUDE_DIRS})
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

#-----------------------------------------------------------------------------
# Build the examples
#-----------------------------------------------------------------------------

if(US_BUILD_EXAMPLES AND NOT ${PROJECT_NAME}_SKIP_EXAMPLES AND
   EXISTS ${CMAKE_CURRENT_SOURCE_DIR}/examples/CMakeLists.txt)
  set(CppMicroServices_DIR ${CppMicroServices_BINARY_DIR})
  add_subdirectory(examples)
endif()

endmacro()
