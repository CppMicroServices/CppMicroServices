
enable_language(C)

include_directories(${US_INCLUDE_DIRS})

add_definitions(-DUS_RCC_EXECUTABLE_NAME=\"${US_RCC_EXECUTABLE_NAME}\")

set(srcs usResourceCompiler.c ../third_party/miniz.c)

add_executable(${US_RCC_EXECUTABLE_NAME} ${srcs})
if(WIN32)
  target_link_libraries(${US_RCC_EXECUTABLE_NAME} Shlwapi)
endif()

set_property(
  TARGET ${US_RCC_EXECUTABLE_NAME} APPEND PROPERTY
  COMPILE_DEFINITIONS "MINIZ_NO_ARCHIVE_READING_API;MINIZ_NO_ZLIB_COMPATIBLE_NAMES"
  )

if(NOT US_NO_INSTALL)
  install(TARGETS ${US_RCC_EXECUTABLE_NAME}
          EXPORT ${PROJECT_NAME}Targets
          FRAMEWORK DESTINATION . ${US_SDK_INSTALL_COMPONENT}
          RUNTIME DESTINATION bin ${US_SDK_INSTALL_COMPONENT})
endif()
