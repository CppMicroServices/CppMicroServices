project(CppMicroServices)

set(${PROJECT_NAME}_MAJOR_VERSION 0)
set(${PROJECT_NAME}_MINOR_VERSION 99)
set(${PROJECT_NAME}_PATCH_VERSION 0)
set(${PROJECT_NAME}_VERSION ${${PROJECT_NAME}_MAJOR_VERSION}.${${PROJECT_NAME}_MINOR_VERSION}.${${PROJECT_NAME}_PATCH_VERSION})

cmake_minimum_required(VERSION 2.8)

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

include(MacroParseArguments)
include(CheckCXXSourceCompiles)
include(usFunctionCheckCompilerFlags)
include(usFunctionEmbedResources)
include(usFunctionGetGccVersion)
include(usFunctionGenerateModuleInit)

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

us_cache_var(US_ENABLE_AUTOLOADING_SUPPORT OFF BOOL "Enable module auto-loading support")
us_cache_var(US_ENABLE_THREADING_SUPPORT OFF BOOL "Enable threading support")
us_cache_var(US_ENABLE_DEBUG_OUTPUT OFF BOOL "Enable debug messages" ADVANCED)
us_cache_var(US_ENABLE_RESOURCE_COMPRESSION ON BOOL "Enable resource compression" ADVANCED)
us_cache_var(US_BUILD_SHARED_LIBS ON BOOL "Build shared libraries")
us_cache_var(US_BUILD_TESTING OFF BOOL "Build tests")

if(MSVC10 OR MSVC11)
  # Visual Studio 2010 and newer have support for C++11 enabled by default
  set(US_USE_C++11 1)
else()
  us_cache_var(US_USE_C++11 OFF BOOL "Enable the use of C++11 features" ADVANCED)
endif()

us_cache_var(US_NAMESPACE "us" STRING "The namespace for the C++ micro services entities")
us_cache_var(US_HEADER_PREFIX "" STRING "The file name prefix for the public C++ micro services header files")

set(BUILD_SHARED_LIBS ${US_BUILD_SHARED_LIBS})

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
if(NOT MSVC)
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

# link libraries for third party libs
if(US_IS_EMBEDDED)
  set(US_LINK_LIBRARIES ${US_EMBEDDING_LIBRARY})
else()
  set(US_LINK_LIBRARIES ${PROJECT_NAME})
endif()

# link libraries for the CppMicroServices lib
set(_link_libraries )
if(UNIX)
  list(APPEND _link_libraries dl)
endif()
list(APPEND US_LINK_LIBRARIES ${_link_libraries})

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
# Last configuration steps
#-----------------------------------------------------------------------------

configure_file(${PROJECT_NAME}Config.cmake.in ${PROJECT_BINARY_DIR}/${PROJECT_NAME}Config.cmake @ONLY)

