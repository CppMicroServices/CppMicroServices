find_package(CppMicroServices COMPONENTS ShellService REQUIRED)

include_directories(${US_INCLUDE_DIRS})

set(srcs
  Shell.cpp
  ../../third_party/linenoise.c
)

if(MSVC)
  set_property(
    SOURCE ../../third_party/linenoise.c APPEND_STRING
    PROPERTY COMPILE_FLAGS " /wd4996"
  )
endif()

add_executable(Shell ${srcs})
target_link_libraries(Shell ${US_LIBRARIES})
