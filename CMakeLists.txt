project(CppMicroServices)

set(${PROJECT_NAME}_MAJOR_VERSION 2)
set(${PROJECT_NAME}_MINOR_VERSION 99)
set(${PROJECT_NAME}_PATCH_VERSION 0)
set(${PROJECT_NAME}_VERSION ${${PROJECT_NAME}_MAJOR_VERSION}.${${PROJECT_NAME}_MINOR_VERSION}.${${PROJECT_NAME}_PATCH_VERSION})

cmake_minimum_required(VERSION 2.8)
cmake_policy(VERSION 2.8)
cmake_policy(SET CMP0017 NEW)

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
include(usFunctionGetGccVersion)
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

# Set C++ compiler flags
if(NOT MSVC)
  foreach(_cxxflag  -Werror -Wall -Wextra -Wpointer-arith -Winvalid-pch -Wcast-align
                    -Wwrite-strings -Woverloaded-virtual -Wnon-virtual-dtor -Wold-style-cast
                    -Wstrict-null-sentinel -Wsign-promo -fdiagnostics-show-option)
    usFunctionCheckCompilerFlags(${_cxxflag} US_CXX_FLAGS)
  endforeach()
endif()

set(US_HAVE_VISIBILITY_ATTRIBUTE 0)
usFunctionCheckCompilerFlags("-fvisibility=hidden -fvisibility-inlines-hidden" _have_visibility)
if(_have_visibility)
  set(US_HAVE_VISIBILITY_ATTRIBUTE 1)
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
    # Enable visibility support (only for gcc >= 4.5)
    # We only support hidden visibility with gcc for now.
    #
    # Clang has troubles with correctly marking template declarations and explicit template
    # instantiations as exported across shared library boundaries. Specifically, comparing
    # type_info objects from STL types does not work (used in us::Any::Type() == typeid(std::string))
    # which could be related to the default visibility of STL types declared in libstdc++ and/or libc++
    # but also to using RTLD_LOCAL or RTLD_GLOBAL when loading shared libraries via dlopen().
    #
    # See http://comments.gmane.org/gmane.comp.compilers.clang.scm/50028
    # and http://llvm.org/bugs/show_bug.cgi?id=10113
    #
    if(_have_visibility AND NOT ${GCC_VERSION} VERSION_LESS "4.5")
      set(US_CXX_FLAGS "${US_CXX_FLAGS} ${_have_visibility}")
    else()
      set(US_GCC_RTTI_WORKAROUND_NEEDED 1)
    endif()
  endif()

  usFunctionCheckCompilerFlags("-O1 -D_FORTIFY_SOURCE=2" _fortify_source_flag)
  if(_fortify_source_flag)
    set(US_CXX_FLAGS_RELEASE "${US_CXX_FLAGS_RELEASE} -D_FORTIFY_SOURCE=2")
  endif()
endif()

if(MSVC)
  set(US_CXX_FLAGS "/MP /WX /wd4996 /wd4251 /wd4503 ${US_CXX_FLAGS}")
endif()

set(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} ${US_CXX_FLAGS}")
set(CMAKE_CXX_FLAGS_RELEASE "${CMAKE_CXX_FLAGS_RELEASE} ${US_CXX_FLAGS_RELEASE}")
set(CMAKE_C_FLAGS "${CMAKE_C_FLAGS} ${US_C_FLAGS}")
set(CMAKE_C_FLAGS_REALEASE "${CMAKE_C_FLAGS_RELEASE} ${US_C_FLAGS_RELEASE}")

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
include(CheckCXXSourceCompiles)

CHECK_INCLUDE_FILE_CXX(cxxabi.h US_HAVE_CXXABI_H)
CHECK_INCLUDE_FILE_CXX(stdint.h US_HAVE_STDINT_H)
CHECK_INCLUDE_FILE_CXX(tr1/unordered_map US_HAVE_TR1_UNORDERED_MAP_H)
CHECK_INCLUDE_FILE_CXX(tr1/unordered_set US_HAVE_TR1_UNORDERED_SET_H)
CHECK_INCLUDE_FILE_CXX(tr1/functional US_HAVE_TR1_FUNCTIONAL_H)
CHECK_INCLUDE_FILE_CXX(unordered_map US_HAVE_UNORDERED_MAP_H)
CHECK_INCLUDE_FILE_CXX(unordered_set US_HAVE_UNORDERED_SET_H)
CHECK_INCLUDE_FILE_CXX(functional US_HAVE_FUNCTIONAL_H)

if(US_HAVE_UNORDERED_MAP_H)
  CHECK_CXX_SOURCE_COMPILES("#include <unordered_map>\nint main() { std::tr1::unordered_map<int,int> m; return 0; }"
                            US_HAVE_TR1_UNORDERED_MAP)
  CHECK_CXX_SOURCE_COMPILES("#include <unordered_map>\nint main() { std::unordered_map<int,int> m; return 0; }"
                            US_HAVE_STD_UNORDERED_MAP)
  CHECK_CXX_SOURCE_COMPILES("#include <unordered_map>\nint main() { std::tr1::hash<int>(); return 0; }"
                            US_HAVE_TR1_HASH)
  CHECK_CXX_SOURCE_COMPILES("#include <unordered_map>\nint main() { std::hash<int>(); return 0; }"
                            US_HAVE_STD_HASH)
  if(US_HAVE_STD_HASH)
    CHECK_CXX_SOURCE_COMPILES("#include <unordered_map>\nstruct A { friend struct std::hash<int>; }; int main() { return 0; }"
                              US_HAVE_STD_HASH_STRUCT)
    CHECK_CXX_SOURCE_COMPILES("#include <unordered_map>\nstruct A { friend class std::hash<int>; }; int main() { return 0; }"
                              US_HAVE_STD_HASH_CLASS)
  elseif(US_HAVE_TR1_HASH)
    CHECK_CXX_SOURCE_COMPILES("#include <unordered_map>\nstruct A { friend struct std::tr1::hash<int>; }; int main() { return 0; }"
                              US_HAVE_TR1_HASH_STRUCT)
    CHECK_CXX_SOURCE_COMPILES("#include <unordered_map>\nstruct A { friend class std::tr1::hash<int>; }; int main() { return 0; }"
                              US_HAVE_TR1_HASH_CLASS)
  endif()
elseif(US_HAVE_TR1_UNORDERED_MAP_H)
  CHECK_CXX_SOURCE_COMPILES("#include <tr1/unordered_map>\nint main() { std::tr1::unordered_map<int,int> m; return 0; }"
                            US_HAVE_TR1_UNORDERED_MAP)
  CHECK_CXX_SOURCE_COMPILES("#include <tr1/unordered_map>\nint main() { std::unordered_map<int,int> m; return 0; }"
                            US_HAVE_STD_UNORDERED_MAP)
  CHECK_CXX_SOURCE_COMPILES("#include <tr1/unordered_map>\nint main() { std::tr1::hash<int>(); return 0; }"
                            US_HAVE_TR1_HASH)
  CHECK_CXX_SOURCE_COMPILES("#include <tr1/unordered_map>\nint main() { std::hash<int>(); return 0; }"
                            US_HAVE_STD_HASH)
  if(US_HAVE_STD_HASH)
    CHECK_CXX_SOURCE_COMPILES("#include <tr1/unordered_map>\nstruct A { friend struct std::hash<int>; }; int main() { return 0; }"
                              US_HAVE_STD_HASH_STRUCT)
    CHECK_CXX_SOURCE_COMPILES("#include <tr1/unordered_map>\nstruct A { friend class std::hash<int>; }; int main() { return 0; }"
                              US_HAVE_STD_HASH_CLASS)
  elseif(US_HAVE_TR1_HASH)
    CHECK_CXX_SOURCE_COMPILES("#include <tr1/unordered_map>\nstruct A { friend struct std::tr1::hash<int>; }; int main() { return 0; }"
                              US_HAVE_TR1_HASH_STRUCT)
    CHECK_CXX_SOURCE_COMPILES("#include <tr1/unordered_map>\nstruct A { friend class std::tr1::hash<int>; }; int main() { return 0; }"
                              US_HAVE_TR1_HASH_CLASS)
  endif()
else()
  message(SEND_ERROR "The header file \"unordered_map\" is not available.")
endif()
if(NOT (US_HAVE_TR1_UNORDERED_MAP OR US_HAVE_STD_UNORDERED_MAP))
  message(SEND_ERROR "The \"unordered_map\" type is not available.")
endif()

if(US_HAVE_UNORDERED_SET_H)
  CHECK_CXX_SOURCE_COMPILES("#include <unordered_set>\nint main() { std::tr1::unordered_set<int> s; return 0; }"
                            US_HAVE_TR1_UNORDERED_SET)
  CHECK_CXX_SOURCE_COMPILES("#include <unordered_set>\nint main() { std::unordered_set<int> s; return 0; }"
                            US_HAVE_STD_UNORDERED_SET)
elseif(US_HAVE_TR1_UNORDERED_SET_H)
  CHECK_CXX_SOURCE_COMPILES("#include <tr1/unordered_set>\nint main() { std::tr1::unordered_set<int> s; return 0; }"
                            US_HAVE_TR1_UNORDERED_SET)
  CHECK_CXX_SOURCE_COMPILES("#include <tr1/unordered_set>\nint main() { std::unordered_set<int> s; return 0; }"
                            US_HAVE_STD_UNORDERED_SET)
else()
  message(SEND_ERROR "The header file \"unordered_set\" is not available.")
endif()
if(NOT (US_HAVE_TR1_UNORDERED_SET OR US_HAVE_STD_UNORDERED_SET))
  message(SEND_ERROR "The \"unordered_set\" type is not available.")
endif()

if(NOT (US_HAVE_FUNCTIONAL_H OR US_HAVE_TR1_FUNCTIONAL_H))
  message(SEND_ERROR "The header file \"functional\" is not available.")
endif()

if(US_HAVE_FUNCTIONAL_H)
  CHECK_CXX_SOURCE_COMPILES("#include <functional>\nint main() { std::tr1::function<int(void)> f(main); return 0; }"
                            US_HAVE_TR1_FUNCTION)
  CHECK_CXX_SOURCE_COMPILES("#include <functional>\nint main() { std::function<int(void)> f(main); return 0; }"
                            US_HAVE_STD_FUNCTION)
endif()
if((NOT (US_HAVE_STD_FUNCTION OR US_HAVE_TR1_FUNCTION)) AND US_HAVE_TR1_FUNCTIONAL_H)
  unset(US_HAVE_TR1_FUNCTION CACHE)
  unset(US_HAVE_STD_FUNCTION CACHE)
  CHECK_CXX_SOURCE_COMPILES("#include <tr1/functional>\nint main() { std::tr1::function<int(void)> f(main); return 0; }"
                            US_HAVE_TR1_FUNCTION)
  CHECK_CXX_SOURCE_COMPILES("#include <tr1/functional>\nint main() { std::function<int(void)> f(main); return 0; }"
                            US_HAVE_STD_FUNCTION)
endif()

if(NOT (US_HAVE_STD_FUNCTION OR US_HAVE_TR1_FUNCTION))
  message(SEND_ERROR "The \"function\" type is not available.")
endif()

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
    ${US_CMAKE_DIR}/CMakeParseArguments.cmake
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
