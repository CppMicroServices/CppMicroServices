#! \ingroup MicroServicesCMake
#! \brief Generate a source file which handles proper initialization of a module.
#!
#! This CMake function will store the path to a generated source file in the
#! src_var variable, which should be compiled into a module. Example usage:
#!
#! \code{.cmake}
#! set(module_srcs )
#! usFunctionGenerateModuleInit(module_srcs
#!                              MODULE_NAME MyModule
#!                             )
#! add_library(mylib ${module_srcs})
#! set_property(TARGET $mylib APPEND PROPERTY COMPILE_DEFINITIONS US_MODULE_NAME=MyModule)
#! \endcode
#!
#! \param src_var (required) The name of a list variable to which the path of the generated
#!        source file will be appended.
#! \param MODULE_NAME (required) A unique module name. Must be a valid C identifier.
#!
#! \see \ref MicroServices_AutoLoading
#!
function(usFunctionGenerateModuleInit src_var)

  cmake_parse_arguments(US "" "MODULE_NAME" "" ${ARGN})

  # sanity checks
  if(NOT US_MODULE_NAME)
    message(SEND_ERROR "MODULE_NAME argument is mandatory")
  endif()

  set(_regex_validation "[a-zA-Z_-][a-zA-Z_0-9-]*")
  string(REGEX MATCH ${_regex_validation} _valid_chars ${US_MODULE_NAME})
  if(NOT _valid_chars STREQUAL US_MODULE_NAME)
    message(FATAL_ERROR "MODULE_NAME argument \"${US_MODULE_NAME}\" contains illegal characters.")
  endif()

  set(module_init_src_file "${CMAKE_CURRENT_BINARY_DIR}/${US_MODULE_NAME}_init.cpp")
  configure_file(${US_MODULE_INIT_TEMPLATE} ${module_init_src_file} @ONLY)

  set(_src ${module_init_src_file} ${${src_var}})
  set(${src_var} ${_src} PARENT_SCOPE)

endfunction()
