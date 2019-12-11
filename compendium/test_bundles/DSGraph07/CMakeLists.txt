usFunctionCreateDSTestBundle(DSGraph07)

usFunctionCreateTestBundleWithResources(DSGraph07
  SOURCES src/ServiceImpl.cpp ${_glue_file}
  RESOURCES manifest.json
  BUNDLE_SYMBOLIC_NAME DSGraph07
  OTHER_LIBRARIES usTestInterfaces usServiceComponent)
