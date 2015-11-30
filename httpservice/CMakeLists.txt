set(_srcs
  ../third_party/civetweb/civetweb.c
  ../third_party/civetweb/CivetServer.cpp

  src/usHttpConstants.cpp
  src/usHttpServlet.cpp
  src/usServletContainer.cpp
  src/usServletContext.cpp
  src/usHttpOutputStreamBuffer.cpp
  src/usHttpServletRequest.cpp
  src/usHttpServletResponse.cpp
  src/usServletConfig.cpp
  src/usServletContainer.cpp
)

if(MSVC)
  set_property(
    SOURCE ../third_party/civetweb/civetweb.c APPEND_STRING
    PROPERTY COMPILE_FLAGS " /wd4267 /wd4311 /wd4312 /wd4996"
	)
endif()

set(_private_headers
  src/usHttpOutputStreamBuffer_p.h
  src/usHttpServlet_p.h
  src/usHttpServletRequest_p.h
  src/usHttpServletResponse_p.h
  src/usServletConfig_p.h
)

set(_public_headers
  include/usHttpConstants.h
  include/usHttpServlet.h
  include/usServletContext.h
  include/usHttpServletRequest.h
  include/usHttpServletResponse.h
  include/usServletConfig.h
  include/usServletContainer.h
)

set(compile_definitions USE_WEBSOCKET)
if(WIN32)
  list(APPEND compile_definitions WIN32)
elseif(LINUX)
  list(APPEND compile_definitions LINUX)
elseif(APPLE)
  list(APPEND compile_definitions DARWIN)
endif()

set_property(SOURCE src/civetweb/civetweb.c src/civetweb/CivetServer.cpp
  APPEND PROPERTY COMPILE_DEFINITIONS ${compile_definitions})
set_property(SOURCE src/civetweb/civetweb.c src/civetweb/CivetServer.cpp
  APPEND PROPERTY COMPILE_DEFINITIONS_DEBUG DEBUG_ENABLED)
set_property(SOURCE src/civetweb/civetweb.c src/civetweb/CivetServer.cpp
  APPEND PROPERTY COMPILE_DEFINITIONS_RELEASE NDEBUG)

# Apple Clang complains that -pthread is unused. To silence this linker error,
# don't add it when building for Mac.
# Another option would be to use "-Wno-error=unused-command-line-argument" instead.
if(NOT APPLE)
  usFunctionCheckCompilerFlags(-pthread CMAKE_CXX_FLAGS)
endif()

usMacroCreateBundle(HttpService
  VERSION "0.99.0"
  DEPENDS Core
  INTERNAL_INCLUDE_DIRS src
  PUBLIC_HEADERS ${_public_headers}
  PRIVATE_HEADERS ${_private_headers}
  SOURCES ${_srcs}
  RESOURCES manifest.json
)
