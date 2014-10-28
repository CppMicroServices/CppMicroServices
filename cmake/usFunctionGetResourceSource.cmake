#! \ingroup MicroServicesCMake
#! \brief Get a source file name for handling resource dependencies
#!
#! This CMake function retrieves the name of a generated file which has to be added
#! to a modules source file list to set-up resource file dependencies. This ensures
#! that changed resource files will automatically be re-added to the module.
#!
#! Example usage:
#! \code{.cmake}
#! set(module_srcs mylib.cpp)
#! usFunctionGetResourceSource(TARGET mylib
#!                             OUT module_srcs
#!                            )
#! add_library(mylib ${module_srcs})
#! \endcode
#!
#! \param TARGET (required) The name of the target to which the resource files are added.
#! \param OUT (required) A list variable to which the file name will be appended.
#! \param LINK (optional) Generate a suitable source file for LINK mode.
#! \param APPEND (optional) Generate a suitable source file for APPEND mode.
#!
#! \sa usFunctionAddResources
#! \sa usFunctionEmbedResources
#!
function(usFunctionGetResourceSource)
  cmake_parse_arguments(_res "LINK;APPEND" "TARGET;OUT" "" "" ${ARGN})
  if(NOT _res_TARGET)
    message(SEND_ERROR "TARGET must not be empty")
  endif()
  if(NOT _res_OUT)
    message(SEND_ERROR "OUT argument must not be empty")
  endif()
  if(_res_LINK AND _res_APPEND)
    message(SEND_ERROR "Both LINK and APPEND options given.")
  endif()

  set(_out "${CMAKE_CURRENT_BINARY_DIR}/us_${_res_TARGET}/us_resources")
  if(_res_LINK)
    set(_out "${_out}${US_RESOURCE_SOURCE_SUFFIX_LINK}")
  elseif(_res_APPEND)
    set(_out "${_out}${US_RESOURCE_SOURCE_SUFFIX_APPEND}")
  else()
    set(_out "${_out}${US_RESOURCE_SOURCE_SUFFIX}")
  endif()

  set(${_res_OUT} ${${_res_OUT}} ${_out} PARENT_SCOPE)
endfunction()
