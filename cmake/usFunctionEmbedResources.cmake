#! \ingroup MicroServicesCMake
#! \brief Embed resources into a shared library or executable.
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
#! \code{.cmake}
#! set(module_srcs )
#! usFunctionEmbedResources(module_srcs
#!                          LIBRARY_NAME "mylib"
#!                          ROOT_DIR resources
#!                          FILES config.properties logo.png
#!                         )
#! \endcode
#!
#! \param LIBRARY_NAME (required if EXECUTABLE_NAME is empty) The library name of the module
#!        which will include the generated source file, without extension.
#! \param EXECUTABLE_NAME (required if LIBRARY_NAME is empty) The name of the executable
#!        which will include the generated source file.
#! \param COMPRESSION_LEVEL (optional) The zip compression level. Defaults to the default zip
#!        level. Level 0 disables compression.
#! \param COMPRESSION_THRESHOLD (optional) The compression threshold ranging from 0 to 100 for
#!        actually compressing the resource data. The default threshold is 30, meaning a size
#!        reduction of 30 percent or better results in the resource data being compressed.
#! \param ROOT_DIR (optional) The root path for all resources listed after the FILES argument.
#!        If no or a relative path is given, it is considered relativ to the current CMake source directory.
#! \param FILES (optional) A list of resources (paths to external files in the file system) relative
#!        to the ROOT_DIR argument or the current CMake source directory if ROOT_DIR is empty.
#!
#! The ROOT_DIR and FILES arguments may be repeated any number of times to merge files from
#! different root directories into the embedded resource tree (hence the relative file paths
#! after the FILES argument must be unique).
#!
function(usFunctionEmbedResources src_var)

  set(prefix US_RESOURCE)
  set(arg_names LIBRARY_NAME EXECUTABLE_NAME COMPRESSION_LEVEL COMPRESSION_THRESHOLD ROOT_DIR FILES)
  foreach(arg_name ${arg_names})
    set(${prefix}_${arg_name})
  endforeach(arg_name)

  set(cmd_line_args )
  set(absolute_res_files )
  set(current_arg_name DEFAULT_ARGS)
  set(current_arg_list)
  set(current_root_dir ${CMAKE_CURRENT_SOURCE_DIR})
  foreach(arg ${ARGN})

    list(FIND arg_names "${arg}" is_arg_name)

    if(is_arg_name GREATER -1)
      set(${prefix}_${current_arg_name} ${current_arg_list})
      set(current_arg_name "${arg}")
      set(current_arg_list)
    else()
      set(current_arg_list ${current_arg_list} "${arg}")
      if(current_arg_name STREQUAL "ROOT_DIR")
        set(current_root_dir "${arg}")
        if(NOT IS_ABSOLUTE ${current_root_dir})
          set(current_root_dir "${CMAKE_CURRENT_SOURCE_DIR}/${current_root_dir}")
        endif()
        if(NOT IS_DIRECTORY ${current_root_dir})
          message(SEND_ERROR "The ROOT_DIR argument is not a directory: ${current_root_dir}")
        endif()
        get_filename_component(current_root_dir "${current_root_dir}" REALPATH)
        if(WIN32)
          string(REPLACE "/" "\\" current_root_dir "${current_root_dir}")
        endif()
        list(APPEND cmd_line_args -d "${current_root_dir}")
      elseif(current_arg_name STREQUAL "FILES")
        set(res_file "${current_root_dir}/${arg}")
        if(WIN32)
          string(REPLACE "/" "\\" res_file_native "${res_file}")
        else()
          set(res_file_native "${res_file}")
        endif()
        if(IS_DIRECTORY ${res_file})
          message(SEND_ERROR "A resource cannot be a directory: ${res_file_native}")
        endif()
        if(NOT EXISTS ${res_file})
          message(SEND_ERROR "Resource does not exists: ${res_file_native}")
        endif()
        list(APPEND absolute_res_files ${res_file})
        file(TO_NATIVE_PATH "${arg}" res_filename_native)
        list(APPEND cmd_line_args "${res_filename_native}")
      endif()
    endif(is_arg_name GREATER -1)

  endforeach(arg ${ARGN})

  set(${prefix}_${current_arg_name} ${current_arg_list})

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

  list(GET cmd_line_args 0 first_arg)
  if(NOT first_arg STREQUAL "-d")
    set(cmd_line_args -d "${CMAKE_CURRENT_SOURCE_DIR}" ${cmd_line_args})
  endif()

  if(US_RESOURCE_COMPRESSION_LEVEL)
    set(cmd_line_args -c ${US_RESOURCE_COMPRESSION_LEVEL} ${cmd_line_args})
  endif()

  if(US_RESOURCE_COMPRESSION_THRESHOLD)
    set(cmd_line_args -t ${US_RESOURCE_COMPRESSION_THRESHOLD} ${cmd_line_args})
  endif()

  if(US_RESOURCE_LIBRARY_NAME)
    set(us_cpp_resource_file "${CMAKE_CURRENT_BINARY_DIR}/${US_RESOURCE_LIBRARY_NAME}_resources.cpp")
    set(us_lib_name ${US_RESOURCE_LIBRARY_NAME})
  else()
    set(us_cpp_resource_file "${CMAKE_CURRENT_BINARY_DIR}/${US_RESOURCE_EXECUTABLE_NAME}_resources.cpp")
    set(us_lib_name "")
  endif()

  set(resource_compiler ${CppMicroServices_RCC_EXECUTABLE})
  if(TARGET ${CppMicroServices_RCC_EXECUTABLE_NAME})
    set(resource_compiler ${CppMicroServices_RCC_EXECUTABLE_NAME})
  elseif(NOT resource_compiler)
    message(FATAL_ERROR "The CppMicroServices resource compiler was not found. Check the CppMicroServices_RCC_EXECUTABLE CMake variable.")
  endif()

  add_custom_command(
    OUTPUT ${us_cpp_resource_file}
    COMMAND ${resource_compiler} "${us_lib_name}" ${us_cpp_resource_file} ${cmd_line_args}
    WORKING_DIRECTORY ${CMAKE_CURRENT_SOURCE_DIR}
    DEPENDS ${absolute_res_files} ${resource_compiler}
    COMMENT "Generating embedded resource file ${us_cpp_resource_name}"
    VERBATIM
  )

  set(${src_var} "${${src_var}};${us_cpp_resource_file}" PARENT_SCOPE)

endfunction()
