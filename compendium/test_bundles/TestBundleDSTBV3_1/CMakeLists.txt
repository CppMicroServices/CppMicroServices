usFunctionCreateDSTestBundle(TestBundleDSTBV3_1)

usFunctionCreateTestBundleWithResources(TestBundleDSTBV3_1
  SOURCES src/ServiceImpl.cpp ${_glue_file}
  RESOURCES manifest.json
  BUNDLE_SYMBOLIC_NAME TestBundleDSTBV3_1
  OTHER_LIBRARIES usTestInterfaces usServiceComponent)
