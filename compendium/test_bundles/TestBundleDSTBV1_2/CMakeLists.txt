usFunctionCreateDSTestBundle(TestBundleDSTBV1_2)

usFunctionCreateTestBundleWithResources(TestBundleDSTBV1_2
  SOURCES src/ServiceImpl.cpp ${_glue_file}
  RESOURCES manifest.json
  BUNDLE_SYMBOLIC_NAME TestBundleDSTBV1_2
  OTHER_LIBRARIES usTestInterfaces usServiceComponent usServiceComponent)

