#! \ingroup MicroServicesCMake
#! \brief Generate a source file which handles proper initialization of a module.
#!
#! This CMake function will store the path to a generated source file in the
#! src_var variable, which should be compiled into a module. The modules source
#! code must be compiled with the US_MODULE_NAME pre-processor definition.
#! Example usage:
#!
#! \code{.cmake}
#! set(module_srcs )
#! usFunctionGenerateModuleInit(module_srcs)
#! add_library(mylib ${module_srcs})
#! set_property(TARGET ${mylib} APPEND PROPERTY COMPILE_DEFINITIONS US_MODULE_NAME=MyModule)
#! \endcode
#!
#! \param src_var (required) The name of a list variable to which the path of the generated
#!        source file will be appended.
#!
#! \see \ref MicroServices_AutoLoading
#!
function(usFunctionGenerateModuleInit src_var)

  set(module_init_src_file "${CMAKE_CURRENT_BINARY_DIR}/us_init.cpp")
  configure_file(${US_MODULE_INIT_TEMPLATE} ${module_init_src_file} @ONLY)

  set(_src ${module_init_src_file} ${${src_var}})
  set(${src_var} ${_src} PARENT_SCOPE)

endfunction()
