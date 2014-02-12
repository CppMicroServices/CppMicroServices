#! \ingroup MicroServicesCMake
#! \brief Generate a source file which handles proper initialization of an executable.
#!
#! This CMake function will store the path to a generated source file in the
#! src_var variable, which should be compiled into an executable. Example usage:
#!
#! \code{.cmake}
#! set(executable_srcs )
#! usFunctionGenerateExecutableInit(executable_srcs
#!                                  IDENTIFIER "MyExecutable"
#!                                  )
#! add_executable(MyExecutable ${executable_srcs})
#! \endcode
#!
#! \param src_var (required) The name of a list variable to which the path of the generated
#!        source file will be appended.
#! \param IDENTIFIER (required) A valid C identifier for the executable.
#!
#! \see #usFunctionGenerateModuleInit
#! \see \ref MicroServices_AutoLoading
#!
function(usFunctionGenerateExecutableInit src_var)

  cmake_parse_arguments(US_EXECUTABLE "" "IDENTIFIER" "" ${ARGN})

  # sanity checks
  if(NOT US_EXECUTABLE_IDENTIFIER)
    message(SEND_ERROR "IDENTIFIER argument is mandatory")
  endif()

  set(_regex_validation "[a-zA-Z_-][a-zA-Z_0-9-]*")
  string(REGEX MATCH ${_regex_validation} _valid_chars ${US_EXECUTABLE_IDENTIFIER})
  if(NOT _valid_chars STREQUAL US_EXECUTABLE_IDENTIFIER)
    message(FATAL_ERROR "IDENTIFIER contains illegal characters.")
  endif()

  set(exec_init_src_file "${CMAKE_CURRENT_BINARY_DIR}/${US_EXECUTABLE_IDENTIFIER}_init.cpp")
  configure_file(${CppMicroServices_EXECUTABLE_INIT_TEMPLATE} ${exec_init_src_file} @ONLY)

  set(_src ${exec_init_src_file} ${${src_var}})
  set(${src_var} ${_src} PARENT_SCOPE)

endfunction()
