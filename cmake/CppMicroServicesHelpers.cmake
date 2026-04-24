# CppMicroServicesHelpers.cmake
#
# Included via cmake_build_modules by the Conan-generated package config.
# Replicates the bundle-authoring environment that the project's own
# CppMicroServicesConfig.cmake provides: helper functions, code-generation
# template paths, and the usResourceCompiler imported executable target.
#
# CMAKE_CURRENT_LIST_DIR resolves to this file's own directory inside the
# Conan package cache, so all paths below are fully relocatable.

get_filename_component(_us_cmake_dir "${CMAKE_CURRENT_LIST_DIR}" ABSOLUTE)
# cmake dir is <pkg>/share/cppmicroservices<M>/cmake/ — three levels up is <pkg>/
get_filename_component(_us_pkg_root  "${_us_cmake_dir}/../../.." ABSOLUTE)

# Code-generation template paths
set(US_BUNDLE_INIT_TEMPLATE            "${_us_cmake_dir}/BundleInit.cpp")
set(US_CMAKE_RESOURCE_DEPENDENCIES_CPP "${_us_cmake_dir}/CMakeResourceDependencies.cpp")
if(EXISTS "${_us_cmake_dir}/cppmicroservices_resources.rc.in")
    set(US_RESOURCE_RC_TEMPLATE "${_us_cmake_dir}/cppmicroservices_resources.rc.in")
endif()

# Resource compiler imported target
# The installed binary is named usResourceCompiler<major>; the CMake target
# name used by the usFunction* helpers is US_RCC_EXECUTABLE_TARGET.
set(US_RCC_EXECUTABLE_TARGET usResourceCompiler)
if(NOT TARGET usResourceCompiler)
    find_program(_us_rcc_exe
        NAMES usResourceCompiler3
        HINTS "${_us_pkg_root}/bin"
        NO_DEFAULT_PATH)
    if(_us_rcc_exe)
        add_executable(usResourceCompiler IMPORTED GLOBAL)
        set_property(TARGET usResourceCompiler PROPERTY IMPORTED_LOCATION "${_us_rcc_exe}")
    else()
        message(WARNING
            "CppMicroServices: usResourceCompiler3 not found in ${_us_pkg_root}/bin — "
            "bundle resource embedding (usFunctionAddResources) will not work.")
    endif()
    unset(_us_rcc_exe CACHE)
endif()

# Framework resource zip
# usFunctionAddResources needs to know where the framework's own zip lives.
# Mirror the logic in CppMicroServicesConfig.cmake.in lines 151-157.
if(TARGET CppMicroServices)
    get_target_property(_existing CppMicroServices _us_resource_zips)
    if(NOT _existing)
        set_target_properties(CppMicroServices PROPERTIES
            _us_resource_zips "${_us_pkg_root}/lib/CppMicroServices/res_0.zip")
    endif()
    unset(_existing)
endif()

# Helper functions
include("${_us_cmake_dir}/usFunctionCheckCompilerFlags.cmake")
include("${_us_cmake_dir}/usFunctionCheckResourceLinking.cmake")
include("${_us_cmake_dir}/usFunctionGenerateBundleInit.cmake")
include("${_us_cmake_dir}/usFunctionAddResources.cmake")
include("${_us_cmake_dir}/usFunctionEmbedResources.cmake")
include("${_us_cmake_dir}/usFunctionGetResourceSource.cmake")
include("${_us_cmake_dir}/usFunctionBoostPath.cmake")

usFunctionCheckResourceLinking()

# Legacy aggregate variables
if(NOT DEFINED US_LIBRARIES)
    set(US_LIBRARIES CppMicroServices)
endif()

unset(_us_cmake_dir)
unset(_us_pkg_root)
