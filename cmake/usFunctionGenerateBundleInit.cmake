#! \ingroup MicroServicesCMake
#! \brief Generate a source file which handles proper initialization of a bundle.
#!
#! This CMake function will store the path to a generated source file in the
#! src_var variable, which should be compiled into a bundle. The bundles source
#! code must be compiled with the US_BUNDLE_NAME pre-processor definition.
#! Example usage:
#!
#! \code{.cmake}
#! set(bundle_srcs )
#! usFunctionGenerateBundleInit(bundle_srcs)
#! add_library(mylib ${bundle_srcs})
#! set_property(TARGET ${mylib} APPEND PROPERTY COMPILE_DEFINITIONS US_BUNDLE_NAME=MyBundle)
#! \endcode
#!
#! \param src_var (required) The name of a list variable to which the path of the generated
#!        source file will be appended.
#!
#! \see \ref MicroServices_AutoLoading
#!
function(usFunctionGenerateBundleInit src_var)

  set(bundle_init_src_file "${CMAKE_CURRENT_BINARY_DIR}/us_init.cpp")
  configure_file(${US_BUNDLE_INIT_TEMPLATE} ${bundle_init_src_file} @ONLY)

  set(_src ${bundle_init_src_file} ${${src_var}})
  set(${src_var} ${_src} PARENT_SCOPE)

endfunction()
