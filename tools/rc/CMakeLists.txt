# Temporarily disable enable exports for executables
set(CMAKE_ENABLE_EXPORTS OFF)

add_definitions(-DUS_RCC_EXECUTABLE_NAME=\"${US_RCC_EXECUTABLE_OUTPUT_NAME}\")

if(US_COMPILER_CLANG OR US_COMPILER_APPLE_CLANG)
  add_compile_options(-Wno-error=unneeded-internal-declaration)
endif()

if(NOT APPLE AND US_COMPILER_GNU_ARM AND CMAKE_SYSTEM_PROCESSOR MATCHES "aarch64|ARM64|arm64")
  add_compile_options(-Wno-error=maybe-uninitialized)
endif()

set(srcs ResourceCompiler.cpp)

if(NOT US_USE_SYSTEM_MINIZ)
  list(APPEND srcs ../../third_party/miniz.c)
endif()

add_executable(${US_RCC_EXECUTABLE_TARGET} ${srcs})

target_include_directories(${US_RCC_EXECUTABLE_TARGET} PRIVATE ../../third_party ${CppMicroServices_SOURCE_DIR}/util/include)

set_property(TARGET ${US_RCC_EXECUTABLE_TARGET} PROPERTY OUTPUT_NAME ${US_RCC_EXECUTABLE_OUTPUT_NAME})

target_link_libraries(${US_RCC_EXECUTABLE_TARGET} PRIVATE
    nowide::nowide
    CLI11::CLI11
    rapidjson::rapidjson
    miniz::miniz)

if(WIN32)
    target_link_libraries(${US_RCC_EXECUTABLE_TARGET} PRIVATE Shlwapi)
endif()

target_compile_definitions(${US_RCC_EXECUTABLE_TARGET} PRIVATE -DMINIZ_NO_ZLIB_COMPATIBLE_NAMES)

if(NOT US_NO_INSTALL)
    install(TARGETS ${US_RCC_EXECUTABLE_TARGET}
            EXPORT ${PROJECT_NAME}Targets
            FRAMEWORK DESTINATION . ${US_SDK_INSTALL_COMPONENT}
            RUNTIME DESTINATION ${TOOLS_INSTALL_DIR} ${US_SDK_INSTALL_COMPONENT})
endif()

if(CMAKE_CROSSCOMPILING)
  # When cross-compiling, import the host machine executable from a file
  # See https://gitlab.kitware.com/cmake/community/wikis/doc/cmake/CrossCompiling
  include(${IMPORT_EXECUTABLES})
else()
  # Export the host usResourceCompiler target to a file so it can be imported when cross-compiling.
  # Use the NAMESPACE option of EXPORT() to get a different target name for ${US_RCC_EXECUTABLE_TARGET}
  # when exporting.
  # Doing this allows the host built usResourceCompiler to be used when cross-compiling and allows
  # the usResourceCompiler to be built and installed for the target machine.
  export( TARGETS ${US_RCC_EXECUTABLE_TARGET} FILE ${IMPORT_EXECUTABLES} NAMESPACE native-)
endif()

# Re-enable enable exports for executables after building usResourceCompiler
set(CMAKE_ENABLE_EXPORTS ON)

# When deterministic bundle builds are enabled, compile miniz without time support so that
# all zip entry timestamps are zeroed (giving deterministic output regardless of file mtimes).
# MINIZ_NO_TIME disables mz_zip_time_t_to_dos_time / mz_zip_dos_to_time_t in miniz, leaving
# dos_time and dos_date at their zero-initialized values for every entry written.
# US_USE_DETERMINISTIC_BUNDLE_BUILDS activates the locale-setting and sorting logic in
# ResourceCompiler.cpp that is also required for deterministic output.
if (US_USE_DETERMINISTIC_BUNDLE_BUILDS)
  if(US_USE_SYSTEM_MINIZ)
    message(FATAL_ERROR "US_USE_DETERMINISTIC_BUNDLE_BUILDS requires bundled miniz (MINIZ_NO_TIME). "
                       "Set US_USE_SYSTEM_MINIZ=OFF or disable deterministic builds.")
  endif()
  target_compile_definitions(${US_RCC_EXECUTABLE_TARGET} PRIVATE
                             MINIZ_NO_TIME
                             US_USE_DETERMINISTIC_BUNDLE_BUILDS)

  add_test(NAME Deterministic_Zip_Files_Test
         WORKING_DIRECTORY ${CMAKE_CURRENT_BINARY_DIR}
         COMMAND ${CMAKE_CURRENT_SOURCE_DIR}/test/basic_deterministic_zip_test.$<IF:$<BOOL:${MSVC}>,bat,sh> ${CMAKE_BINARY_DIR}/bin/${US_RCC_EXECUTABLE_OUTPUT_NAME}
         )
endif()
