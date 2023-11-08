usFunctionCreateDSTestBundle(TestBundleDSTBV2)

usFunctionCreateTestBundleWithResources(TestBundleDSTBV2 USES_DS
  SOURCES src/ServiceImpl.cpp ${_glue_file}
  RESOURCES manifest.json
  BUNDLE_SYMBOLIC_NAME TestBundleDSTBV2
  OTHER_LIBRARIES usTestInterfaces usServiceComponent)
