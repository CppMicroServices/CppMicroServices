#! \ingroup MicroServicesCMake
#! \brief Add resources to a library or executable.
#!
#! This CMake function uses an external command line program to generate a ZIP archive
#! containing data from external resources such as text files or images or other ZIP
#! archives. The created archive file can be appended or linked into the target file
#! using the usFunctionEmbedResources macro.
#!
#! Each module can call this function to add resources and make them available at
#! runtime through the Module class. Multiple calls to this function append the
#! input files.
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
#! \sa usFunctionEmbedResources
#! \sa \ref MicroServices_Resources
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

  if(NOT US_RESOURCE_FILES AND NOT _zip_args)
    return()
  endif()

  if(US_RESOURCE_FILES)
    set(_file_args -a ${US_RESOURCE_FILES})
  endif()
  if(_zip_args)
    set(_zip_args -m ${_zip_args})
  endif()

  get_target_property(_counter ${US_RESOURCE_TARGET} _us_resource_counter)
  if((NOT ${_counter} EQUAL 0) AND NOT _counter)
    set(_counter 0)
  else()
    math(EXPR _counter "${_counter} + 1")
  endif()

  set(_res_zip "${CMAKE_CURRENT_BINARY_DIR}/us_${US_RESOURCE_TARGET}/res_${_counter}.zip")

  add_custom_command(
    OUTPUT ${_res_zip}
    COMMAND ${CMAKE_COMMAND} -E make_directory "${CMAKE_CURRENT_BINARY_DIR}/us_${US_RESOURCE_TARGET}"
    COMMAND ${resource_compiler} ${cmd_line_args} ${_res_zip} ${US_RESOURCE_MODULE_NAME} ${_file_args} ${_zip_args}
    WORKING_DIRECTORY ${US_RESOURCE_WORKING_DIRECTORY}
    DEPENDS ${_cmd_deps} ${resource_compiler}
    COMMENT "Checking resource dependencies for ${US_RESOURCE_TARGET}"
    VERBATIM
  )

  get_target_property(_res_zips ${US_RESOURCE_TARGET} _us_resource_zips)
  if(NOT _res_zips)
    set(_res_zips )
  endif()
  list(APPEND _res_zips ${_res_zip})

  set_target_properties(${US_RESOURCE_TARGET} PROPERTIES
    _us_resource_counter "${_counter}"
    _us_resource_zips "${_res_zips}"
  )

endfunction()
