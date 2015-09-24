# The library name for the module
set(_lib_name dictionaryservice)

# A list of source code files
set(_srcs
  Activator.cpp
  IDictionaryService.cpp
)

# Add a special source file to the _srcs variable that
# will enable dependency checking on module resources.
usFunctionGetResourceSource(TARGET Example-${_lib_name} OUT _srcs)

# Generate module initialization code
usFunctionGenerateModuleInit(_srcs)

# Create the library
add_library(Example-${_lib_name} ${_srcs})

# Add the US_MODULE_NAME target property
set_property(TARGET Example-${_lib_name} APPEND PROPERTY US_MODULE_NAME ${_lib_name})

# Add the required compile definition
set_property(TARGET Example-${_lib_name} APPEND PROPERTY COMPILE_DEFINITIONS US_MODULE_NAME=${_lib_name})

# Embed the manifest file
usFunctionEmbedResources(TARGET Example-${_lib_name} WORKING_DIRECTORY resources FILES manifest.json)

# Link the CppMicroServices library
target_link_libraries(Example-${_lib_name} ${CppMicroServices_LIBRARIES})

set_target_properties(Example-${_lib_name} PROPERTIES
  LABELS Examples
  OUTPUT_NAME ${_lib_name}
)

# The above CMake code achieves the same as the CreateExample macro,
# but it is decorated with comments and will show up verbatim in the
# documentation.

#CreateExample(dictionaryservice ${_srcs})
