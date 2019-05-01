find_package(CppMicroServices COMPONENTS ShellService REQUIRED)

include_directories(../../third_party)

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

set(US_SHELL_EXECUTABLE_TARGET usShell)
set(US_SHELL_EXECUTABLE_OUTPUT_NAME ${US_SHELL_EXECUTABLE_TARGET}${US_GLOBAL_VERSION_SUFFIX})

add_executable(${US_SHELL_EXECUTABLE_TARGET} ${srcs})
set_property(TARGET ${US_SHELL_EXECUTABLE_TARGET} PROPERTY OUTPUT_NAME ${US_SHELL_EXECUTABLE_OUTPUT_NAME})

target_link_libraries(${US_SHELL_EXECUTABLE_TARGET} ${US_LIBRARIES})

if(NOT US_NO_INSTALL)
  install(TARGETS ${US_SHELL_EXECUTABLE_TARGET}
          EXPORT ${PROJECT_NAME}Targets
          FRAMEWORK DESTINATION . ${US_SDK_INSTALL_COMPONENT}
          RUNTIME DESTINATION ${TOOLS_INSTALL_DIR} ${US_SDK_INSTALL_COMPONENT})
endif()
