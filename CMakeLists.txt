project(CppMicroServices)

cmake_minimum_required(VERSION 2.8)

#-----------------------------------------------------------------------------
# Update CMake module path
#------------------------------------------------------------------------------

set(CMAKE_MODULE_PATH
  ${PROJECT_SOURCE_DIR}/CMake
  ${CMAKE_MODULE_PATH}
  )

#-----------------------------------------------------------------------------
# CMake Function(s) and Macro(s)
#-----------------------------------------------------------------------------

include(MacroParseArguments)
include(CheckCXXSourceCompiles)
include(usFunctionCheckCompilerFlags)
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

us_cache_var(US_ENABLE_SERVICE_FACTORY_SUPPORT ON BOOL "Enable Service Factory support" ADVANCED)
us_cache_var(US_ENABLE_THREADING_SUPPORT OFF BOOL "Enable threading support")
us_cache_var(US_BUILD_SHARED_LIBS ON BOOL "Build shared libraries")
us_cache_var(US_BUILD_TESTING OFF BOOL "Build tests")

if(MSVC10)
  # Visual Studio 2010 has support for C++11 enabled by default
  set(US_USE_C++11 1)
else()
  us_cache_var(US_USE_C++11 OFF BOOL "Enable the use of C++11 features" ADVANCED)
endif()

us_cache_var(US_NAMESPACE "us" STRING "The namespace for the C++ micro services entities")
us_cache_var(US_HEADER_PREFIX "" STRING "The file name prefix for the public C++ micro services header files")
us_cache_var(US_BASECLASS_NAME "" STRING "The fully-qualified name of the base class")

if(US_ENABLE_SERVICE_FACTORY_SUPPORT)
  us_cache_var(US_BASECLASS_PACKAGE "" STRING "The name of the package providing the base class definition" ADVANCED)

  set(bc_inc_d_doc "A list of include directories containing the header files for the base class")
  us_cache_var(US_BASECLASS_INCLUDE_DIRS "" STRING "${bc_inc_d_doc}" ADVANCED)

  set(bc_lib_d_doc "A list of library directories for the base class")
  us_cache_var(US_BASECLASS_LIBRARY_DIRS "" STRING "${bc_lib_d_doc}" ADVANCED)

  set(bc_lib_doc "A list of libraries needed for the base class")
  us_cache_var(US_BASECLASS_LIBRARIES "" STRING "${bc_lib_doc}" ADVANCED)

  us_cache_var(US_BASECLASS_HEADER "" STRING "The name of the header file containing the base class declaration" ADVANCED)
endif()

set(BUILD_SHARED_LIBS ${US_BUILD_SHARED_LIBS})

# Sanity checks

if(US_ENABLE_SERVICE_FACTORY_SUPPORT OR US_BUILD_TESTING)
  if(US_BASECLASS_PACKAGE)
    find_package(${US_BASECLASS_PACKAGE} REQUIRED)

    # Try to get the include dirs
    foreach(_suffix DIRECTORIES DIRS DIRECTORY DIR)
      if(${US_BASECLASS_PACKAGE}_INCLUDE_${_suffix} AND NOT US_BASECLASS_INCLUDE_DIRS)
        us_cache_var(US_BASECLASS_INCLUDE_DIRS ${${US_BASECLASS_PACKAGE}_INCLUDE_${_suffix}} STRING "${bc_inc_d_doc}" FORCE)
        break()
      endif()
    endforeach()

    # Try to get the library dirs
    foreach(_suffix DIRECTORIES DIRS DIRECTORY DIR)
      if(${US_BASECLASS_PACKAGE}_LIBRARY_${_suffix} AND NOT US_BASECLASS_LIBRARY_DIRS)
        us_cache_var(US_BASECLASS_LIBRARY_DIRS ${${US_BASECLASS_PACKAGE}_LIBRARY_${_suffix}} STRING "${bc_lib_d_doc}" FORCE)
        break()
      endif()
    endforeach()

    # Try to get the libraries
    foreach(_suffix LIBRARIES LIBS LIBRARY LIB)
      if(${US_BASECLASS_PACKAGE}_${_suffix} AND NOT US_BASECLASS_LIBRARIES)
        us_cache_var(US_BASECLASS_LIBRARIES ${${US_BASECLASS_PACKAGE}_${_suffix}} STRING "${bc_lib_doc}" FORCE)
        break()
      endif()
    endforeach()

    if(NOT US_BASECLASS_NAME)
      message(FATAL_ERROR "US_BASECLASS_NAME not set")
    elseif(NOT US_BASECLASS_HEADER)
      message(FATAL_ERROR "US_BASECLASS_HEADER not set")
    endif()
  endif()

  if(US_ENABLE_SERVICE_FACTORY_SUPPORT AND US_BASECLASS_NAME AND NOT US_BASECLASS_HEADER)
    message(FATAL_ERROR "US_ENABLE_SERVICE_FACTORY_SUPPORT requires a US_BASECLASS_HEADER value")
  endif()
endif()

set(_us_baseclass_default 0)
if(NOT US_BASECLASS_NAME)
  message(WARNING "Using build in base class \"::${US_NAMESPACE}::Base\"")
  set(_us_baseclass_default 1)
  set(US_BASECLASS_NAME "${US_NAMESPACE}::Base")
  set(US_BASECLASS_HEADER "usBase.h")
endif()

if(US_BUILD_TESTING AND US_BASECLASS_NAME AND NOT US_BASECLASS_HEADER)
  message(FATAL_ERROR "US_BUILD_TESTING requires a US_BASECLASS_HEADER value")
endif()

set(US_BASECLASS_INCLUDE "#include <${US_BASECLASS_HEADER}>")

string(REPLACE "::" ";" _bc_token "${US_BASECLASS_NAME}")
list(GET _bc_token -1 _bc_name)
list(REMOVE_AT _bc_token -1)

set(US_BASECLASS_FORWARD_DECLARATION "")
foreach(_namespace_tok ${_bc_token})
  if(_namespace_tok)
    set(US_BASECLASS_FORWARD_DECLARATION "${US_BASECLASS_FORWARD_DECLARATION}namespace ${_namespace_tok} { ")
  endif()
endforeach()
set(US_BASECLASS_FORWARD_DECLARATION "${US_BASECLASS_FORWARD_DECLARATION}class ${_bc_name}; ")
foreach(_namespace_tok ${_bc_token})
  if(_namespace_tok)
    set(US_BASECLASS_FORWARD_DECLARATION "${US_BASECLASS_FORWARD_DECLARATION}}")
  endif()
endforeach()

#-----------------------------------------------------------------------------
# US C/CXX Flags
#-----------------------------------------------------------------------------

set(US_C_FLAGS "${COVERAGE_C_FLAGS} ${ADDITIONAL_C_FLAGS}")
set(US_CXX_FLAGS "${COVERAGE_CXX_FLAGS} ${ADDITIONAL_CXX_FLAGS}")

# This is used as a preprocessor define
set(US_USE_CXX11 ${US_USE_C++11})

if(CMAKE_COMPILER_IS_GNUCXX)
  set(cflags "-Wall -Wextra -Wpointer-arith -Winvalid-pch -Wcast-align -Wwrite-strings -D_FORTIFY_SOURCE=2")

  usFunctionCheckCompilerFlags("-fdiagnostics-show-option" cflags)
  usFunctionCheckCompilerFlags("-Wl,--no-undefined" cflags)

  if(US_USE_C++11)
    usFunctionCheckCompilerFlags("-std=c++0x" US_CXX_FLAGS)
  endif()

  usFunctionGetGccVersion(${CMAKE_CXX_COMPILER} GCC_VERSION)

  # With older version of gcc supporting the flag -fstack-protector-all, an extra dependency to libssp.so
  # is introduced. If gcc is smaller than 4.4.0 and the build type is Release let's not include the flag.
  # Doing so should allow to build package made for distribution using older linux distro.
  if(${GCC_VERSION} VERSION_GREATER "4.4.0" OR (CMAKE_BUILD_TYPE STREQUAL "Debug" AND ${GCC_VERSION} VERSION_LESS "4.4.0"))
    usFunctionCheckCompilerFlags("-fstack-protector-all" cflags)
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

  set(US_C_FLAGS "${cflags} ${US_C_FLAGS}")
  set(US_CXX_FLAGS "${cflags} -Woverloaded-virtual -Wold-style-cast -Wstrict-null-sentinel -Wsign-promo ${US_CXX_FLAGS}")
elseif(MSVC90 OR MSVC10)
  set(US_CXX_FLAGS "/MP ${US_CXX_FLAGS}")
endif()

if(NOT US_IS_EMBEDDED)
  set(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} ${US_CXX_FLAGS}")
  set(CMAKE_C_FLAGS "${CMAKE_C_FLAGS} ${US_C_FLAGS}")
endif()

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

if(US_ENABLE_SERVICE_FACTORY_SUPPORT)
  list(APPEND _us_internal_include_dirs ${US_BASECLASS_INCLUDE_DIRS})
endif()

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

if(US_ENABLE_SERVICE_FACTORY_SUPPORT)
  list(APPEND US_LINK_LIBRARIES ${US_BASECLASS_LIBRARIES})
endif()

set(US_LINK_DIRS )
if(US_ENABLE_SERVICE_FACTORY_SUPPORT)
  list(APPEND US_LINK_DIRS ${US_BASECLASS_LIBRARY_DIRS})
endif()

#-----------------------------------------------------------------------------
# Source directory
#-----------------------------------------------------------------------------

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

configure_file(usConfig.h.in ${PROJECT_BINARY_DIR}/include/usConfig.h)

configure_file(${PROJECT_NAME}Config.cmake.in ${PROJECT_BINARY_DIR}/${PROJECT_NAME}Config.cmake @ONLY)

