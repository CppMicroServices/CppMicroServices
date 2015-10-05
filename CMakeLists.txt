project(CppMicroServices)

set(${PROJECT_NAME}_MAJOR_VERSION 2)
set(${PROJECT_NAME}_MINOR_VERSION 99)
set(${PROJECT_NAME}_PATCH_VERSION 0)
set(${PROJECT_NAME}_VERSION ${${PROJECT_NAME}_MAJOR_VERSION}.${${PROJECT_NAME}_MINOR_VERSION}.${${PROJECT_NAME}_PATCH_VERSION})

cmake_minimum_required(VERSION 2.8.7) # CMake version in Ubuntu 14.04
cmake_policy(VERSION 2.8.7)

list(APPEND policies_new
  CMP0017 # Prefer files from the CMake module directory when including from there
  CMP0025 # Compiler id for Apple Clang is now AppleClang (CMake 3.0)
  CMP0054 # Only interpret if() arguments as variables or keywords when unquoted
  )
foreach(policy ${policies_new})
  if(POLICY ${policy})
    cmake_policy(SET ${policy} NEW)
  endif()
endforeach()

#-----------------------------------------------------------------------------
# Check minimum required compiler versions
#------------------------------------------------------------------------------

# We make use of the following C++11 language and library features:
#
# Language feature                 GCC   Clang   AppleClang   VisualStudio
# ------------------------------------------------------------------------
# Rvalue references                4.3   2.9     ??           2012
# Variadic templates               4.3   2.9     ??           2013*
# Initializer lists                4.4   3.1     ??           2013
# Static assertions                4.3   2.9     ??           2010
# auto                             4.4   2.9     ??           2010
# Lambdas                          4.5   3.1     ??           2012
# decltype                         4.3   2.9     ??           2012
# Right angle brackets             4.3   2.9     ??           2010
# Null pointer constant            4.6   3.0     ??           2010
# Defaulted functions              4.4   3.0     ??           2013*
# Deleted functions                4.4   2.9     ??           2013*
# Range-based for                  4.6   3.0     ??           2012
# Atomic operations                4.4   3.1     ??           2012
#
# Library features used
# ---------------------
# function
# unique_ptr
# shared_ptr
# unordered_map
# unordered_set
# atomic
# mutex
# wait_condition

set(US_COMPILER_GNU_MINIMUM_VERSION           4.6          )
set(US_COMPILER_CLANG_MINIMUM_VERSION         3.3          ) # this could probably be 3.1, but we only tested with 3.3
set(US_COMPILER_APPLE_CLANG_MINIMUM_VERSION   5.0          ) # ??
set(US_COMPILER_MSVC_MINIMUM_VERSION          18.00.31101 )

set(US_COMPILER_MSVC_MINIMUM_VERSION_PRODUCT "Visual Studio 2013 Update 4")

# CMAKE_CXX_COMPILER_VERSION is only available in CMake >= 2.8.10

if("${CMAKE_CXX_COMPILER_ID}" STREQUAL "GNU")
  if (CMAKE_CXX_COMPILER_VERSION AND CMAKE_CXX_COMPILER_VERSION VERSION_LESS ${US_COMPILER_GNU_MINIMUM_VERSION})
    message(FATAL_ERROR "GCC version ${US_COMPILER_GNU_MINIMUM_VERSION} or higher required (you are using version ${CMAKE_CXX_COMPILER_VERSION}).")
  endif()
  set(US_COMPILER_GNU 1)
elseif("${CMAKE_CXX_COMPILER_ID}" STREQUAL "Clang")
  if (CMAKE_CXX_COMPILER_VERSION AND CMAKE_CXX_COMPILER_VERSION VERSION_LESS ${US_COMPILER_CLANG_MINIMUM_VERSION})
    message(FATAL_ERROR "Clang version ${US_COMPILER_CLANG_MINIMUM_VERSION} or higher required (you are using version ${CMAKE_CXX_COMPILER_VERSION}).")
  endif()
  set(US_COMPILER_CLANG 1)
elseif("${CMAKE_CXX_COMPILER_ID}" STREQUAL "AppleClang")
  if (CMAKE_CXX_COMPILER_VERSION AND CMAKE_CXX_COMPILER_VERSION VERSION_LESS ${US_COMPILER_APPLE_CLANG_MINIMUM_VERSION})
    message(FATAL_ERROR "Apple Clang version ${US_COMPILER_APPLE_CLANG_MINIMUM_VERSION} or higher required (you are using version ${CMAKE_CXX_COMPILER_VERSION}).")
  endif()
  set(US_COMPILER_APPLE_CLANG 1)
elseif("${CMAKE_CXX_COMPILER_ID}" STREQUAL "MSVC")
  if (CMAKE_CXX_COMPILER_VERSION AND CMAKE_CXX_COMPILER_VERSION VERSION_LESS ${US_COMPILER_MSVC_MINIMUM_VERSION})
    message(FATAL_ERROR "${US_COMPILER_MSVC_MINIMUM_VERSION_PRODUCT} or newer required (MSVC ${US_COMPILER_MSVC_MINIMUM_VERSION} or higher, you are using version ${CMAKE_CXX_COMPILER_VERSION})")
  endif()
  set(US_COMPILER_MSVC 1)
else()
  message(WARNING "You are using an unsupported compiler! Compilation has only been tested with Clang (Linux or Apple), GCC and MSVC.")
endif()

#-----------------------------------------------------------------------------
# Update CMake module path
#------------------------------------------------------------------------------

set(US_CMAKE_DIR ${PROJECT_SOURCE_DIR}/cmake)

set(CMAKE_MODULE_PATH
  ${US_CMAKE_DIR}
  ${CMAKE_MODULE_PATH}
  )

#-----------------------------------------------------------------------------
# CMake function(s) and macro(s)
#-----------------------------------------------------------------------------

include(CMakeParseArguments)
include(CMakePackageConfigHelpers)
include(CheckCXXSourceCompiles)
include(usFunctionAddResources)
include(usFunctionEmbedResources)
include(usFunctionGetResourceSource)
include(usFunctionCheckResourceLinking)
include(usFunctionCheckCompilerFlags)
include(usFunctionGenerateModuleInit)
include(usMacroCreateModule)

if(US_BUILD_TESTING)
  include(usFunctionCompileSnippets)
endif()

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
us_cache_var(US_BUILD_SHARED_LIBS ON BOOL "Build shared libraries")
us_cache_var(US_BUILD_TESTING OFF BOOL "Build tests")
us_cache_var(US_BUILD_EXAMPLES OFF BOOL "Build example projects")

if(US_BUILD_TESTING)
  enable_testing()
endif()

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
set(AUXILIARY_CMAKE_INSTALL_DIR ${AUXILIARY_INSTALL_DIR}/cmake)

us_cache_var(US_NAMESPACE "us" STRING "The namespace for the C++ Micro Services symbols")

set(BUILD_SHARED_LIBS ${US_BUILD_SHARED_LIBS})

set(US_MODULE_INIT_TEMPLATE "${US_CMAKE_DIR}/usModuleInit.cpp" CACHE INTERNAL "The module initialization template code")
set(US_RESOURCE_RC_TEMPLATE "${US_CMAKE_DIR}/us_resources.rc.in" CACHE INTERNAL "The Windows RC resource template")
set(US_CMAKE_RESOURCE_DEPENDENCIES_CPP "${US_CMAKE_DIR}/usCMakeResourceDependencies.cpp" CACHE INTERNAL "The dummy resource dependencies code")

#-----------------------------------------------------------------------------
# US C/CXX Flags
#-----------------------------------------------------------------------------

if(US_IS_EMBEDDED)
  set(CMAKE_C_FLAGS)
  set(CMAKE_C_FLAGS_RELEASE)
  set(CMAKE_C_FLAGS_DEBUG)
  set(CMAKE_CXX_FLAGS)
  set(CMAKE_CXX_FLAGS_RELEASE)
  set(CMAKE_CXX_FLAGS_DEBUG)
  set(CMAKE_LINK_FLAGS)
  set(CMAKE_LINK_FLAGS_RELEASE)
  set(CMAKE_LINK_FLAGS_DEBUG)
endif()

set(US_HAVE_VISIBILITY_ATTRIBUTE 0)
set(US_CXX11_FLAGS )
set(US_CXX11_LINK_FLAGS )

if(MSVC)
  set(disabled_warnings
    4251 # 'identifier' : class 'type' needs to have dll-interface to be used by clients of class 'type2'
  )
  foreach(wd ${disabled_warnings})
    set(US_CXX_FLAGS "/wd${wd} ${US_CXX_FLAGS}")
  endforeach()
  set(US_CXX_FLAGS "/MP /WX ${US_CXX_FLAGS}")
else()
  # C++11 support
  set(US_CXX11_FLAGS -std=c++11)
  usFunctionCheckCompilerFlags(${US_CXX11_FLAGS} _have_cxx11)
  if(_have_cxx11)
    set(US_CXX_FLAGS "${US_CXX_FLAGS} ${US_CXX11_FLAGS}")
  else()
    set(US_CXX11_FLAGS -std=c++0x)
    usFunctionCheckCompilerFlags(${US_CXX11_FLAGS} _have_cxx0x)
    if(_have_cxx0x)
      set(US_CXX_FLAGS "${US_CXX_FLAGS} ${US_CXX11_FLAGS}")
    else()
      set(US_CXX11_FLAGS )
    endif()
  endif()

  if(US_COMPILER_APPLE_CLANG)
    set(US_CXX11_FLAGS "${US_CXX11_FLAGS} -stdlib=libc++")
    set(US_CXX_FLAGS "${US_CXX_FLAGS} -stdlib=libc++")
  endif()

  foreach(_cxxflag  -Werror -Wall -Wextra -Wpointer-arith -Winvalid-pch -Wcast-align
                    -Wwrite-strings -Woverloaded-virtual -Wnon-virtual-dtor -Wold-style-cast
                    -Wstrict-null-sentinel -Wsign-promo -fdiagnostics-show-option
                    -Wno-mismatched-tags
                    -fstack-protector-all)
    usFunctionCheckCompilerFlags(${_cxxflag} US_CXX_FLAGS)
  endforeach()

  usFunctionCheckCompilerFlags("-fvisibility=hidden -fvisibility-inlines-hidden" _have_visibility)
  if(_have_visibility)
    set(US_HAVE_VISIBILITY_ATTRIBUTE 1)
    set(US_CXX_FLAGS "${US_CXX_FLAGS} ${_have_visibility}")
  endif()

  usFunctionCheckCompilerFlags("-O1 -D_FORTIFY_SOURCE=2" _fortify_source_flag)
  if(_fortify_source_flag)
    set(US_CXX_FLAGS_RELEASE "${US_CXX_FLAGS_RELEASE} -D_FORTIFY_SOURCE=2")
  endif()
endif()

if(MINGW)
  # suppress warnings about auto imported symbols
  set(US_CXX_FLAGS "-Wl,--enable-auto-import ${US_CXX_FLAGS}")
  # we need to define a Windows version
  set(US_CXX_FLAGS "-D_WIN32_WINNT=0x0500 ${US_CXX_FLAGS}")
endif()

set(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} ${US_CXX_FLAGS}")
set(CMAKE_CXX_FLAGS_RELEASE "${CMAKE_CXX_FLAGS_RELEASE} ${US_CXX_FLAGS_RELEASE}")
set(CMAKE_C_FLAGS "${CMAKE_C_FLAGS} ${US_C_FLAGS}")
set(CMAKE_C_FLAGS_RELEASE "${CMAKE_C_FLAGS_RELEASE} ${US_C_FLAGS_RELEASE}")

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

usFunctionCheckResourceLinking()

#-----------------------------------------------------------------------------
# US Header Checks
#-----------------------------------------------------------------------------

include(CheckIncludeFileCXX)

CHECK_INCLUDE_FILE_CXX(cxxabi.h US_HAVE_CXXABI_H)

#-----------------------------------------------------------------------------
# Source directory
#-----------------------------------------------------------------------------

set(US_INCLUDE_DIRS ${PROJECT_SOURCE_DIR}/third_party ${PROJECT_BINARY_DIR}/include)

# Configure CppMicroServicesConfig.cmake for the build tree.
# The file is used in sub-directories.

set(PACKAGE_CONFIG_INCLUDE_DIR ${US_INCLUDE_DIRS})
set(PACKAGE_CONFIG_RUNTIME_DIR ${CMAKE_RUNTIME_OUTPUT_DIRECTORY})
set(PACKAGE_CONFIG_CMAKE_DIR ${US_CMAKE_DIR})

set(US_RCC_EXECUTABLE_NAME usResourceCompiler CACHE INTERNAL "The target name of the usResourceCompiler executable.")

configure_file(
  ${CMAKE_CURRENT_SOURCE_DIR}/${PROJECT_NAME}Config.cmake.in
  ${PROJECT_BINARY_DIR}/${PROJECT_NAME}Config.cmake
  @ONLY
  )

set(us_global_config_h_file "${PROJECT_BINARY_DIR}/include/usGlobalConfig.h")
configure_file(${US_CMAKE_DIR}/usGlobalConfig.h.in ${us_global_config_h_file})

include_directories(${US_INCLUDE_DIRS})

add_subdirectory(tools)

add_subdirectory(core)

#-----------------------------------------------------------------------------
# Documentation
#-----------------------------------------------------------------------------

add_subdirectory(doc)

#-----------------------------------------------------------------------------
# Last configuration and install steps
#-----------------------------------------------------------------------------

# Version information
configure_file(
  ${CMAKE_CURRENT_SOURCE_DIR}/${PROJECT_NAME}ConfigVersion.cmake.in
  ${CMAKE_CURRENT_BINARY_DIR}/${PROJECT_NAME}ConfigVersion.cmake
  @ONLY
  )

export(TARGETS ${US_RCC_EXECUTABLE_NAME}
       FILE ${CMAKE_CURRENT_BINARY_DIR}/${PROJECT_NAME}Targets.cmake)

if(NOT US_NO_INSTALL)

  install(EXPORT ${PROJECT_NAME}Targets
          FILE ${PROJECT_NAME}Targets.cmake
          DESTINATION ${AUXILIARY_CMAKE_INSTALL_DIR})

  set(_install_cmake_scripts
    ${US_MODULE_INIT_TEMPLATE}
    ${US_CMAKE_RESOURCE_DEPENDENCIES_CPP}
    ${US_CMAKE_DIR}/FindPackageHandleStandardArgs.cmake
    ${US_CMAKE_DIR}/FindPackageMessage.cmake
    ${US_CMAKE_DIR}/usFunctionGenerateModuleInit.cmake
    ${US_CMAKE_DIR}/usFunctionAddResources.cmake
    ${US_CMAKE_DIR}/usFunctionEmbedResources.cmake
    ${US_CMAKE_DIR}/usFunctionGetResourceSource.cmake
    ${US_CMAKE_DIR}/usFunctionCheckResourceLinking.cmake
    ${US_CMAKE_DIR}/usFunctionCheckCompilerFlags.cmake
    )

  install(FILES ${_install_cmake_scripts}
          DESTINATION ${AUXILIARY_CMAKE_INSTALL_DIR})

  install(FILES ${us_global_config_h_file}
          DESTINATION ${HEADER_INSTALL_DIR})

  # Configure CppMicroServicesConfig.cmake for the install tree
  set(CONFIG_INCLUDE_DIR ${HEADER_INSTALL_DIR})
  set(CONFIG_RUNTIME_DIR ${RUNTIME_INSTALL_DIR})
  set(CONFIG_CMAKE_DIR ${AUXILIARY_CMAKE_INSTALL_DIR})
  configure_package_config_file(
    ${CMAKE_CURRENT_SOURCE_DIR}/${PROJECT_NAME}Config.cmake.in
    ${PROJECT_BINARY_DIR}${CMAKE_FILES_DIRECTORY}/${PROJECT_NAME}Config.cmake
    INSTALL_DESTINATION ${AUXILIARY_CMAKE_INSTALL_DIR}
    PATH_VARS CONFIG_INCLUDE_DIR CONFIG_RUNTIME_DIR CONFIG_CMAKE_DIR
    NO_SET_AND_CHECK_MACRO
    NO_CHECK_REQUIRED_COMPONENTS_MACRO
    )

  install(FILES ${CMAKE_CURRENT_BINARY_DIR}${CMAKE_FILES_DIRECTORY}/${PROJECT_NAME}Config.cmake
                ${CMAKE_CURRENT_BINARY_DIR}/${PROJECT_NAME}ConfigVersion.cmake
          DESTINATION ${AUXILIARY_CMAKE_INSTALL_DIR}
          ${US_SDK_INSTALL_COMPONENT}
    )

endif()
