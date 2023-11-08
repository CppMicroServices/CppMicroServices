usFunctionCreateDSTestBundle(TestBundleDSTBV1_5)

usFunctionCreateTestBundleWithResources(TestBundleDSTBV1_5 USES_DS
  SOURCES src/ServiceImpl.cpp ${_glue_file}
  RESOURCES manifest.json
  BUNDLE_SYMBOLIC_NAME TestBundleDSTBV1_5
  OTHER_LIBRARIES usTestInterfaces usServiceComponent)
