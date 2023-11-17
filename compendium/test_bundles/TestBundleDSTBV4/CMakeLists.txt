usFunctionCreateDSTestBundle(TestBundleDSTBV4)

usFunctionCreateTestBundleWithResources(TestBundleDSTBV4
  SOURCES src/ServiceImpl.cpp ${_glue_file}
  RESOURCES manifest.json
  BUNDLE_SYMBOLIC_NAME TestBundleDSTBV4
  OTHER_LIBRARIES usTestInterfaces usServiceComponent)
