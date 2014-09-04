#! \ingroup MicroServicesCMake
#! \brief Add resources to a library or executable.
#!
#! This CMake function uses an external command line program to generate a ZIP archive
#! containing data from external resources such as text files or images or other ZIP
#! archives. The created archive file is appended as a binary blob to the target file.
#!
#! Each module can call this function to add resources and make them available at
#! runtime through the Module class. Multiple calls to this function append the
#! input files to the target file.
#!
#! \note To set-up correct file dependencies from your module target to your resource
#!       files, you have to add a file named \emph $<your-target-name>_resources.cpp
#!       to the source list of the target. This ensures that changes resource files
#!       will be automatically re-added to the module.
#!
#! Example usage:
#! \code{.cmake}
#! set(module_srcs )
#! usFunctionAddResources(TARGET mylib
#!                        MODULE_NAME org_me_mylib
#!                        FILES config.properties logo.png
#!                       )
#! \endcode
#!
#! \param TARGET (required) The target to which the resource files are added.
#! \param MODULE_NAME (required/optional) The module name of the target, as specified in
#!        the \c US_MODULE_NAME pre-processor definition of that target. This parameter
#!        is optional if a target property with the name US_MODULE_NAME exists, containing
#!        the required module name.
#! \param COMPRESSION_LEVEL (optional) The zip compression level (0-9). Defaults to the default zip
#!        level. Level 0 disables compression.
#! \param WORKING_DIRECTORY (optional) The root path for all resource files listed after the
#!        FILES argument. If no or a relative path is given, it is considered relativ to the
#!        current CMake source directory.
#! \param FILES (optional) A list of resource files (paths to external files in the file system)
#!        relative to the current working directory.
#!
function(usFunctionAddResources)

  cmake_parse_arguments(US_RESOURCE "" "TARGET;MODULE_NAME;WORKING_DIRECTORY;COMPRESSION_LEVEL" "FILES" ${ARGN})

  if(NOT US_RESOURCE_TARGET)
    message(SEND_ERROR "TARGET argument not specified.")
  endif()

  if(NOT US_RESOURCE_MODULE_NAME)
    get_target_property(US_RESOURCE_MODULE_NAME ${US_RESOURCE_TARGET} US_MODULE_NAME)
    if(NOT US_RESOURCE_MODULE_NAME)
      message(SEND_ERROR "Either the MODULE_NAME argument or the US_MODULE_NAME target property is required.")
    endif()
  endif()

  if(NOT US_RESOURCE_FILES)
    message(WARNING "No FILES argument given. Skipping resource processing.")
    return()
  endif()

  if(NOT US_RESOURCE_WORKING_DIRECTORY)
    set(US_RESOURCE_WORKING_DIRECTORY ${CMAKE_CURRENT_SOURCE_DIR})
  endif()
  if(NOT IS_ABSOLUTE ${US_RESOURCE_WORKING_DIRECTORY})
    set(US_RESOURCE_WORKING_DIRECTORY "${CMAKE_CURRENT_SOURCE_DIR}/${US_RESOURCE_WORKING_DIRECTORY}")
  endif()

  if(US_RESOURCE_COMPRESSION_LEVEL)
    set(cmd_line_args -${US_RESOURCE_COMPRESSION_LEVEL})
  endif()

  set(resource_compiler ${US_RCC_EXECUTABLE})
  if(TARGET ${US_RCC_EXECUTABLE_NAME})
    set(resource_compiler ${US_RCC_EXECUTABLE_NAME})
  elseif(NOT resource_compiler)
    message(FATAL_ERROR "The CppMicroServices resource compiler was not found. Check the US_RCC_EXECUTABLE CMake variable.")
  endif()

  set(_abs_files)
  foreach(_file ${US_RESOURCE_FILES})
    list(APPEND _abs_files ${US_RESOURCE_WORKING_DIRECTORY}/${_file})
  endforeach()

  # This command depends on the given resource files and creates an empty
  # cpp which must be added to the source list of the related target.
  # This way, the following command is executed if the resources change
  # and it just touches the empty cpp file to fore a (actually unnecessary)
  # re-linking and hence the execution of POST_BUILD commands.
  add_custom_command(
    OUTPUT ${US_RESOURCE_TARGET}_resources.cpp
    COMMAND ${CMAKE_COMMAND} -E touch ${US_RESOURCE_TARGET}_resources.cpp
    DEPENDS ${_abs_files} ${resource_compiler}
    COMMENT "Checking resource dependencies for ${US_RESOURCE_TARGET}"
    VERBATIM
   )

  add_custom_command(
    TARGET ${US_RESOURCE_TARGET}
    POST_BUILD
    COMMAND ${resource_compiler} ${cmd_line_args} $<TARGET_FILE:${US_RESOURCE_TARGET}> ${US_RESOURCE_MODULE_NAME} ${US_RESOURCE_FILES}
    WORKING_DIRECTORY ${US_RESOURCE_WORKING_DIRECTORY}
    COMMENT "Adding resources to ${US_RESOURCE_TARGET}"
    VERBATIM
  )

endfunction()
