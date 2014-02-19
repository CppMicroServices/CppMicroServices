project(Core)

set(${PROJECT_NAME}_MAJOR_VERSION 2)
set(${PROJECT_NAME}_MINOR_VERSION 99)
set(${PROJECT_NAME}_PATCH_VERSION 0)
set(${PROJECT_NAME}_VERSION ${${PROJECT_NAME}_MAJOR_VERSION}.${${PROJECT_NAME}_MINOR_VERSION}.${${PROJECT_NAME}_PATCH_VERSION})

set(${PROJECT_NAME}_TARGET CppMicroServices)
set(PROJECT_TARGET ${${PROJECT_NAME}_TARGET})

cmake_minimum_required(VERSION 2.8)
cmake_policy(VERSION 2.8)
cmake_policy(SET CMP0017 NEW)

#-----------------------------------------------------------------------------
# US include dirs and libraries
#-----------------------------------------------------------------------------

set(${PROJECT_NAME}_INCLUDE_DIRS
  ${CMAKE_CURRENT_SOURCE_DIR}/include
  ${CMAKE_CURRENT_BINARY_DIR}/include
)

set(${PROJECT_NAME}_INTERNAL_INCLUDE_DIRS
  ${CMAKE_CURRENT_SOURCE_DIR}/src/util
  ${CMAKE_CURRENT_SOURCE_DIR}/src/service
  ${CMAKE_CURRENT_SOURCE_DIR}/src/module
)

configure_file(${CMAKE_CURRENT_SOURCE_DIR}/include/us${PROJECT_NAME}Config.h.in
               ${CMAKE_CURRENT_BINARY_DIR}/include/us${PROJECT_NAME}Config.h
)

configure_file(${CppMicroServices_SOURCE_DIR}/cmake/usExport.h.in
               ${CMAKE_CURRENT_BINARY_DIR}/include/us${PROJECT_NAME}Export.h
)

include_directories(
  ${${PROJECT_NAME}_INCLUDE_DIRS}
  ${${PROJECT_NAME}_INTERNAL_INCLUDE_DIRS}
)

# link libraries for the CppMicroServices lib
set(${PROJECT_NAME}_LINK_LIBRARIES )
if(UNIX)
  list(APPEND ${PROJECT_NAME}_LINK_LIBRARIES dl)
endif()

#-----------------------------------------------------------------------------
# Create library
#-----------------------------------------------------------------------------

include(${CMAKE_CURRENT_SOURCE_DIR}/src/CMakeLists.txt)
include(${CMAKE_CURRENT_SOURCE_DIR}/include/CMakeLists.txt)

set(${PROJECT_NAME}_SOURCES )
foreach(_src ${_srcs})
  list(APPEND ${PROJECT_NAME}_SOURCES ${CMAKE_CURRENT_SOURCE_DIR}/src/${_src})
endforeach()

set(${PROJECT_NAME}_PRIVATE_HEADERS )
foreach(_header ${_private_headers})
  list(APPEND ${PROJECT_NAME}_PRIVATE_HEADERS ${CMAKE_CURRENT_SOURCE_DIR}/src/${_header})
endforeach()

set(${PROJECT_NAME}_PUBLIC_HEADERS )
foreach(_header ${_public_headers})
  list(APPEND ${PROJECT_NAME}_PUBLIC_HEADERS ${CMAKE_CURRENT_SOURCE_DIR}/include/${_header})
endforeach()
list(APPEND ${PROJECT_NAME}_PUBLIC_HEADERS
  ${CMAKE_CURRENT_BINARY_DIR}/include/us${PROJECT_NAME}Config.h
  ${CMAKE_CURRENT_BINARY_DIR}/include/us${PROJECT_NAME}Export.h
)

# Generate the module init file
usFunctionGenerateModuleInit(${PROJECT_NAME}_SOURCES NAME ${${PROJECT_NAME}_TARGET})

# Configure the modules manifest.json file
configure_file(${CMAKE_CURRENT_SOURCE_DIR}/resources/manifest.json.in
               ${CMAKE_CURRENT_BINARY_DIR}/resources/manifest.json)

# Embed module resources
usFunctionEmbedResources(${PROJECT_NAME}_SOURCES LIBRARY_NAME ${${PROJECT_NAME}_TARGET}
                         ROOT_DIR ${CMAKE_CURRENT_BINARY_DIR}/resources
                         FILES manifest.json)

# Create the module library
add_library(${${PROJECT_NAME}_TARGET} ${${PROJECT_NAME}_SOURCES}
            ${${PROJECT_NAME}_PRIVATE_HEADERS} ${${PROJECT_NAME}_PUBLIC_HEADERS} ${us_config_h_file})

# Link flags
if(${PROJECT_NAME}_LINK_FLAGS OR US_LINK_FLAGS)
  set_target_properties(${${PROJECT_NAME}_TARGET} PROPERTIES
    LINK_FLAGS "${US_LINK_FLAGS} ${${PROJECT_NAME}_LINK_FLAGS}"
  )
endif()

# Compile definitions
set_property(TARGET ${${PROJECT_NAME}_TARGET}
  APPEND PROPERTY COMPILE_DEFINITIONS US_FORCE_MODULE_INIT
)

set_target_properties(${${PROJECT_NAME}_TARGET} PROPERTIES
  SOVERSION ${${PROJECT_NAME}_VERSION}
  PUBLIC_HEADER "${${PROJECT_NAME}_PUBLIC_HEADERS}"
  PRIVATE_HEADER "${${PROJECT_NAME}_PRIVATE_HEADERS}"
)

# Link additional libraries
if(${PROJECT_NAME}_LINK_LIBRARIES)
  target_link_libraries(${${PROJECT_NAME}_TARGET} ${${PROJECT_NAME}_LINK_LIBRARIES})
endif()


#-----------------------------------------------------------------------------
# Install support
#-----------------------------------------------------------------------------

install(TARGETS ${${PROJECT_NAME}_TARGET}
        EXPORT us${PROJECT_NAME}Targets
        RUNTIME DESTINATION ${RUNTIME_INSTALL_DIR} ${US_SDK_INSTALL_COMPONENT}
        LIBRARY DESTINATION ${LIBRARY_INSTALL_DIR} ${US_SDK_INSTALL_COMPONENT}
        ARCHIVE DESTINATION ${ARCHIVE_INSTALL_DIR} ${US_SDK_INSTALL_COMPONENT}
        PUBLIC_HEADER DESTINATION ${HEADER_INSTALL_DIR} ${US_SDK_INSTALL_COMPONENT}
        PRIVATE_HEADER DESTINATION ${HEADER_INSTALL_DIR} ${US_SDK_INSTALL_COMPONENT})

#-----------------------------------------------------------------------------
# US testing
#-----------------------------------------------------------------------------

if(US_BUILD_TESTING)
  add_subdirectory(test)
endif()

#-----------------------------------------------------------------------------
# Documentation
#-----------------------------------------------------------------------------

add_subdirectory(doc)

#-----------------------------------------------------------------------------
# Last configuration and install steps
#-----------------------------------------------------------------------------

export(TARGETS ${${PROJECT_NAME}_TARGET}
       FILE ${CppMicroServices_BINARY_DIR}/us${PROJECT_NAME}Targets.cmake)
install(EXPORT us${PROJECT_NAME}Targets
        FILE us${PROJECT_NAME}Targets.cmake
        DESTINATION ${AUXILIARY_CMAKE_INSTALL_DIR})

# Configure CoreConfig.cmake for the build tree

set(PACKAGE_CONFIG_INCLUDE_DIR
  ${${PROJECT_NAME}_INCLUDE_DIRS}
  ${${PROJECT_NAME}_INTERNAL_INCLUDE_DIRS})
set(PACKAGE_CONFIG_RUNTIME_LIBRARY_DIR ${CMAKE_RUNTIME_OUTPUT_DIRECTORY})

configure_file(
  ${CMAKE_CURRENT_SOURCE_DIR}/us${PROJECT_NAME}Config.cmake.in
  ${CppMicroServices_BINARY_DIR}/us${PROJECT_NAME}Config.cmake
  @ONLY
  )

# Configure CoreConfig.cmake for the install tree

set(CONFIG_INCLUDE_DIR ${HEADER_INSTALL_DIR})
set(CONFIG_RUNTIME_LIBRARY_DIR ${RUNTIME_INSTALL_DIR})

configure_package_config_file(
  ${CMAKE_CURRENT_SOURCE_DIR}/us${PROJECT_NAME}Config.cmake.in
  ${CppMicroServices_BINARY_DIR}${CMAKE_FILES_DIRECTORY}/us${PROJECT_NAME}Config.cmake
  INSTALL_DESTINATION ${AUXILIARY_CMAKE_INSTALL_DIR}
  PATH_VARS CONFIG_INCLUDE_DIR CONFIG_RUNTIME_LIBRARY_DIR
  NO_SET_AND_CHECK_MACRO
  NO_CHECK_REQUIRED_COMPONENTS_MACRO
  )

# Version information
configure_file(
  ${CMAKE_CURRENT_SOURCE_DIR}/us${PROJECT_NAME}ConfigVersion.cmake.in
  ${CppMicroServices_BINARY_DIR}/us${PROJECT_NAME}ConfigVersion.cmake
  @ONLY
  )

install(FILES ${CppMicroServices_BINARY_DIR}${CMAKE_FILES_DIRECTORY}/us${PROJECT_NAME}Config.cmake
              ${CppMicroServices_BINARY_DIR}/us${PROJECT_NAME}ConfigVersion.cmake
        DESTINATION ${AUXILIARY_CMAKE_INSTALL_DIR}
        ${US_SDK_INSTALL_COMPONENT}
  )

#-----------------------------------------------------------------------------
# Build the examples
#-----------------------------------------------------------------------------

if(US_BUILD_EXAMPLES)
  if(NOT US_BUILD_SHARED_LIBS)
    message(WARNING "Core examples are not available if US_BUILD_SHARED_LIBS is OFF")
  else()
    set(CppMicroServices_DIR ${CppMicroServices_BINARY_DIR})
    add_subdirectory(examples)
  endif()
endif()
