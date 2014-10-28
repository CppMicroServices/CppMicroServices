#! \ingroup MicroServicesCMake
#! \brief Embed resources in a library or executable.
#!
#! This CMake function uses an external command line program to generate a ZIP archive
#! containing data from external resources such as text files or images or other ZIP
#! archives. The created archive file is appended or embedded as a binary blob to the target file.
#!
#! \note To set-up correct file dependencies from your module target to your resource
#!       files, you have to add a special source file to the source list of the target.
#!       The source file name can be retrieved by using usFunctionGetResourceSoruce.
#!       This ensures that changed resource files will automatically be re-added to the
#!       module.
#!
#! There are two differend modes for including resources: APPEND and LINK. In APPEND mode,
#! the generated zip file is appended at the end of the target file. In LINK mode, the
#! zip file is compiled / linked into the target using platform specific techniques. LINK
#! mode is necessary if certain tools make additional assumptions about the object layout
#! of the target file (e.g. codesign on MacOS). LINK mode may result in slower module
#! initialization and bigger object files. The default mode is LINK mode on MacOS and
#! APPEND mode on all other platforms.
#!
#! Example usage:
#! \code{.cmake}
#! set(module_srcs )
#! usFunctionEmbedResources(TARGET mylib
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
#! \param APPEND Append the resources zip file to the target file.
#! \param LINK Link (embed) the resources zip file if possible.
#!
#! For the WORKING_DIRECTORY, COMPRESSION_LEVEL, FILES, ZIP_ARCHIVES parameters see the
#! documentation of the usFunctionAddResources macro which is called with these parameters if set.
#!
#! \sa usFunctionAddResources
#! \sa usFunctionGetResourceSource
#! \sa \ref MicroServices_Resources
#!
function(usFunctionEmbedResources)

  cmake_parse_arguments(US_RESOURCE "APPEND;LINK" "TARGET;MODULE_NAME;WORKING_DIRECTORY;COMPRESSION_LEVEL" "FILES;ZIP_ARCHIVES" ${ARGN})

  if(NOT US_RESOURCE_TARGET)
    message(SEND_ERROR "TARGET argument not specified.")
  endif()

  if(US_RESOURCE_FILES OR US_RESOURCE_ZIP_ARCHIVES)
    usFunctionAddResources(TARGET ${US_RESOURCE_TARGET}
      MODULE_NAME ${US_RESOURCE_MODULE_NAME}
      WORKING_DIRECTORY ${US_RESOURCE_WORKING_DIRECTORY}
      COMPRESSION_LEVEL ${US_RESOURCE_COMPRESSION_LEVEL}
      FILES ${US_RESOURCE_FILES}
      ZIP_ARCHIVES ${US_RESOURCE_ZIP_ARCHIVES}
    )
  endif()

  get_target_property(_res_zips ${US_RESOURCE_TARGET} _us_resource_zips)
  if(NOT _res_zips)
    return()
  endif()

  if(US_RESOURCE_APPEND AND US_RESOURCE_LINK)
    message(WARNING "Both APPEND and LINK options specified. Falling back to default behaviour.")
    set(US_RESOURCE_APPEND 0)
    set(US_RESOURCE_LINK 0)
  endif()

  if(US_RESOURCE_LINK AND NOT US_RESOURCE_LINKING_AVAILABLE)
    message(WARNING "Resource linking not available. Falling back to APPEND mode.")
    set(US_RESOURCE_LINK 0)
    set(US_RESOURCE_APPEND 1)
  endif()

  # Set default resource mode
  if(NOT US_RESOURCE_APPEND AND NOT US_RESOURCE_LINK)
    if(US_DEFAULT_RESOURCE_MODE STREQUAL "LINK")
      set(US_RESOURCE_LINK 1)
    else()
      set(US_RESOURCE_APPEND 1)
    endif()
  endif()

  set(_mode )
  if(US_RESOURCE_LINK)
    set(_mode LINK)
  elseif(US_RESOURCE_APPEND)
    set(_mode APPEND)
  endif()
  usFunctionGetResourceSource(TARGET ${US_RESOURCE_TARGET} OUT _source_output ${_mode})

  if(NOT US_RESOURCE_WORKING_DIRECTORY)
    set(US_RESOURCE_WORKING_DIRECTORY ${CMAKE_CURRENT_SOURCE_DIR})
  endif()
  if(NOT IS_ABSOLUTE ${US_RESOURCE_WORKING_DIRECTORY})
    set(US_RESOURCE_WORKING_DIRECTORY "${CMAKE_CURRENT_SOURCE_DIR}/${US_RESOURCE_WORKING_DIRECTORY}")
  endif()

  set(resource_compiler ${US_RCC_EXECUTABLE})
  if(TARGET ${US_RCC_EXECUTABLE_NAME})
    set(resource_compiler ${US_RCC_EXECUTABLE_NAME})
  elseif(NOT resource_compiler)
    message(FATAL_ERROR "The CppMicroServices resource compiler was not found. Check the US_RCC_EXECUTABLE CMake variable.")
  endif()

  set(_zip_archive )
  get_target_property(_counter ${US_RESOURCE_TARGET} _us_resource_counter)
  if(_counter EQUAL 0)
    set(_zip_archive ${_res_zips})
  else()
    set(_zip_archive ${CMAKE_CURRENT_BINARY_DIR}/us_${US_RESOURCE_TARGET}/res.zip)
    add_custom_command(
      OUTPUT ${_zip_archive}
      COMMAND ${resource_compiler} ${_zip_archive} dummy -m ${_res_zips}
      DEPENDS ${_res_zips} ${resource_compiler}
      COMMENT "Creating resources zip file for ${US_RESOURCE_TARGET}"
      VERBATIM
     )
  endif()
  get_filename_component(_zip_archive_name ${_zip_archive} NAME)
  get_filename_component(_zip_archive_path ${_zip_archive} PATH)

  if(US_RESOURCE_LINK)
    if(APPLE)
      add_custom_command(
        OUTPUT ${_source_output}
        COMMAND ${CMAKE_CXX_COMPILER} -c ${US_CMAKE_RESOURCE_DEPENDENCIES_CPP} -o stub.o
        COMMAND ${CMAKE_LINKER} -r -sectcreate __TEXT us_resources ${_zip_archive_name} stub.o -o ${_source_output}
        DEPENDS ${_zip_archive}
        WORKING_DIRECTORY ${_zip_archive_path}
        COMMENT "Linking resources zip file for ${US_RESOURCE_TARGET}"
        VERBATIM
       )
      set_source_files_properties(${_source_output} PROPERTIES EXTERNAL_OBJECT 1 GENERATED 1)
    elseif(WIN32 AND CMAKE_RC_COMPILER)
      set(US_RESOURCE_ARCHIVE ${_zip_archive})
      configure_file(${US_RESOURCE_RC_TEMPLATE} ${_source_output})
      add_custom_command(
        OUTPUT ${_source_output}
        COMMAND ${CMAKE_COMMAND} -E touch ${_source_output}
        DEPENDS ${_zip_archive}
        WORKING_DIRECTORY ${_zip_archive_path}
        COMMENT "Linking resources zip file for ${US_RESOURCE_TARGET}"
        VERBATIM
       )
    elseif(UNIX)
      add_custom_command(
        OUTPUT ${_source_output}
        COMMAND ${CMAKE_LINKER} -r -b binary -o ${_source_output} ${_zip_archive_name}
        COMMAND objcopy --rename-section .data=.rodata,alloc,load,readonly,data,contents ${_source_output} ${_source_output}
        DEPENDS ${_zip_archive}
        WORKING_DIRECTORY ${_zip_archive_path}
        COMMENT "Linking resources zip file for ${US_RESOURCE_TARGET}"
        VERBATIM
       )
      set_source_files_properties(${_source_output} PROPERTIES EXTERNAL_OBJECT 1 GENERATED 1)
    else()
      message(WARNING "Internal error: Resource linking not available. Falling back to APPEND mode.")
      set(US_RESOURCE_LINK 0)
      set(US_RESOURCE_APPEND 1)
    endif()
  endif()

  if(US_RESOURCE_APPEND)
    # This command depends on the given resource files and creates a source
    # file which must be added to the source list of the related target.
    # This way, the following command is executed if the resources change
    # and it just touches the created source file to force a (actually unnecessary)
    # re-linking and hence the execution of POST_BUILD commands.
    add_custom_command(
      OUTPUT ${_source_output}
      COMMAND ${CMAKE_COMMAND} -E copy ${US_CMAKE_RESOURCE_DEPENDENCIES_CPP} ${_source_output}
      DEPENDS ${_zip_archive}
      COMMENT "Checking resource dependencies for ${US_RESOURCE_TARGET}"
      VERBATIM
     )

    add_custom_command(
      TARGET ${US_RESOURCE_TARGET}
      POST_BUILD
      COMMAND ${resource_compiler} --append $<TARGET_FILE:${US_RESOURCE_TARGET}> ${US_RESOURCE_MODULE_NAME} ${_zip_archive}
      WORKING_DIRECTORY ${US_RESOURCE_WORKING_DIRECTORY}
      COMMENT "Appending zipped resources to ${US_RESOURCE_TARGET}"
      VERBATIM
    )
  endif()

endfunction()
