# sources and headers
add_subdirectory(src)

set(_ea_srcs)
foreach(_src ${_srcs})
  list(APPEND _ea_srcs ${CMAKE_CURRENT_SOURCE_DIR}/src/${_src})
endforeach()

set(_ea_private_headers)
foreach(_header ${_private_headers})
  list(APPEND _ea_private_headers ${CMAKE_CURRENT_SOURCE_DIR}/src/${_header})
endforeach()

set(_link_libraries)
if (UNIX)
  list(APPEND _link_libraries dl)
elseif (WIN32)
  list(APPEND _link_libraries shlwapi.lib)
endif()

if (CMAKE_THREAD_LIBS_INIT)
  list(APPEND _link_libraries ${CMAKE_THREAD_LIBS_INIT})
endif()

configure_file(${CMAKE_CURRENT_SOURCE_DIR}/resources/manifest.json.in
               ${CMAKE_CURRENT_BINARY_DIR}/resources/manifest.json)

if (MINGW)
  # silence ignored attributes warnings
  add_compile_options(-Wno-attributes)

  # needed for boost::asio::thread_pool
  list(APPEND _link_libraries ws2_32.lib)
endif()

add_compile_definitions(BOOST_DATE_TIME_NO_LIB)
add_compile_definitions(BOOST_REGEX_NO_LIB)

usMacroCreateBundle(EventAdmin
  VERSION "0.1.0"
  DEPENDS Framework
  TARGET EventAdmin
  SYMBOLIC_NAME event_admin
  EMBED_RESOURCE_METHOD LINK
  PRIVATE_INCLUDE_DIRS src/util src/service src/bundle ../third_party
  LINK_LIBRARIES ${_link_libraries} LogService usAsyncWorkService usEM
  PUBLIC_HEADERS ${_ea_public_headers}
  PRIVATE_HEADERS ${_ea_private_headers}
  SOURCES $<TARGET_OBJECTS:EventAdminObjs> src/EMActivator.cpp
  BINARY_RESOURCES manifest.json
)

target_include_directories(EventAdmin PRIVATE
  ${CMAKE_CURRENT_SOURCE_DIR}/src
  ${CppMicroServices_BINARY_DIR}/include
  ${CppMicroServices_SOURCE_DIR}/framework/include
  ${CppMicroServices_BINARY_DIR}/framework/include
  ${CppMicroServices_SOURCE_DIR}/compendium/LogService/include
  ${CppMicroServices_BINARY_DIR}/compendium/LogService/include
  ${CppMicroServices_SOURCE_DIR}/compendium/AsyncWorkService/include
  ${CppMicroServices_BINARY_DIR}/compendium/AsyncWorkService/include
  ${CppMicroServices_SOURCE_DIR}/compendium/EM/include
  ${CppMicroServices_BINARY_DIR}/compendium/EM/include
  ${CppMicroServices_SOURCE_DIR}/third_party/googletest/googletest/include
  ${CppMicroServices_SOURCE_DIR}/third_party/googletest/googlemock/include
)

# There are warnings in the boost asio headers which are flagged as errors. Include the boost
# asio headers as system headers to ignore these warnings and not treat them as errors.
include_directories(SYSTEM ${CppMicroServices_SOURCE_DIR}/third_party/boost/include)
