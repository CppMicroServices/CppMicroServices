# The library name for the module
set(_lib_name dictionaryservice)

# A list of source code files
set(_srcs
  Activator.cpp
  IDictionaryService.cpp
)

# Generate module initialization code
usFunctionGenerateModuleInit(_srcs
                             NAME "Dictionary Service"
                             LIBRARY_NAME ${_lib_name})

# Create the library
add_library(Example-${_lib_name} SHARED ${_srcs})

# Link the CppMicroServices library
target_link_libraries(Example-${_lib_name} ${CppMicroServices_LIBRARIES})

set_target_properties(Example-${_lib_name} PROPERTIES
  LABELS Examples
  OUTPUT_NAME ${_lib_name}
)

#CreateExample(dictionaryservice ${_srcs})
