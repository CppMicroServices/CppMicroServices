usFunctionCreateDSTestBundle(TestBundleDSTBV5)

usFunctionCreateTestBundleWithResources(TestBundleDSTBV5 USES_DS
  SOURCES src/ServiceImpl.cpp ${_glue_file}
  RESOURCES manifest.json
  BUNDLE_SYMBOLIC_NAME TestBundleDSTBV5
  OTHER_LIBRARIES usTestInterfaces usServiceComponent)
