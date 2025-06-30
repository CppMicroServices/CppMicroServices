# sources and headers
find_package(spdlog REQUIRED)
set(_srcs
  src/LogServiceImpl.cpp
  src/LoggerFactoryImpl.cpp
  src/LoggerImpl.cpp
  )

set(_hdrs
  src/Activator.hpp
  src/LogServiceImpl.hpp
  src/LoggerFactoryImpl.hpp
  src/LoggerImpl.hpp
  )

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

# This is to suppress a warning from MSVC regarding the use of 
# stdext::checked_array_iterator in spdlog. If and when spdlog upgrades 
# to a future release of c++, std::span is available in c++20 and is not deprecated
if(MSVC)
  add_compile_definitions(_SILENCE_STDEXT_ARR_ITERS_DEPRECATION_WARNING)
endif()

if(MINGW)
  # silence ignored attributes warnings
  add_compile_options(-Wno-attributes)
  # silence dangling references warning due to false positives
  add_compile_options(-Wno-dangling-reference)
endif()

usMacroCreateBundle(LogServiceImpl
  VERSION "1.0.0"
  DEPENDS Framework
  TARGET LogService
  SYMBOLIC_NAME log_service
  EMBED_RESOURCE_METHOD LINK
  PRIVATE_INCLUDE_DIRS src/util src/service src/bundle ../third_party
  LINK_LIBRARIES ${_link_libraries}
  PUBLIC_HEADERS ${_public_headers}
  PRIVATE_HEADERS ${_hdrs}
  SOURCES ${_srcs} src/Activator.cpp
  BINARY_RESOURCES manifest.json
  )


# find_or_add_third_party(spdlog spdlog spdlog/spdlog.h)

# message(FATAL_ERROR "spdlog headers are ${SPDLOG_INCLUDE_DIR}")
target_include_directories(LogService PRIVATE
  ${CMAKE_CURRENT_SOURCE_DIR}/src
  ${CppMicroServices_BINARY_DIR}/include
  ${CppMicroServices_SOURCE_DIR}/framework/include
  ${CppMicroServices_BINARY_DIR}/framework/include
  ${CppMicroServices_SOURCE_DIR}/compendium/LogService/include
  ${CppMicroServices_BINARY_DIR}/compendium/LogService/include
  ${CppMicroServices_SOURCE_DIR}/third_party/googletest/googletest/include
  ${CppMicroServices_SOURCE_DIR}/third_party/googletest/googlemock/include
  
  )

target_link_libraries(LogService PRIVATE spdlog::spdlog)

get_target_property(tmp_includes LogService INCLUDE_DIRECTORIES)
message(STATUS "LogService includes: ${tmp_includes}")

if(APPLE)
  if(CMAKE_CXX_COMPILER_ID STREQUAL "AppleClang" AND CMAKE_CXX_COMPILER_VERSION VERSION_LESS "8.0.0")
    target_compile_definitions(LogService PRIVATE SPDLOG_NO_TLS)
  endif()
endif()
