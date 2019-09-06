usFunctionCreateDSTestBundle(TestBundleDSTOI2)

usFunctionCreateTestBundleWithResources(TestBundleDSTOI2
  SOURCES src/ServiceImpl.cpp ${_glue_file}
  RESOURCES manifest.json
  BUNDLE_SYMBOLIC_NAME TestBundleDSTOI2
  OTHER_LIBRARIES usTestInterfaces usServiceComponent)

