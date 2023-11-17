usFunctionCreateDSTestBundle(TestBundleDSTOI1)

usFunctionCreateTestBundleWithResources(TestBundleDSTOI1
  SOURCES src/ServiceImpl.cpp ${_glue_file}
  RESOURCES manifest.json
  BUNDLE_SYMBOLIC_NAME TestBundleDSTOI1
  OTHER_LIBRARIES usTestInterfaces usServiceComponent)
