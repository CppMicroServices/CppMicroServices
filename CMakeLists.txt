project(CppMicroServices)

set(${PROJECT_NAME}_MAJOR_VERSION 2)
set(${PROJECT_NAME}_MINOR_VERSION 99)
set(${PROJECT_NAME}_PATCH_VERSION 0)
set(${PROJECT_NAME}_VERSION ${${PROJECT_NAME}_MAJOR_VERSION}.${${PROJECT_NAME}_MINOR_VERSION}.${${PROJECT_NAME}_PATCH_VERSION})

string(TOLOWER ${PROJECT_NAME} PROJECT_NAME_LOWER)

set(US_CMAKE_MINIMUM_REQUIRED_VERSION 2.8.12)

cmake_minimum_required(VERSION ${US_CMAKE_MINIMUM_REQUIRED_VERSION})
cmake_policy(VERSION ${US_CMAKE_MINIMUM_REQUIRED_VERSION})

list(APPEND policies_new
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
# Explicit conversion operators    4.5   3.0     ??           2013
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
# exception_ptr

set(US_COMPILER_GNU_MINIMUM_VERSION           4.6          )
set(US_COMPILER_CLANG_MINIMUM_VERSION         3.1          )
set(US_COMPILER_APPLE_CLANG_MINIMUM_VERSION   5.0          ) # ??
set(US_COMPILER_MSVC_MINIMUM_VERSION          18.00.31101 )

set(US_COMPILER_MSVC_MINIMUM_VERSION_PRODUCT "Visual Studio 2013 Update 4")

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
include(usFunctionGenerateBundleInit)
include(usMacroCreateBundle)

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

us_cache_var(US_ENABLE_THREADING_SUPPORT ON BOOL "Enable threading support")
us_cache_var(US_ENABLE_TSAN OFF BOOL "Enable tsan (thread sanitizer)" ADVANCED)
us_cache_var(US_BUILD_SHARED_LIBS ON BOOL "Build shared libraries")
us_cache_var(US_BUILD_TESTING OFF BOOL "Build tests")
us_cache_var(US_BUILD_EXAMPLES OFF BOOL "Build example projects")

if(WIN32 AND NOT CYGWIN)
  set(default_runtime_install_dir bin/)
  set(default_library_install_dir bin/)
  set(default_archive_install_dir lib/)
  set(default_header_install_dir include/)
  set(default_auxiliary_install_dir share/)
else()
  set(default_runtime_install_dir bin/)
  set(default_library_install_dir lib/)
  set(default_archive_install_dir lib/)
  set(default_header_install_dir include/)
  set(default_auxiliary_install_dir share/${PROJECT_NAME_LOWER})
endif()

us_cache_var(RUNTIME_INSTALL_DIR ${default_runtime_install_dir} STRING "Relative install location for binaries" ADVANCED)
us_cache_var(LIBRARY_INSTALL_DIR ${default_library_install_dir} STRING "Relative install location for libraries" ADVANCED)
us_cache_var(ARCHIVE_INSTALL_DIR ${default_archive_install_dir} STRING "Relative install location for archives" ADVANCED)
us_cache_var(HEADER_INSTALL_DIR ${default_header_install_dir} STRING "Relative install location for headers" ADVANCED)
us_cache_var(AUXILIARY_INSTALL_DIR ${default_auxiliary_install_dir} STRING "Relative install location for auxiliary files" ADVANCED)
set(AUXILIARY_CMAKE_INSTALL_DIR ${AUXILIARY_INSTALL_DIR}/cmake)

set(BUILD_SHARED_LIBS ${US_BUILD_SHARED_LIBS})

set(US_BUNDLE_INIT_TEMPLATE "${US_CMAKE_DIR}/BundleInit.cpp" CACHE INTERNAL "The bundle initialization template code")
set(US_RESOURCE_RC_TEMPLATE "${US_CMAKE_DIR}/cppmicroservices_resources.rc.in" CACHE INTERNAL "The Windows RC resource template")
set(US_CMAKE_RESOURCE_DEPENDENCIES_CPP "${US_CMAKE_DIR}/CMakeResourceDependencies.cpp" CACHE INTERNAL "The dummy resource dependencies code")
if(APPLE)
  # Avoid RPATH warning in cmakegui
  set(CMAKE_MACOSX_RPATH 1)
endif()

#-----------------------------------------------------------------------------
# Testing configuration
#-----------------------------------------------------------------------------

function(us_add_tests test_driver)
  foreach(test ${ARGN})
    add_test(NAME ${test} COMMAND ${test_driver} ${test}
      WORKING_DIRECTORY ${CMAKE_BINARY_DIR})
    set_property(TEST ${test} PROPERTY LABELS regular)
  if(US_MEMCHECK_COMMAND)
    add_test(NAME memcheck_${test} COMMAND ${US_MEMCHECK_COMMAND} --error-exitcode=1 ${US_RUNTIME_OUTPUT_DIRECTORY}/${test_driver} ${test}
      WORKING_DIRECTORY ${CMAKE_BINARY_DIR})
    set_property(TEST memcheck_${test} PROPERTY LABELS valgrind memcheck)
    if(US_ENABLE_HELGRIND)
      # By default, these tests are not active until the concurrent bundle
      # start / stop mechanism has been properly implemented (GitHub Issue #25)
      add_test(NAME helgrind_${test} COMMAND ${US_MEMCHECK_COMMAND} --tool=helgrind --error-exitcode=1 ${US_RUNTIME_OUTPUT_DIRECTORY}/${test_driver} ${test}
        WORKING_DIRECTORY ${CMAKE_BINARY_DIR})
      set_property(TEST helgrind_${test} PROPERTY LABELS valgrind helgrind)
    endif()
  endif()
  endforeach()
endfunction()

# Valgrind rc file
configure_file(
  ${CMAKE_CURRENT_SOURCE_DIR}/valgrindrc
  ${CMAKE_BINARY_DIR}/.valgrindrc
  @ONLY
  )

if(US_BUILD_TESTING)
  enable_testing()
  find_program(US_MEMCHECK_COMMAND valgrind)
  mark_as_advanced(US_MEMCHECK_COMMAND)
endif()

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
set(US_CXX_TSAN_FLAGS )
set(US_TSAN_LINK_FLAGS )

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
                    -fstack-protector-all)
    usFunctionCheckCompilerFlags(${_cxxflag} US_CXX_FLAGS)
  endforeach()

  if(US_COMPILER_CLANG OR US_COMPILER_APPLE_CLANG)
    # The check does not work reliably for gcc
    usFunctionCheckCompilerFlags(-Wno-mismatched-tags US_CXX_FLAGS)
  endif()

  # Issue #56: Workaround a clang compiler issue (https://llvm.org/bugs/show_bug.cgi?id=26749).
  # The typeinfo symbols do not have reliable symbol visibility. Instead of exporting only
  # symbols and associated typeinfo symbols which have explicit "default" visibility,
  # export all typeinfo symbols by default and maintain hidden symbol visibility for all other
  # symbols (unless explicitly overriden). See
  # https://developer.apple.com/library/mac/technotes/tn2185/_index.html#//apple_ref/doc/uid/DTS10004200-CH1-SUBSECTION2
  # to understand what -fvisibility-ms-compat does.
  if(APPLE AND (US_COMPILER_CLANG OR US_COMPILER_APPLE_CLANG))
    usFunctionCheckCompilerFlags("-fvisibility-ms-compat" _have_visibility)
  else()
    usFunctionCheckCompilerFlags("-fvisibility=hidden -fvisibility-inlines-hidden" _have_visibility)
  endif()
  if(_have_visibility)
    set(US_HAVE_VISIBILITY_ATTRIBUTE 1)
    set(US_CXX_FLAGS "${US_CXX_FLAGS} ${_have_visibility}")
  endif()

  usFunctionCheckCompilerFlags("-O1 -D_FORTIFY_SOURCE=2" _fortify_source_flag)
  if(_fortify_source_flag)
    set(US_CXX_FLAGS_RELEASE "${US_CXX_FLAGS_RELEASE} -U_FORTIFY_SOURCE -D_FORTIFY_SOURCE=2")
  endif()

  set(_orig_req_libs ${CMAKE_REQUIRED_LIBRARIES})
  set(CMAKE_REQUIRED_LIBRARIES "-fsanitize=thread")
  usFunctionCheckCompilerFlags("-O1 -fsanitize=thread" US_CXX_TSAN_FLAGS)
  set(CMAKE_REQUIRED_LIBRARIES "${_orig_req_libs}")
  if(NOT US_CXX_TSAN_FLAGS AND US_ENABLE_TSAN)
    message(WARNING "TSAN requested, but compiler does not recognize it")
  endif()
endif()

if(MINGW)
  # suppress warnings about auto imported symbols
  set(US_CXX_FLAGS "-Wl,--enable-auto-import ${US_CXX_FLAGS}")
  # we need to define a Windows version
  set(US_CXX_FLAGS "-D_WIN32_WINNT=0x0500 ${US_CXX_FLAGS}")
endif()

if(US_ENABLE_THREADING_SUPPORT)
  set(THREADS_PREFER_PTHREAD_FLAG 1)
  find_package(Threads REQUIRED)

  if(US_ENABLE_TSAN AND US_CXX_TSAN_FLAGS)
    set(US_CXX_FLAGS "${US_CXX_FLAGS} ${US_CXX_TSAN_FLAGS}")
  endif()
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

if(NOT MSVC)
  if (US_ENABLE_TSAN AND US_CXX_TSAN_FLAGS)
    set(US_TSAN_LINK_FLAGS "-fsanitize=thread")
  endif()
endif()

if(US_ENABLE_THREADING_SUPPORT AND US_TSAN_LINK_FLAGS)
  set(US_LINK_FLAGS "${US_LINK_FLAGS} ${US_TSAN_LINK_FLAGS}")
endif()

usFunctionCheckResourceLinking()

#-----------------------------------------------------------------------------
# US Header Checks
#-----------------------------------------------------------------------------

include(CheckIncludeFileCXX)

CHECK_INCLUDE_FILE_CXX(cxxabi.h US_HAVE_CXXABI_H)

#-----------------------------------------------------------------------------
# C++ library support
#-----------------------------------------------------------------------------

# c++11 regular expressions are used in the unit tests.
# Currently the default C++ library for gcc < 4.9 doesn't
# fully support c++11 regex.
# Generelize the detection of regex and make its availability
# conditional of wether the compiler's C++ library supports it
# or not.
# This can be removed once the minimum supported gcc version is 4.9
CHECK_CXX_SOURCE_COMPILES(
"
#include <regex>
int main() {
auto regex_iter_end = std::sregex_iterator();
(void)regex_iter_end;
return 0;
}
" US_HAVE_REGEX)

#-----------------------------------------------------------------------------
# System Information
#-----------------------------------------------------------------------------

include(TestBigEndian)

test_big_endian(US_BIG_ENDIAN)

if(NOT US_BIG_ENDIAN)
  set(US_LITTLE_ENDIAN 1)
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

set(global_config_h_file "${PROJECT_BINARY_DIR}/include/${PROJECT_NAME_LOWER}/GlobalConfig.h")
configure_file(${US_CMAKE_DIR}/GlobalConfig.h.in ${global_config_h_file})

include_directories(${US_INCLUDE_DIRS})

add_subdirectory(tools/rc)

add_subdirectory(framework)
add_subdirectory(httpservice)
add_subdirectory(webconsole)
add_subdirectory(shellservice)

add_subdirectory(tools/shell)

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
    ${US_BUNDLE_INIT_TEMPLATE}
    ${US_CMAKE_RESOURCE_DEPENDENCIES_CPP}
    ${US_CMAKE_DIR}/usFunctionGenerateBundleInit.cmake
    ${US_CMAKE_DIR}/usFunctionAddResources.cmake
    ${US_CMAKE_DIR}/usFunctionEmbedResources.cmake
    ${US_CMAKE_DIR}/usFunctionGetResourceSource.cmake
    ${US_CMAKE_DIR}/usFunctionCheckResourceLinking.cmake
    ${US_CMAKE_DIR}/usFunctionCheckCompilerFlags.cmake
    )

  install(FILES ${_install_cmake_scripts}
          DESTINATION ${AUXILIARY_CMAKE_INSTALL_DIR})

  install(FILES ${global_config_h_file}
          DESTINATION ${HEADER_INSTALL_DIR}/${PROJECT_NAME_LOWER})

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
