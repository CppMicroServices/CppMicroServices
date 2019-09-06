usFunctionCreateDSTestBundle(TestBundleDSTOI16)

usFunctionCreateTestBundleWithResources(TestBundleDSTOI16
  SOURCES src/ServiceImpl.cpp ${_glue_file}
  RESOURCES manifest.json
  BUNDLE_SYMBOLIC_NAME TestBundleDSTOI16
  OTHER_LIBRARIES usTestInterfaces usServiceComponent)

