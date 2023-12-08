usFunctionCreateDSTestBundle(TestBundleDSTBV1_1)

usFunctionCreateTestBundleWithResources(TestBundleDSTBV1_1
  SOURCES src/ServiceImpl.cpp ${_glue_file}
  RESOURCES manifest.json
  BUNDLE_SYMBOLIC_NAME TestBundleDSTBV1_1
  OTHER_LIBRARIES usTestInterfaces usServiceComponent)

