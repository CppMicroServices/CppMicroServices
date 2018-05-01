
enable_language(C)

include_directories(../../third_party)

add_definitions(-DUS_RCC_EXECUTABLE_NAME=\"${US_RCC_EXECUTABLE_OUTPUT_NAME}\")

set(srcs ResourceCompiler.cpp ../../third_party/miniz.c ../../third_party/jsoncpp.cpp)

# When cross-compiling, import the host executable targets from a file
if(CMAKE_CROSSCOMPILING)
  # See https://gitlab.kitware.com/cmake/community/wikis/doc/cmake/CrossCompiling
  include(${IMPORT_EXECUTABLES})

else()
  add_executable(${US_RCC_EXECUTABLE_TARGET} ${srcs})
  set_property(TARGET ${US_RCC_EXECUTABLE_TARGET} PROPERTY OUTPUT_NAME ${US_RCC_EXECUTABLE_OUTPUT_NAME})
  if(WIN32)
    target_link_libraries(${US_RCC_EXECUTABLE_TARGET} Shlwapi)
  endif()

  set_property(
  TARGET ${US_RCC_EXECUTABLE_TARGET} APPEND PROPERTY
  COMPILE_DEFINITIONS "MINIZ_NO_ARCHIVE_READING_API;MINIZ_NO_ZLIB_COMPATIBLE_NAMES"
  )

  if(NOT US_NO_INSTALL)
    # Only install the target if we are not cross-compiling, because the target is only built during the host build
    install(TARGETS ${US_RCC_EXECUTABLE_TARGET}
            EXPORT ${PROJECT_NAME}Targets
            FRAMEWORK DESTINATION . ${US_SDK_INSTALL_COMPONENT}
            RUNTIME DESTINATION bin ${US_SDK_INSTALL_COMPONENT})
  endif()

  # Export the host usResourceCompiler target to a file so it can be imported when cross-compiling
  export( TARGETS ${US_RCC_EXECUTABLE_TARGET} FILE ${IMPORT_EXECUTABLES} )

endif()
