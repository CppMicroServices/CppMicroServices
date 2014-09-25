set(_srcs
  src/usShellActivator.cpp
  src/usShellService.cpp

  ../third_party/tinyscheme/scheme.c

  usShellService_resources.cpp
)

set(_private_headers
)

set(_public_headers
  include/usShellService
)

set(_resources

)

add_definitions(-DSTANDALONE=0 -DUSE_INTERFACE=1)

usMacroCreateModule(ShellService
  VERSION "0.1.0"
  DEPENDS Core
  PUBLIC_HEADERS ${_public_headers}
  PRIVATE_HEADERS ${_private_headers}
  INTERNAL_INCLUDE_DIRS ../third_party/tinyscheme
  SOURCES ${_srcs}
)

usFunctionAddResources(TARGET ${ShellService_TARGET}
  FILES tinyscheme/init.scm
  WORKING_DIRECTORY ../third_party
)
