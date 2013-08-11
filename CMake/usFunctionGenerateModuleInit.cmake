#! \ingroup MicroServicesCMake
#! \brief Generate a source file which handles proper initialization of a module.
#!
#! This CMake function will store the path to a generated source file in the
#! src_var variable, which should be compiled into a module. Example usage:
#!
#! \code{.cmake}
#! set(module_srcs )
#! usFunctionGenerateModuleInit(module_srcs
#!                              NAME "My Module"
#!                              LIBRARY_NAME "mylib"
#!                             )
#! add_library(mylib ${module_srcs})
#! \endcode
#!
#! \param src_var (required) The name of a list variable to which the path of the generated
#!        source file will be appended.
#! \param NAME (required) A human-readable name for the module.
#! \param LIBRARY_NAME (optional) The name of the module, without extension. If empty, the
#!        NAME argument will be used.
#!
#! \see #usFunctionGenerateExecutableInit
#! \see \ref MicroServices_AutoLoading
#!
function(usFunctionGenerateModuleInit src_var)

  cmake_parse_arguments(US_MODULE "EXECUTABLE" "NAME;LIBRARY_NAME" "" ${ARGN})

  if(US_MODULE_EXECUTABLE)
    message(SEND_ERROR "EXECUTABLE option no longer supported. Use usFunctionGenerateExecutableInit instead.")
  endif()

  # sanity checks
  if(NOT US_MODULE_NAME)
    message(SEND_ERROR "NAME argument is mandatory")
  endif()

  if(NOT US_MODULE_LIBRARY_NAME)
    set(US_MODULE_LIBRARY_NAME ${US_MODULE_NAME})
  endif()

  set(_regex_validation "[a-zA-Z_-][a-zA-Z_0-9-]*")
  string(REGEX MATCH ${_regex_validation} _valid_chars ${US_MODULE_LIBRARY_NAME})
  if(NOT _valid_chars STREQUAL US_MODULE_LIBRARY_NAME)
    message(FATAL_ERROR "[Module: ${US_MODULE_NAME}] LIBRARY_NAME \"${US_MODULE_LIBRARY_NAME}\" contains illegal characters.")
  endif()

  set(module_init_src_file "${CMAKE_CURRENT_BINARY_DIR}/${US_MODULE_LIBRARY_NAME}_init.cpp")
  configure_file(${CppMicroServices_MODULE_INIT_TEMPLATE} ${module_init_src_file} @ONLY)

  set(_src ${${src_var}} ${module_init_src_file})
  set(${src_var} ${_src} PARENT_SCOPE)

endfunction()
