project(CppMicroServices)

set(${PROJECT_NAME}_MAJOR_VERSION 1)
set(${PROJECT_NAME}_MINOR_VERSION 99)
set(${PROJECT_NAME}_PATCH_VERSION 0)
set(${PROJECT_NAME}_VERSION ${${PROJECT_NAME}_MAJOR_VERSION}.${${PROJECT_NAME}_MINOR_VERSION}.${${PROJECT_NAME}_PATCH_VERSION})

cmake_minimum_required(VERSION 2.8)
cmake_policy(VERSION 2.8)
cmake_policy(SET CMP0017 NEW)

#-----------------------------------------------------------------------------
# Update CMake module path
#------------------------------------------------------------------------------

set(CMAKE_MODULE_PATH
  ${PROJECT_SOURCE_DIR}/CMake
  ${CMAKE_MODULE_PATH}
  )

#-----------------------------------------------------------------------------
# CMake function(s) and macro(s)
#-----------------------------------------------------------------------------

include(CMakeParseArguments)
include(CMakePackageConfigHelpers)
include(CheckCXXSourceCompiles)
include(usFunctionCheckCompilerFlags)
include(usFunctionEmbedResources)
include(usFunctionGetGccVersion)
include(usFunctionGenerateModuleInit)
include(usFunctionGenerateExecutableInit)

#-----------------------------------------------------------------------------
# Init output directories
#-----------------------------------------------------------------------------

set(US_ARCHIVE_OUTPUT_DIRECTORY "${PROJECT_BINARY_DIR}/lib")
set(US_LIBRARY_OUTPUT_DIRECTORY "${PROJECT_BINARY_DIR}/lib")
set(US_RUNTIME_OUTPUT_DIRECTORY "${PROJECT_BINARY_DIR}/bin")

foreach(_type ARCHIVE LIBRARY RUNTIME)
  if(NOT CMAKE_${_type}_OUTPUT_DIRECTORY)
    set(CMAKE_${_type}_OUTPUT_DIRECTORY ${US_${_type}_OUTPUT_DIRECTORY})
  endif()
endforeach()

#-----------------------------------------------------------------------------
# Set a default build type if none was specified
#-----------------------------------------------------------------------------

if(NOT CMAKE_BUILD_TYPE AND NOT CMAKE_CONFIGURATION_TYPES)
  message(STATUS "Setting build type to 'Debug' as none was specified.")
  set(CMAKE_BUILD_TYPE Debug CACHE STRING "Choose the type of build." FORCE)

  # Set the possible values of build type for cmake-gui
  set_property(CACHE CMAKE_BUILD_TYPE PROPERTY
               STRINGS "Debug" "Release" "MinSizeRel" "RelWithDebInfo")
endif()

#-----------------------------------------------------------------------------
# CMake options
#-----------------------------------------------------------------------------

function(us_cache_var _var_name _var_default _var_type _var_help)
  set(_advanced 0)
  set(_force)
  foreach(_argn ${ARGN})
    if(_argn STREQUAL ADVANCED)
      set(_advanced 1)
    elseif(_argn STREQUAL FORCE)
      set(_force FORCE)
    endif()
  endforeach()

  if(US_IS_EMBEDDED)
    if(NOT DEFINED ${_var_name} OR _force)
      set(${_var_name} ${_var_default} PARENT_SCOPE)
    endif()
  else()
    set(${_var_name} ${_var_default} CACHE ${_var_type} "${_var_help}" ${_force})
    if(_advanced)
      mark_as_advanced(${_var_name})
    endif()
  endif()
endfunction()

# Determine if we are being build inside a larger project
if(NOT DEFINED US_IS_EMBEDDED)
  if(CMAKE_PROJECT_NAME STREQUAL PROJECT_NAME)
    set(US_IS_EMBEDDED 0)
  else()
    set(US_IS_EMBEDDED 1)
    set(CppMicroServices_EXPORTS 1)
  endif()
endif()

# Determine the name of the install component for "SDK" artifacts.
# The default is "sdk"
if(NOT DEFINED US_SDK_INSTALL_COMPONENT)
  set(US_SDK_INSTALL_COMPONENT COMPONENT sdk)
elseif(US_SDK_INSTALL_COMPONENT)
  set(US_SDK_INSTALL_COMPONENT COMPONENT ${US_SDK_INSTALL_COMPONENT})
endif()

us_cache_var(US_ENABLE_AUTOLOADING_SUPPORT OFF BOOL "Enable module auto-loading support")
us_cache_var(US_ENABLE_THREADING_SUPPORT OFF BOOL "Enable threading support")
us_cache_var(US_ENABLE_DEBUG_OUTPUT OFF BOOL "Enable debug messages" ADVANCED)
us_cache_var(US_ENABLE_RESOURCE_COMPRESSION ON BOOL "Enable resource compression" ADVANCED)
us_cache_var(US_BUILD_SHARED_LIBS ON BOOL "Build shared libraries")
us_cache_var(US_BUILD_TESTING OFF BOOL "Build tests")

if(WIN32 AND NOT CYGWIN)
  set(default_runtime_install_dir bin/)
  set(default_library_install_dir bin/)
  set(default_archive_install_dir lib/)
  set(default_header_install_dir include/)
  set(default_auxiliary_install_dir share/)
else()
  set(default_runtime_install_dir bin/)
  set(default_library_install_dir lib/${PROJECT_NAME})
  set(default_archive_install_dir lib/${PROJECT_NAME})
  set(default_header_install_dir include/${PROJECT_NAME})
  set(default_auxiliary_install_dir share/${PROJECT_NAME})
endif()

us_cache_var(RUNTIME_INSTALL_DIR ${default_runtime_install_dir} STRING "Relative install location for binaries" ADVANCED)
us_cache_var(LIBRARY_INSTALL_DIR ${default_library_install_dir} STRING "Relative install location for libraries" ADVANCED)
us_cache_var(ARCHIVE_INSTALL_DIR ${default_archive_install_dir} STRING "Relative install location for archives" ADVANCED)
us_cache_var(HEADER_INSTALL_DIR ${default_header_install_dir} STRING "Relative install location for headers" ADVANCED)
us_cache_var(AUXILIARY_INSTALL_DIR ${default_auxiliary_install_dir} STRING "Relative install location for auxiliary files" ADVANCED)

if(MSVC10 OR MSVC11 OR MSVC12)
  # Visual Studio 2010 and newer have support for C++11 enabled by default
  set(US_USE_C++11 1)
else()
  us_cache_var(US_USE_C++11 OFF BOOL "Enable the use of C++11 features" ADVANCED)
endif()

us_cache_var(US_NAMESPACE "us" STRING "The namespace for the C++ micro services entities")
us_cache_var(US_HEADER_PREFIX "" STRING "The file name prefix for the public C++ micro services header files")

set(BUILD_SHARED_LIBS ${US_BUILD_SHARED_LIBS})

set(${PROJECT_NAME}_MODULE_INIT_TEMPLATE "${CMAKE_CURRENT_SOURCE_DIR}/CMake/usModuleInit.cpp")
set(${PROJECT_NAME}_EXECUTABLE_INIT_TEMPLATE "${CMAKE_CURRENT_SOURCE_DIR}/CMake/usExecutableInit.cpp")

#-----------------------------------------------------------------------------
# US C/CXX Flags
#-----------------------------------------------------------------------------

set(US_C_FLAGS)
set(US_C_FLAGS_RELEASE)
set(US_CXX_FLAGS)
set(US_CXX_FLAGS_RELEASE)

# This is used as a preprocessor define
set(US_USE_CXX11 ${US_USE_C++11})

# Set C++ compiler flags
if(NOT MSVC)
  foreach(_cxxflag  -Werror -Wall -Wextra -Wpointer-arith -Winvalid-pch -Wcast-align
                    -Wwrite-strings -Woverloaded-virtual -Wnon-virtual-dtor -Wold-style-cast
                    -Wstrict-null-sentinel -Wsign-promo -fdiagnostics-show-option)
    usFunctionCheckCompilerFlags(${_cxxflag} US_CXX_FLAGS)
  endforeach()

  if(US_USE_C++11)
    usFunctionCheckCompilerFlags("-std=c++0x" US_CXX_FLAGS)
  endif()
endif()

if(CMAKE_COMPILER_IS_GNUCXX)

  usFunctionGetGccVersion(${CMAKE_CXX_COMPILER} GCC_VERSION)
  if(${GCC_VERSION} VERSION_LESS "4.0.0")
    message(FATAL_ERROR "gcc version ${GCC_VERSION} not supported. Please use gcc >= 4.")
  endif()

  # With older versions of gcc the flag -fstack-protector-all requires an extra dependency to libssp.so.
  # If the gcc version is lower than 4.4.0 and the build type is Release let's not include the flag.
  if(${GCC_VERSION} VERSION_GREATER "4.4.0" OR (CMAKE_BUILD_TYPE STREQUAL "Debug" AND ${GCC_VERSION} VERSION_LESS "4.4.0"))
    usFunctionCheckCompilerFlags("-fstack-protector-all" US_CXX_FLAGS)
  endif()

  if(MINGW)
    # suppress warnings about auto imported symbols
    set(US_CXX_FLAGS "-Wl,--enable-auto-import ${US_CXX_FLAGS}")
    # we need to define a Windows version
    set(US_CXX_FLAGS "-D_WIN32_WINNT=0x0500 ${US_CXX_FLAGS}")
  else()
    # Enable visibility support
    if(NOT ${GCC_VERSION} VERSION_LESS "4.5")
      usFunctionCheckCompilerFlags("-fvisibility=hidden -fvisibility-inlines-hidden" US_CXX_FLAGS)
    else()
      set(US_GCC_RTTI_WORKAROUND_NEEDED 1)
    endif()
  endif()

  usFunctionCheckCompilerFlags("-O1 -D_FORTIFY_SOURCE=2" _fortify_source_flag)
  if(_fortify_source_flag)
    set(US_CXX_FLAGS_RELEASE "${US_CXX_FLAGS_RELEASE} -D_FORTIFY_SOURCE=2")
  endif()


elseif(MSVC)
  set(US_CXX_FLAGS "/MP /wd4996 ${US_CXX_FLAGS}")
endif()

if(NOT US_IS_EMBEDDED)
  set(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} ${US_CXX_FLAGS}")
  set(CMAKE_CXX_FLAGS_RELEASE "${CMAKE_CXX_FLAGS_RELEASE} ${US_CXX_FLAGS_RELEASE}")
  set(CMAKE_C_FLAGS "${CMAKE_C_FLAGS} ${US_C_FLAGS}")
  set(CMAKE_C_FLAGS_REALEASE "${CMAKE_C_FLAGS_RELEASE} ${US_C_FLAGS_RELEASE}")
endif()

#-----------------------------------------------------------------------------
# US Link Flags
#-----------------------------------------------------------------------------

set(US_LINK_FLAGS )
if(CMAKE_COMPILER_IS_GNUCXX AND NOT APPLE)
  foreach(_linkflag -Wl,--no-undefined)
    set(_add_flag)
    usFunctionCheckCompilerFlags("${_linkflag}" _add_flag)
    if(_add_flag)
      set(US_LINK_FLAGS "${US_LINK_FLAGS} ${_linkflag}")
    endif()
  endforeach()
endif()

#-----------------------------------------------------------------------------
# US Header Checks
#-----------------------------------------------------------------------------

include(CheckIncludeFile)

CHECK_INCLUDE_FILE(stdint.h HAVE_STDINT)

#-----------------------------------------------------------------------------
# US include dirs and libraries
#-----------------------------------------------------------------------------

set(US_INCLUDE_DIRS
  ${PROJECT_BINARY_DIR}/include
)

set(US_INTERNAL_INCLUDE_DIRS
  ${PROJECT_BINARY_DIR}/include
  ${CMAKE_CURRENT_SOURCE_DIR}/src/util
  ${CMAKE_CURRENT_SOURCE_DIR}/src/service
  ${CMAKE_CURRENT_SOURCE_DIR}/src/module
)

# link library for third party libs
if(US_IS_EMBEDDED)
  set(US_LIBRARY_TARGET ${US_EMBEDDING_LIBRARY})
else()
  set(US_LIBRARY_TARGET ${PROJECT_NAME})
endif()

# link libraries for the CppMicroServices lib
set(US_LINK_LIBRARIES )
if(UNIX)
  list(APPEND US_LINK_LIBRARIES dl)
endif()

#-----------------------------------------------------------------------------
# Source directory
#-----------------------------------------------------------------------------

set(us_config_h_file "${PROJECT_BINARY_DIR}/include/usConfig.h")
configure_file(usConfig.h.in ${us_config_h_file})

set(US_RCC_EXECUTABLE_NAME usResourceCompiler)
set(CppMicroServices_RCC_EXECUTABLE_NAME ${US_RCC_EXECUTABLE_NAME})

add_subdirectory(tools)

add_subdirectory(src)


#-----------------------------------------------------------------------------
# US testing
#-----------------------------------------------------------------------------

if(US_BUILD_TESTING)
  enable_testing()
  add_subdirectory(test)
endif()

#-----------------------------------------------------------------------------
# Documentation
#-----------------------------------------------------------------------------

add_subdirectory(documentation)

#-----------------------------------------------------------------------------
# Last configuration and install steps
#-----------------------------------------------------------------------------

if(NOT US_IS_EMBEDDED)
  export(TARGETS ${PROJECT_NAME} ${US_RCC_EXECUTABLE_NAME}
         FILE ${CMAKE_CURRENT_BINARY_DIR}/${PROJECT_NAME}Targets.cmake)
  install(EXPORT ${PROJECT_NAME}Targets
          FILE ${PROJECT_NAME}Targets.cmake
          DESTINATION ${AUXILIARY_INSTALL_DIR}/CMake/)
endif()

set(_install_cmake_scripts
  ${${PROJECT_NAME}_MODULE_INIT_TEMPLATE}
  ${${PROJECT_NAME}_EXECUTABLE_INIT_TEMPLATE}
  ${CMAKE_CURRENT_SOURCE_DIR}/CMake/CMakeParseArguments.cmake
  ${CMAKE_CURRENT_SOURCE_DIR}/CMake/usFunctionGenerateModuleInit.cmake
  ${CMAKE_CURRENT_SOURCE_DIR}/CMake/usFunctionGenerateExecutableInit.cmake
  ${CMAKE_CURRENT_SOURCE_DIR}/CMake/usFunctionEmbedResources.cmake
  )

install(FILES ${_install_cmake_scripts}
        DESTINATION ${AUXILIARY_INSTALL_DIR}/CMake/
  )

# Configure CppMicroServicesConfig.cmake for the build tree

set(PACKAGE_CONFIG_INCLUDE_DIR ${US_INCLUDE_DIRS})
set(PACKAGE_CONFIG_RUNTIME_DIR ${CMAKE_RUNTIME_OUTPUT_DIRECTORY})
set(PACKAGE_CONFIG_CMAKE_DIR ${PROJECT_SOURCE_DIR}/CMake)
if(US_IS_EMBEDDED)
  set(PACKAGE_EMBEDDED "if(@PROJECT_NAME@_IS_EMBEDDED)
  set(@PROJECT_NAME@_INTERNAL_INCLUDE_DIRS @US_INTERNAL_INCLUDE_DIRS@)
  set(@PROJECT_NAME@_SOURCES @US_SOURCES@)
  set(@PROJECT_NAME@_PUBLIC_HEADERS @US_PUBLIC_HEADERS@)
  set(@PROJECT_NAME@_PRIVATE_HEADERS @US_PRIVATE_HEADERS@)
endif()")
else()
  set(PACKAGE_EMBEDDED )
endif()

configure_file(
  ${CMAKE_CURRENT_SOURCE_DIR}/${PROJECT_NAME}Config.cmake.in
  ${PROJECT_BINARY_DIR}/${PROJECT_NAME}Config.cmake
  @ONLY
  )

# Configure CppMicroServicesConfig.cmake for the install tree
set(CONFIG_INCLUDE_DIR ${HEADER_INSTALL_DIR})
set(CONFIG_RUNTIME_DIR ${RUNTIME_INSTALL_DIR})
set(CONFIG_CMAKE_DIR ${AUXILIARY_INSTALL_DIR}/CMake)
configure_package_config_file(
  ${CMAKE_CURRENT_SOURCE_DIR}/${PROJECT_NAME}Config.cmake.in
  ${PROJECT_BINARY_DIR}${CMAKE_FILES_DIRECTORY}/${PROJECT_NAME}Config.cmake
  INSTALL_DESTINATION ${AUXILIARY_INSTALL_DIR}/CMake/
  PATH_VARS CONFIG_INCLUDE_DIR CONFIG_RUNTIME_DIR CONFIG_CMAKE_DIR
  NO_SET_AND_CHECK_MACRO
  NO_CHECK_REQUIRED_COMPONENTS_MACRO
  )

# Version information
configure_file(
  ${CMAKE_CURRENT_SOURCE_DIR}/${PROJECT_NAME}ConfigVersion.cmake.in
  ${CMAKE_CURRENT_BINARY_DIR}/${PROJECT_NAME}ConfigVersion.cmake
  @ONLY
  )

install(FILES ${CMAKE_CURRENT_BINARY_DIR}${CMAKE_FILES_DIRECTORY}/${PROJECT_NAME}Config.cmake
              ${CMAKE_CURRENT_BINARY_DIR}/${PROJECT_NAME}ConfigVersion.cmake
        DESTINATION ${AUXILIARY_INSTALL_DIR}/CMake/
        ${US_SDK_INSTALL_COMPONENT}
  )

#-----------------------------------------------------------------------------
# Build the examples
#-----------------------------------------------------------------------------

option(US_BUILD_EXAMPLES "Build example projects" OFF)
if(US_BUILD_EXAMPLES)
  if(NOT US_BUILD_SHARED_LIBS)
    message(WARNING "Examples are not available if US_BUILD_SHARED_LIBS is OFF")
  else()
    set(CppMicroServices_DIR ${PROJECT_BINARY_DIR})
    add_subdirectory(examples)
  endif()
endif()
