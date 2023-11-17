usFunctionCreateDSTestBundle(TestBundleDSTOI14)

usFunctionCreateTestBundleWithResources(TestBundleDSTOI14
  SOURCES src/ServiceImpl.cpp ${_glue_file}
  RESOURCES manifest.json
  BUNDLE_SYMBOLIC_NAME TestBundleDSTOI14
  OTHER_LIBRARIES usTestInterfaces usServiceComponent)

