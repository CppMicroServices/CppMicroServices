find_package(CppMicroServices COMPONENTS ShellService REQUIRED)

include_directories(${US_INCLUDE_DIRS})

add_definitions(-DUS_MODULE_NAME=main)

set(srcs
  usShell.cpp
  ../../third_party/linenoise.c
)
add_executable(usShell ${srcs})
target_link_libraries(usShell ${US_LIBRARIES})
