# The library name for the module
set(_lib_name dictionaryservice)

# A list of source code files
set(_srcs
  Activator.cpp
  IDictionaryService.cpp
)

# Generate module initialization code
usFunctionGenerateModuleInit(_srcs)

# Create the library
add_library(Example-${_lib_name} ${_srcs})

# Add the required compile definitions
set_property(TARGET Example-${_lib_name} APPEND PROPERTY COMPILE_DEFINITIONS US_MODULE_NAME=${_lib_name})
if(NOT US_BUILD_SHARED_LIBS)
  set_property(TARGET Example-${_lib_name} APPEND PROPERTY COMPILE_DEFINITIONS US_STATIC_MODULE)
endif()


# Link the CppMicroServices library
target_link_libraries(Example-${_lib_name} ${CppMicroServices_LIBRARIES})

set_target_properties(Example-${_lib_name} PROPERTIES
  LABELS Examples
  OUTPUT_NAME ${_lib_name}
)

#CreateExample(dictionaryservice ${_srcs})
