usFunctionCreateDSTestBundle(TestBundleDSTOI15)

usFunctionCreateTestBundleWithResources(TestBundleDSTOI15
  SOURCES src/ServiceImpl.cpp ${_glue_file}
  RESOURCES manifest.json
  BUNDLE_SYMBOLIC_NAME TestBundleDSTOI15
  OTHER_LIBRARIES usTestInterfaces usServiceComponent)

