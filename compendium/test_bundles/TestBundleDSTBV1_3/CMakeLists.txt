usFunctionCreateDSTestBundle(TestBundleDSTBV1_3)

usFunctionCreateTestBundleWithResources(TestBundleDSTBV1_3
  SOURCES src/ServiceImpl.cpp ${_glue_file}
  RESOURCES manifest.json
  BUNDLE_SYMBOLIC_NAME TestBundleDSTBV1_3
  OTHER_LIBRARIES usTestInterfaces usServiceComponent)
