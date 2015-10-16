set(_srcs
  src/usShellActivator.cpp
  src/usShellService.cpp

  ../third_party/tinyscheme/scheme.c
)

if(MSVC)
  set_property(
    SOURCE ../third_party/tinyscheme/scheme.c APPEND_STRING
    PROPERTY COMPILE_FLAGS " /wd4311 /wd4312 /wd4996"
  )
endif()

usFunctionCheckCompilerFlags(-Wno-switch _no_switch_flag)
if(_no_switch_flag)
  set_property(
    SOURCE ../third_party/tinyscheme/scheme.c APPEND_STRING
    PROPERTY COMPILE_FLAGS " ${_no_switch_flag}"
  )
endif()


set(_private_headers
)

set(_public_headers
  include/usShellService.h
)

set(_resources
  manifest.json
  usShellProcedures.scm
)

add_definitions(-DSTANDALONE=0 -DUSE_INTERFACE=1)

configure_file(${CMAKE_CURRENT_SOURCE_DIR}/../third_party/tinyscheme/init.scm
  ${CMAKE_CURRENT_BINARY_DIR}/resources/tinyscheme/init.scm COPYONLY)

usMacroCreateBundle(ShellService
  VERSION "0.1.0"
  DEPENDS Core
  PUBLIC_HEADERS ${_public_headers}
  PRIVATE_HEADERS ${_private_headers}
  INTERNAL_INCLUDE_DIRS ../third_party/tinyscheme
  SOURCES ${_srcs}
  RESOURCES ${_resources}
  BINARY_RESOURCES tinyscheme/init.scm
)

