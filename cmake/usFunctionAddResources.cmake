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
#!       files, you have to add a file named \e $<your-target-name>_resources.cpp
#!       to the source list of the target. This ensures that changed resource files
#!       will automatically be re-added to the module.
#!
#! In the case of linking static modules which contain resources to the target module,
#! adding the static module target name to the ZIP_ARCHIVES list will merge its
#! resources into the target module.
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
#! \param ZIP_ARCHIVES (optional) A list of zip archives (relative to the current working directory
#!        or absolute file paths) whose contents is merged into the target module. If a list entry
#!        is a valid target name and that target is a static library, its absolute file path is
#!        used instead.
#!
function(usFunctionAddResources)

  cmake_parse_arguments(US_RESOURCE "" "TARGET;MODULE_NAME;WORKING_DIRECTORY;COMPRESSION_LEVEL" "FILES;ZIP_ARCHIVES" ${ARGN})

  if(NOT US_RESOURCE_TARGET)
    message(SEND_ERROR "TARGET argument not specified.")
  endif()

  if(NOT US_RESOURCE_MODULE_NAME)
    get_target_property(US_RESOURCE_MODULE_NAME ${US_RESOURCE_TARGET} US_MODULE_NAME)
    if(NOT US_RESOURCE_MODULE_NAME)
      message(SEND_ERROR "Either the MODULE_NAME argument or the US_MODULE_NAME target property is required.")
    endif()
  endif()

  if(NOT US_RESOURCE_FILES AND NOT US_RESOURCE_ZIP_ARCHIVES)
    message(WARNING "No resources specified. Skipping resource processing.")
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

  set(_cmd_deps )
  foreach(_file ${US_RESOURCE_FILES})
    if(IS_ABSOLUTE ${_file})
      list(APPEND _cmd_deps ${_file})
    else()
      list(APPEND _cmd_deps ${US_RESOURCE_WORKING_DIRECTORY}/${_file})
    endif()
  endforeach()

  set(_zip_args )
  if(US_RESOURCE_ZIP_ARCHIVES)
    foreach(_zip_archive ${US_RESOURCE_ZIP_ARCHIVES})
      if(TARGET ${_zip_archive})
        get_target_property(_is_static_lib ${_zip_archive} TYPE)
        if(_is_static_lib STREQUAL "STATIC_LIBRARY")
          list(APPEND _cmd_deps ${_zip_archive})
          list(APPEND _zip_args $<TARGET_FILE:${_zip_archive}>)
        endif()
      else()
        if(IS_ABSOLUTE ${_zip_archive})
          list(APPEND _cmd_deps ${_zip_archive})
        else()
          list(APPEND _cmd_deps ${US_RESOURCE_WORKING_DIRECTORY}/${_zip_archive})
        endif()
        list(APPEND _zip_args ${_zip_archive})
      endif()
    endforeach()
  endif()

  # This command depends on the given resource files and creates a source
  # file which must be added to the source list of the related target.
  # This way, the following command is executed if the resources change
  # and it just touches the created source file to force a (actually unnecessary)
  # re-linking and hence the execution of POST_BUILD commands.
  add_custom_command(
    OUTPUT ${CMAKE_CURRENT_BINARY_DIR}/${US_RESOURCE_TARGET}_resources.cpp
    COMMAND ${CMAKE_COMMAND} -E copy ${US_CMAKE_RESOURCE_DEPENDENCIES_CPP} ${CMAKE_CURRENT_BINARY_DIR}/${US_RESOURCE_TARGET}_resources.cpp
    DEPENDS ${_cmd_deps} ${resource_compiler}
    COMMENT "Checking resource dependencies for ${US_RESOURCE_TARGET}"
    VERBATIM
   )

  add_custom_command(
    TARGET ${US_RESOURCE_TARGET}
    POST_BUILD
    COMMAND ${resource_compiler} ${cmd_line_args} $<TARGET_FILE:${US_RESOURCE_TARGET}> ${US_RESOURCE_MODULE_NAME} -a ${US_RESOURCE_FILES} -m ${_zip_args}
    WORKING_DIRECTORY ${US_RESOURCE_WORKING_DIRECTORY}
    COMMENT "Adding resources to ${US_RESOURCE_TARGET}"
    VERBATIM
  )

endfunction()
