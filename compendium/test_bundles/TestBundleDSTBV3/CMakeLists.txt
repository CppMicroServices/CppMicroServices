usFunctionCreateDSTestBundle(TestBundleDSTBV3)

usFunctionCreateTestBundleWithResources(TestBundleDSTBV3
  SOURCES src/ServiceImpl.cpp ${_glue_file}
  RESOURCES manifest.json
  BUNDLE_SYMBOLIC_NAME TestBundleDSTBV3
  OTHER_LIBRARIES usTestInterfaces usServiceComponent)
