
set(resource_files
  manifest.json
)

usFunctionCreateTestBundleWithResources(TestBundleM
  SOURCES usTestBundleM.cpp
  RESOURCES ${resource_files})
