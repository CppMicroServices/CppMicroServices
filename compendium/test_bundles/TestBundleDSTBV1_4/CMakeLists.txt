usFunctionCreateDSTestBundle(TestBundleDSTBV1_4)

usFunctionCreateTestBundleWithResources(TestBundleDSTBV1_4
  SOURCES src/ServiceImpl.cpp ${_glue_file}
  RESOURCES manifest.json
  BUNDLE_SYMBOLIC_NAME TestBundleDSTBV1_4
  OTHER_LIBRARIES usTestInterfaces usServiceComponent)
