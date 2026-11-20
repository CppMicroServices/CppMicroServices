# sources and headers
include(${CMAKE_CURRENT_SOURCE_DIR}/src/CMakeLists.txt)
include(${CMAKE_CURRENT_SOURCE_DIR}/include/CMakeLists.txt)

set(_framework_srcs )
foreach(_src ${_srcs})
  list(APPEND _framework_srcs ${CMAKE_CURRENT_SOURCE_DIR}/src/${_src})
endforeach()

set(_framework_private_headers)
foreach(_header ${_private_headers})
  list(APPEND _framework_private_headers ${CMAKE_CURRENT_SOURCE_DIR}/src/${_header})
endforeach()

set(_framework_public_headers )
foreach(_header ${_public_headers})
  list(APPEND _framework_public_headers ${CMAKE_CURRENT_SOURCE_DIR}/include/${_header})
endforeach()

# link libraries for the CppMicroServices lib
set(_link_libraries )
if(UNIX)
  list(APPEND _link_libraries dl)
endif()
if(WIN32)
  list(APPEND _link_libraries shlwapi.lib)
endif()
if(CMAKE_THREAD_LIBS_INIT)
  list(APPEND _link_libraries ${CMAKE_THREAD_LIBS_INIT})
endif()

#-----------------------------------------------------------------------------
# Framework object library
#
# The framework sources are compiled exactly once, into this OBJECT library.
# The shipping CppMicroServices library is linked from these objects, and the
# white-box unit tests (test/unit) link the very same objects directly into
# the test executable. This gives the unit tests access to private framework
# classes (CoreBundleContext, BundleRegistry, BundleManifest, ...) without
# exporting any of them from the shipping library. This mirrors the
# DeclarativeServicesObjs / ConfigurationAdminObjs pattern used by the
# compendium bundles.
#-----------------------------------------------------------------------------

add_library(CppMicroServicesObjs OBJECT ${_framework_srcs} ${_framework_private_headers})
set_property(TARGET CppMicroServicesObjs PROPERTY POSITION_INDEPENDENT_CODE 1)

target_compile_features(CppMicroServicesObjs PUBLIC cxx_variadic_templates cxx_nullptr)

# The private source directories are deliberately part of this target's
# *usage requirements*: the only consumers of the object library are the
# shipping library and white-box tests, both of which need them.
target_include_directories(CppMicroServicesObjs
  PUBLIC
    ${CMAKE_CURRENT_SOURCE_DIR}/include
    ${CMAKE_CURRENT_BINARY_DIR}/include
    ${CppMicroServices_BINARY_DIR}/include
    ${CMAKE_CURRENT_SOURCE_DIR}/src/util
    ${CMAKE_CURRENT_SOURCE_DIR}/src/service
    ${CMAKE_CURRENT_SOURCE_DIR}/src/bundle
  PRIVATE
    $<TARGET_PROPERTY:util,INCLUDE_DIRECTORIES>
  )

# CppMicroServices_EXPORTS is the symbol CMake would define automatically for
# sources of the CppMicroServices SHARED target. Object libraries do not get
# it, so define it here. It is PUBLIC so that anything linking these objects
# directly (i.e. the unit test executable) sees US_Framework_EXPORT resolve
# to the local definitions instead of to dllimport.
target_compile_definitions(CppMicroServicesObjs
  PUBLIC
    CppMicroServices_EXPORTS
  PRIVATE
    US_BUNDLE_NAME=system_bundle
    MINIZ_NO_ARCHIVE_WRITING_API
    MINIZ_NO_ZLIB_COMPATIBLE_NAMES
  )

# miniz is PUBLIC because a private header (BundleResourceContainer.h)
# includes miniz.h. This only affects consumers of the object library (the
# unit tests); the shipping library keeps miniz private below.
target_link_libraries(CppMicroServicesObjs
  PUBLIC ${_link_libraries} miniz::miniz
  PRIVATE rapidjson::rapidjson
  )

usMacroCreateBundle(Framework
  VERSION "${_version}"
  TARGET CppMicroServices
  SYMBOLIC_NAME system_bundle
  SKIP_INIT # we do it manually
  EMBED_RESOURCE_METHOD LINK
  PRIVATE_INCLUDE_DIRS src/util src/service src/bundle
  LINK_LIBRARIES ${_link_libraries}
  SOURCES $<TARGET_OBJECTS:CppMicroServicesObjs>
  PUBLIC_HEADERS ${_framework_public_headers}
  PRIVATE_HEADERS ${_framework_private_headers}
  BINARY_RESOURCES manifest.json
)

# Private implementation deps — not propagated to framework consumers.
target_link_libraries(${PROJECT_TARGET} PRIVATE miniz::miniz rapidjson::rapidjson)
if(BUILD_SHARED_LIBS AND US_USE_SYSTEM_MINIZ AND NOT US_MINIZ_SHARED)
  if(APPLE)
    target_link_options(${PROJECT_TARGET} PRIVATE "-Wl,-hidden-lminiz")
  elseif(UNIX)
    target_link_options(${PROJECT_TARGET} PRIVATE "LINKER:--exclude-libs,ALL")
  endif()
endif()

set_property(
  TARGET ${PROJECT_TARGET} APPEND PROPERTY
  COMPILE_DEFINITIONS "MINIZ_NO_ARCHIVE_WRITING_API;MINIZ_NO_ZLIB_COMPATIBLE_NAMES"
)

if (US_ENABLE_ASAN AND US_ASAN_USER_DLL)
  add_custom_command(TARGET CppMicroServices POST_BUILD COMMAND ${CMAKE_COMMAND} -E copy ${US_ASAN_USER_DLL} ${US_RUNTIME_OUTPUT_DIRECTORY}/.)
endif()
