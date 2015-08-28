# The library name for the bundle
set(_lib_name dictionaryservice)

# A list of source code files
set(_srcs
  Activator.cpp
  IDictionaryService.cpp
)

CreateExample(dictionaryservice ${_srcs})
