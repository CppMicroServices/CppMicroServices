#! Generate a source file which handles proper initialization of a module.
#!
#! This CMake function will store the path to a generated source file in the
#! src_var variable, which should be compiled into a module. Example usage:
#!
#! \verbatim
#! set(module_srcs )
#! usFunctionGenerateModuleInit(module_srcs
#!                              NAME "My Module"
#!                              LIBRARY_NAME "mylib"
#!                              VERSION "1.2.0"
#!                             )
#! add_library(mylib ${module_srcs})
#! \endverbatim
#!
#! \param src_var (required) The name of a list variable to which the path of the generated
#!        source file will be appended.
#! \param NAME (required) A human-readable name for the module.
#! \param LIBRARY_NAME (optional) The name of the module, without extension. If empty, the
#!        NAME argument will be used.
#! \param AUTOLOAD_DIR (optional) The name of a directory relative to this modules library
#!        location from which modules will be auto-loaded during activation of this module.
#!        If unspecified, the LIBRARY_NAME argument will be used. If an empty string is provided,
#!        auto-loading will be disabled for this module.
#! \param DEPENDS (optional) A string containing module dependencies.
#! \param VERSION (optional) A version string for the module.
#!
function(usFunctionGenerateModuleInit src_var)

MACRO_PARSE_ARGUMENTS(US_MODULE "NAME;LIBRARY_NAME;AUTOLOAD_DIR;DEPENDS;VERSION" "EXECUTABLE" ${ARGN})

# sanity checks
if(NOT US_MODULE_NAME)
  message(SEND_ERROR "NAME argument is mandatory")
endif()

if(US_MODULE_EXECUTABLE AND US_MODULE_LIBRARY_NAME)
  message("[Executable: ${US_MODULE_NAME}] Ignoring LIBRARY_NAME argument.")
  set(US_MODULE_LIBRARY_NAME )
endif()

if(NOT US_MODULE_LIBRARY_NAME AND NOT US_MODULE_EXECUTABLE)
  set(US_MODULE_LIBRARY_NAME ${US_MODULE_NAME})
endif()

set(_regex_validation "[a-zA-Z-_][a-zA-Z-_0-9]*")
if(US_MODULE_EXECUTABLE)
  string(REGEX MATCH ${_regex_validation} _valid_chars ${US_MODULE_NAME})
  if(NOT _valid_chars STREQUAL US_MODULE_NAME)
    message(FATAL_ERROR "[Executable: ${US_MODULE_NAME}] MODULE_NAME contains illegal characters.")
  endif()
else()
  string(REGEX MATCH ${_regex_validation} _valid_chars ${US_MODULE_LIBRARY_NAME})
  if(NOT _valid_chars STREQUAL US_MODULE_LIBRARY_NAME)
    message(FATAL_ERROR "[Module: ${US_MODULE_NAME}] LIBRARY_NAME \"${US_MODULE_LIBRARY_NAME}\" contains illegal characters.")
  endif()
endif()

# The call to MACRO_PARSE_ARGUMENTS always defines variables for the argument names.
# Check manually if AUTOLOAD_DIR was provided or not.
list(FIND ARGN AUTOLOAD_DIR _is_autoload_dir_defined)
if(_is_autoload_dir_defined EQUAL -1)
  set(US_MODULE_AUTOLOAD_DIR ${US_MODULE_LIBRARY_NAME})
endif()

# Create variables of the ModuleInfo object, created in CMake/usModuleInit.cpp
set(US_MODULE_DEPENDS_STR "")
foreach(_dep ${US_MODULE_DEPENDS})
  set(US_MODULE_DEPENDS_STR "${US_MODULE_DEPENDS_STR} ${_dep}")
endforeach()

if(US_MODULE_LIBRARY_NAME)
  set(module_init_src_file "${CMAKE_CURRENT_BINARY_DIR}/${US_MODULE_LIBRARY_NAME}_init.cpp")
else()
  set(module_init_src_file "${CMAKE_CURRENT_BINARY_DIR}/${US_MODULE_NAME}_init.cpp")
endif()

configure_file(${CppMicroServices_SOURCE_DIR}/CMake/usModuleInit.cpp ${module_init_src_file} @ONLY)

set(_src ${${src_var}} ${module_init_src_file})
set(${src_var} ${_src} PARENT_SCOPE)

endfunction()
