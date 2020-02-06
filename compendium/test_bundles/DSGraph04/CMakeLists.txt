usFunctionCreateDSTestBundle(DSGraph04)

usFunctionCreateTestBundleWithResources(DSGraph04
  SOURCES src/ServiceImpl.cpp ${_glue_file}
  RESOURCES manifest.json
  BUNDLE_SYMBOLIC_NAME DSGraph04
  OTHER_LIBRARIES usTestInterfaces usServiceComponent)
