usFunctionCreateDSTestBundle(DSGraph06)

usFunctionCreateTestBundleWithResources(DSGraph06
  SOURCES src/ServiceImpl.cpp ${_glue_file}
  RESOURCES manifest.json
  BUNDLE_SYMBOLIC_NAME DSGraph06
  OTHER_LIBRARIES usTestInterfaces usServiceComponent)
