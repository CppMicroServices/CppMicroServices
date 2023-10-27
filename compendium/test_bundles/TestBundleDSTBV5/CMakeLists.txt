usFunctionCreateDSTestBundle(TestBundleDSTBV5)

usFunctionCreateTestBundleWithResources(TestBundleDSTBV5
  SOURCES src/ServiceImpl.cpp ${_glue_file}
  RESOURCES manifest.json
  BUNDLE_SYMBOLIC_NAME TestBundleDSTBV5
  OTHER_LIBRARIES usTestInterfaces usServiceComponent)
