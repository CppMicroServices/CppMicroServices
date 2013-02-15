#! Embed resources into a shared library or executable.
#!
#! This CMake function uses an external command line program to generate a source
#! file containing data from external resources such as text files or images. The path
#! to the generated source file is appended to the \c src_var variable.
#!
#! Each module can call this function (at most once) to embed resources and make them
#! available at runtime through the Module class. Resources can also be embedded into
#! executables, using the EXECUTABLE_NAME argument instead of LIBRARY_NAME.
#!
#! Example usage:
#! \verbatim
#! set(module_srcs )
#! usFunctionEmbedResources(module_srcs
#!                          LIBRARY_NAME "mylib"
#!                          ROOT_DIR resources
#!                          FILES config.properties logo.png
#!                         )
#!
#! \param LIBRARY_NAME (required if EXECUTABLE_NAME is empty) The library name of the module
#!        which will include the generated source file, without extension.
#! \param EXECUTABLE_NAME (required if LIBRARY_NAME is empty) The name of the executable
#!        which will include the generated source file.
#! \param ROOT_DIR (optional) The root path for all resources listed after the FILES argument.
#!        If no or a relative path is given, it is considered relativ to the current CMake source directory.
#! \param FILES (optional) A list of resources (paths to external files in the file system) relative
#!        to the ROOT_DIR argument or the current CMake source directory if ROOT_DIR is empty.
#!
function(usFunctionEmbedResources src_var)

  MACRO_PARSE_ARGUMENTS(US_RESOURCE "LIBRARY_NAME;EXECUTABLE_NAME;ROOT_DIR;FILES" "" ${ARGN})

  if(NOT src_var)
    message(SEND_ERROR "Output variable name not specified.")
  endif()

  if(US_RESOURCE_EXECUTABLE_NAME AND US_RESOURCE_LIBRARY_NAME)
    message(SEND_ERROR "Only one of LIBRARY_NAME or EXECUTABLE_NAME can be specified.")
  endif()

  if(NOT US_RESOURCE_LIBRARY_NAME AND NOT US_RESOURCE_EXECUTABLE_NAME)
    message(SEND_ERROR "LIBRARY_NAME or EXECUTABLE_NAME argument not specified.")
  endif()

  if(NOT US_RESOURCE_FILES)
    message(WARNING "No FILES argument given. Skipping resource processing.")
    return()
  endif()

  if(NOT US_RESOURCE_ROOT_DIR)
    set(US_RESOURCE_ROOT_DIR ${CMAKE_CURRENT_SOURCE_DIR})
  endif()
  if(NOT IS_ABSOLUTE ${US_RESOURCE_ROOT_DIR})
    set(US_RESOURCE_ROOT_DIR "${CMAKE_CURRENT_SOURCE_DIR}/${US_RESOURCE_ROOT_DIR}")
  endif()
  if(NOT IS_DIRECTORY ${US_RESOURCE_ROOT_DIR})
    message(SEND_ERROR "The ROOT_DIR argument is not a directory: ${US_RESOURCE_ROOT_DIR}")
  endif()

  set(absolute_res_files)
  foreach(res_file ${US_RESOURCE_FILES})
    set(res_file "${US_RESOURCE_ROOT_DIR}/${res_file}")
    if(IS_DIRECTORY ${res_file})
      message(SEND_ERROR "A resource cannot be a directory: ${res_file}")
    endif()
    if(NOT EXISTS ${res_file})
      message(SEND_ERROR "Resource does not exists: ${res_file}")
    endif()
    get_filename_component(res_file "${res_file}" REALPATH)
    file(TO_NATIVE_PATH "${res_file}" res_file)
    list(APPEND absolute_res_files ${res_file})
  endforeach()

  if(US_RESOURCE_LIBRARY_NAME)
    set(us_cpp_resource_file "${CMAKE_CURRENT_BINARY_DIR}/${US_RESOURCE_LIBRARY_NAME}_resources.cpp")
    set(us_lib_name ${US_RESOURCE_LIBRARY_NAME})
  else()
    set(us_cpp_resource_file "${CMAKE_CURRENT_BINARY_DIR}/${US_RESOURCE_EXECUTABLE_NAME}_resources.cpp")
    set(us_lib_name "\"\"")
  endif()

  set(resource_compiler ${CppMicroServices_RCC_EXECUTABLE})
  if(TARGET ${CppMicroServices_RCC_EXECUTABLE_NAME})
    set(resource_compiler ${CppMicroServices_RCC_EXECUTABLE_NAME})
  endif()

  add_custom_command(
    OUTPUT ${us_cpp_resource_file}
    COMMAND ${resource_compiler} "${us_lib_name}" ${us_cpp_resource_file} ${absolute_res_files}
    WORKING_DIRECTORY ${US_RESOURCE_ROOT_DIR}
    DEPENDS ${absolute_res_files}
    COMMENT "Generating embedded resource file ${us_cpp_resource_name}"
  )

  set(${src_var} "${${src_var}};${us_cpp_resource_file}" PARENT_SCOPE)

endfunction()
