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

set(US_SHELL_EXECUTABLE_NAME usShell${US_GLOBAL_VERSION_SUFFIX})
add_executable(${US_SHELL_EXECUTABLE_NAME} ${srcs})
target_link_libraries(${US_SHELL_EXECUTABLE_NAME} ${US_LIBRARIES})

if(NOT US_NO_INSTALL)
  install(TARGETS ${US_SHELL_EXECUTABLE_NAME}
          EXPORT ${PROJECT_NAME}Targets
          FRAMEWORK DESTINATION . ${US_SDK_INSTALL_COMPONENT}
          RUNTIME DESTINATION bin ${US_SDK_INSTALL_COMPONENT})
endif()
