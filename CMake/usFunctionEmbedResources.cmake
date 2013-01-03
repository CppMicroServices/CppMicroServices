function(usFunctionEmbedResources src_var)

  MACRO_PARSE_ARGUMENTS(US_RESOURCE "LIBRARY_NAME;ROOT_DIR;FILES" "" ${ARGN})

  if(NOT src_var)
    message(SEND_ERROR "Output variable name not specified.")
  endif()

  if(NOT US_RESOURCE_LIBRARY_NAME)
    message(SEND_ERROR "LIBRARY_NAME argument not specified.")
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
	file(TO_NATIVE_PATH "${res_file}" res_file)
    list(APPEND absolute_res_files ${res_file})
  endforeach()

  set(us_cpp_resource_file "${CMAKE_CURRENT_BINARY_DIR}/${US_RESOURCE_LIBRARY_NAME}_resources.cpp")
  add_custom_command(
    OUTPUT ${us_cpp_resource_file}
    COMMAND ${US_RCC_EXECUTABLE_NAME} ${US_RESOURCE_LIBRARY_NAME} ${us_cpp_resource_file} ${absolute_res_files}
    WORKING_DIRECTORY ${US_RESOURCE_ROOT_DIR}
    DEPENDS ${absolute_res_files}
    COMMENT "Generating embedded resource file ${us_cpp_resource_name}"
  )

  set(${src_var} "${${src_var}};${us_cpp_resource_file}" PARENT_SCOPE)

endfunction()
