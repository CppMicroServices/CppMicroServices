usFunctionCreateDSTestBundle(TestBundleDSTOI7)

usFunctionCreateTestBundleWithResources(TestBundleDSTOI7
  SOURCES src/ServiceImpl.cpp ${_glue_file}
  RESOURCES manifest.json
  BUNDLE_SYMBOLIC_NAME TestBundleDSTOI7
  OTHER_LIBRARIES usTestInterfaces usServiceComponent usServiceComponent)

