
# This cmake module builds a very specific test module; one with embedded zip data
# that contains NO directories within the zip. This test module is used
# for test cases involving malformed/invalid bundles.
project(TestModuleWithEmbeddedZip)

set(resource_files
  test.xml
)

set(_srcs foo.cpp)

add_library(${PROJECT_NAME} ${_srcs})

# Disable code-signing on macOS if appending resources.
if (APPLE)
  set_target_properties(${PROJECT_NAME}
    PROPERTIES
    XCODE_ATTRIBUTE_CODE_SIGN_IDENTITY ""
  )
endif()

if(CMAKE_CROSSCOMPILING)
    # Cross-compiled builds need to use the imported host version of usResourceCompiler
    include(${IMPORT_EXECUTABLES})
    set(resource_compiler native-${US_RCC_EXECUTABLE_TARGET})
else()
    set(resource_compiler ${US_RCC_EXECUTABLE})
    if(TARGET ${US_RCC_EXECUTABLE_TARGET})
      set(resource_compiler ${US_RCC_EXECUTABLE_TARGET})
    elseif(NOT resource_compiler)
      message(FATAL_ERROR "The CppMicroServices resource compiler was not found. Check the US_RCC_EXECUTABLE CMake variable.")
    endif()
endif()
      
add_custom_command(
      TARGET ${PROJECT_NAME}
      POST_BUILD
      COMMAND ${CMAKE_COMMAND} -E tar "cf" "${CMAKE_CURRENT_BINARY_DIR}/data.zip" --format=zip -- ${resource_files}
      COMMAND ${resource_compiler} -V -b $<TARGET_FILE:${PROJECT_NAME}> -z ${CMAKE_CURRENT_BINARY_DIR}/data.zip
      WORKING_DIRECTORY ${CMAKE_CURRENT_SOURCE_DIR}
      DEPENDS ${resource_compiler}
      COMMENT "Embedding zip file into ${PROJECT_NAME}"
      VERBATIM)
