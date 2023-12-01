usFunctionCreateDSTestBundle(TestBundleDSTOI4)

usFunctionCreateTestBundleWithResources(TestBundleDSTOI4
  SOURCES src/ServiceImpl.cpp ${_glue_file}
  RESOURCES manifest.json
  BUNDLE_SYMBOLIC_NAME TestBundleDSTOI4
  OTHER_LIBRARIES usTestInterfaces usServiceComponent)

