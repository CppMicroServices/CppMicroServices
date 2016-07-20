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
else()
  set_property(
    SOURCE ../third_party/civetweb/CivetServer.cpp APPEND_STRING
    PROPERTY COMPILE_FLAGS " -Wno-old-style-cast"
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

usFunctionCheckCompilerFlags(-pthread CMAKE_CXX_FLAGS)


usMacroCreateBundle(HttpService
  VERSION "0.99.0"
  DEPENDS Core
  INTERNAL_INCLUDE_DIRS src
  PUBLIC_HEADERS ${_public_headers}
  PRIVATE_HEADERS ${_private_headers}
  SOURCES ${_srcs}
  RESOURCES manifest.json
)

# Needed for clock_gettime with glibc < 2.17
if(UNIX AND NOT APPLE)
  target_link_libraries(usHttpService rt)
endif()

