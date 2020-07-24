# soruces and headers
add_subdirectory(src)

set(_ca_srcs)
foreach(_src ${_srcs})
  list(APPEND _ca_srcs ${CMAKE_CURRENT_SOURCE_DIR}/src/${_src})
endforeach()

set(_ca_private_headers)
foreach(_header ${_private_headers})
  list(APPEND _ca_private_headers ${CMAKE_CURRENT_SOURCE_DIR}/src/${_header})
endforeach()

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

configure_file(${CMAKE_CURRENT_SOURCE_DIR}/resources/manifest.json.in
               ${CMAKE_CURRENT_BINARY_DIR}/resources/manifest.json)

if(MINGW)
  # silence ignored attributes warnings
  add_compile_options(-Wno-attributes)
endif()

usMacroCreateBundle(ConfigurationAdmin
  VERSION "1.0.0"
  DEPENDS Framework
  TARGET ConfigurationAdmin
  SYMBOLIC_NAME configuration_admin
  EMBED_RESOURCE_METHOD LINK
  PRIVATE_INCLUDE_DIRS src/util src/service src/bundle ../third_party
  LINK_LIBRARIES ${_link_libraries} usLogService
  PUBLIC_HEADERS ${_ca_public_headers}
  PRIVATE_HEADERS ${_ca_private_headers}
  SOURCES $<TARGET_OBJECTS:ConfigurationAdminObjs> src/CMActivator.cpp
  BINARY_RESOURCES manifest.json
)

target_include_directories(ConfigurationAdmin PRIVATE
  ${CMAKE_CURRENT_SOURCE_DIR}/include
  ${CppMicroServices_BINARY_DIR}/include
  ${CppMicroServices_SOURCE_DIR}/framework/include
  ${CppMicroServices_BINARY_DIR}/framework/include
  ${CppMicroServices_SOURCE_DIR}/compendium/LogService/include
  ${CppMicroServices_BINARY_DIR}/compendium/LogService/include
  ${CppMicroServices_SOURCE_DIR}/compendium/CM/include
  ${CppMicroServices_BINARY_DIR}/compendium/CM/include
  ${CppMicroServices_SOURCE_DIR}/third_party/googletest/googletest/include
  ${CppMicroServices_SOURCE_DIR}/third_party/googletest/googlemock/include
  )