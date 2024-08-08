usFunctionCreateDSTestBundle(DSGraph08)

usFunctionCreateTestBundleWithResources(DSGraph08
  SOURCES src/ServiceImpl.cpp ${_glue_file}
  RESOURCES manifest.json
  BUNDLE_SYMBOLIC_NAME DSGraph08
  OTHER_LIBRARIES usTestInterfaces usServiceComponent)
