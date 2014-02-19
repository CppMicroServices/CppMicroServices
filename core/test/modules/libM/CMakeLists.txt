
set(resource_files
  manifest.json
)

usFunctionCreateTestModuleWithResources(TestModuleM
  SOURCES usTestModuleM.cpp
  RESOURCES ${resource_files})
