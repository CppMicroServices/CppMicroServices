
set(resource_files
  manifest.json
)

FunctionCreateTestBundleWithResources(TestBundleM
  SOURCES TestBundleM.cpp
  RESOURCES ${resource_files})
