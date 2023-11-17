usFunctionCreateDSTestBundle(TestBundleDSTBV1)

usFunctionCreateTestBundleWithResources(TestBundleDSTBV1
  SOURCES src/ServiceImpl.cpp ${_glue_file}
  RESOURCES manifest.json
  BUNDLE_SYMBOLIC_NAME TestBundleDSTBV1
  OTHER_LIBRARIES usTestInterfaces usServiceComponent)
