usFunctionCreateDSTestBundle(TestBundleDSTOI10)

usFunctionCreateTestBundleWithResources(TestBundleDSTOI10
  SOURCES src/ServiceImpl.cpp ${_glue_file}
  RESOURCES manifest.json
  BUNDLE_SYMBOLIC_NAME TestBundleDSTOI10
  OTHER_LIBRARIES usTestInterfaces usServiceComponent)
