# sources and headers
include(${CMAKE_CURRENT_SOURCE_DIR}/src/CMakeLists.txt)
include(${CMAKE_CURRENT_SOURCE_DIR}/include/CMakeLists.txt)

set(_core_srcs )
foreach(_src ${_srcs})
  list(APPEND _core_srcs ${CMAKE_CURRENT_SOURCE_DIR}/src/${_src})
endforeach()

set(_core_private_headers)
foreach(_header ${_private_headers})
  list(APPEND _core_private_headers ${CMAKE_CURRENT_SOURCE_DIR}/src/${_header})
endforeach()

set(_core_public_headers )
foreach(_header ${_public_headers})
  list(APPEND _core_public_headers ${CMAKE_CURRENT_SOURCE_DIR}/include/${_header})
endforeach()

# link libraries for the CppMicroServices lib
set(_link_libraries )
if(UNIX)
  list(APPEND _link_libraries dl)
endif()

# Configure the modules manifest.json file
configure_file(${CMAKE_CURRENT_SOURCE_DIR}/resources/manifest.json.in
               ${CMAKE_CURRENT_BINARY_DIR}/resources/manifest.json)

usMacroCreateModule(Core
  SKIP_INIT # we initialize the module in usModuleRegistry.cpp ourselves
  VERSION "2.99.0"
  TARGET CppMicroServices
  INTERNAL_INCLUDE_DIRS src/util src/service src/module
  LINK_LIBRARIES ${_link_libraries}
  SOURCES ${_core_srcs}
  PUBLIC_HEADERS ${_core_public_headers}
  PRIVATE_HEADERS ${_core_private_headers}
  BINARY_RESOURCES manifest.json
)

set_property(
  TARGET CppMicroServices APPEND PROPERTY
  COMPILE_DEFINITIONS "MINIZ_NO_ARCHIVE_WRITING_API;MINIZ_NO_ZLIB_COMPATIBLE_NAMES"
  )
