usFunctionCreateDSTestBundle(TestBundleDSTOI6)

usFunctionCreateTestBundleWithResources(TestBundleDSTOI6
  SOURCES src/ServiceImpl.cpp ${_glue_file}
  RESOURCES manifest.json
  BUNDLE_SYMBOLIC_NAME TestBundleDSTOI6
  OTHER_LIBRARIES usTestInterfaces usServiceComponent)

