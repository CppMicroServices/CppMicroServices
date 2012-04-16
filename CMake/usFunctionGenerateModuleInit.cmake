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
#! \param DEPENDS (optional) A string containing module dependencies.
#! \param VERSION (optional) A version string for the module.
#!
function(usFunctionGenerateModuleInit src_var)

MACRO_PARSE_ARGUMENTS(US_MODULE "NAME;LIBRARY_NAME;DEPENDS;VERSION" "EXECUTABLE" ${ARGN})

# sanity checks
if(NOT US_MODULE_NAME)
  message(SEND_ERROR "NAME argument is mandatory")
endif()

if(NOT US_MODULE_LIBRARY_NAME AND NOT US_MODULE_EXECUTABLE)
  set(US_MODULE_LIBRARY_NAME ${US_MODULE_NAME})
endif()

# Create variables of the ModuleInfo object, created in CMake/usModuleInit.cpp
set(US_MODULE_DEPENDS_STR "")
foreach(_dep ${US_MODULE_DEPENDS})
  set(US_MODULE_DEPENDS_STR "${US_MODULE_DEPENDS_STR} ${_dep}")
endforeach()

set(module_init_src_file "${CMAKE_CURRENT_BINARY_DIR}/${US_MODULE_NAME}_init.cpp")
configure_file(${CppMicroServices_SOURCE_DIR}/CMake/usModuleInit.cpp ${module_init_src_file} @ONLY)

set(_src ${${src_var}} ${module_init_src_file})
set(${src_var} ${_src} PARENT_SCOPE)

endfunction()
