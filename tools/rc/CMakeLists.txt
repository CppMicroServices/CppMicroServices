enable_language(C)

# Temporarily disable enable exports for executables
set(CMAKE_ENABLE_EXPORTS OFF)

include_directories(../../third_party)

add_definitions(-DUS_RCC_EXECUTABLE_NAME=\"${US_RCC_EXECUTABLE_OUTPUT_NAME}\")

if(US_COMPILER_CLANG OR US_COMPILER_APPLE_CLANG)
  add_compile_options(-Wno-error=unneeded-internal-declaration)
endif()

set(srcs ResourceCompiler.cpp ../../third_party/miniz.c ../../third_party/jsoncpp.cpp)

if(MINGW)
  # Suppress warnings for specific files
  set_source_files_properties(
    ../../third_party/jsoncpp.cpp
    PROPERTIES COMPILE_FLAGS -Wno-maybe-uninitialized
  )
endif()

if(NOT APPLE AND US_COMPILER_GNU_ARM AND CMAKE_SYSTEM_PROCESSOR MATCHES "aarch64|ARM64|arm64")
  add_compile_options(-Wno-error=maybe-uninitialized)
endif()

add_executable(${US_RCC_EXECUTABLE_TARGET} ${srcs})

if(US_COMPILER_CLANG OR US_COMPILER_APPLE_CLANG OR US_COMPILER_GNU)
  set_property(SOURCE ResourceCompiler.cpp 
               APPEND PROPERTY COMPILE_OPTIONS 
               -Wno-old-style-cast -Wno-non-virtual-dtor)
endif()

set_property(TARGET ${US_RCC_EXECUTABLE_TARGET} PROPERTY OUTPUT_NAME ${US_RCC_EXECUTABLE_OUTPUT_NAME})
if(WIN32)
    target_link_libraries(${US_RCC_EXECUTABLE_TARGET} Shlwapi)
endif()

target_link_libraries(${US_RCC_EXECUTABLE_TARGET} nowide::nowide)
target_include_directories(${US_RCC_EXECUTABLE_TARGET} PRIVATE ${CppMicroServices_SOURCE_DIR}/third_party/boost/nowide/include)

set_property(TARGET ${US_RCC_EXECUTABLE_TARGET} APPEND PROPERTY
             COMPILE_DEFINITIONS "MINIZ_NO_ARCHIVE_READING_API;MINIZ_NO_ZLIB_COMPATIBLE_NAMES")

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

# Test case to make sure that if deterministic builds are switched on
if (US_USE_DETERMINISTIC_BUNDLE_BUILDS)
  add_test(NAME Deterministic_Zip_Files_Test
         WORKING_DIRECTORY ${CMAKE_CURRENT_BINARY_DIR}
         COMMAND ${CMAKE_CURRENT_SOURCE_DIR}/test/basic_deterministic_zip_test.$<IF:$<BOOL:${MSVC}>,bat,sh> ${CMAKE_BINARY_DIR}/bin/${US_RCC_EXECUTABLE_OUTPUT_NAME}
         )
endif()
