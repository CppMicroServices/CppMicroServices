usFunctionCreateDSTestBundle(DSGraph10)

usFunctionCreateTestBundleWithResources(DSGraph10
  SOURCES src/ServiceImpl.cpp ${_glue_file}
  RESOURCES manifest.json
  BUNDLE_SYMBOLIC_NAME DSGraph10
  OTHER_LIBRARIES usTestInterfaces usServiceComponent)
