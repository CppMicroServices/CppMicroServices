set(_srcs
  src/usShellActivator.cpp
  src/usShellService.cpp

  ../third_party/tinyscheme/scheme.c
)

set(_private_headers
)

set(_public_headers
  include/usShellService.h
)

set(_resources
  usShellProcedures.scm
)

add_definitions(-DSTANDALONE=0 -DUSE_INTERFACE=1)

configure_file(${CMAKE_CURRENT_SOURCE_DIR}/../third_party/tinyscheme/init.scm
  ${CMAKE_CURRENT_BINARY_DIR}/resources/tinyscheme/init.scm COPYONLY)

usMacroCreateModule(ShellService
  VERSION "0.1.0"
  DEPENDS Core
  PUBLIC_HEADERS ${_public_headers}
  PRIVATE_HEADERS ${_private_headers}
  INTERNAL_INCLUDE_DIRS ../third_party/tinyscheme
  SOURCES ${_srcs}
  RESOURCES ${_resources}
  BINARY_RESOURCES tinyscheme/init.scm
)

