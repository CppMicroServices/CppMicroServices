usFunctionCreateDSTestBundle(TestBundleDSTOI5)

usFunctionCreateTestBundleWithResources(TestBundleDSTOI5
  SOURCES src/ServiceImpl.cpp ${_glue_file}
  RESOURCES manifest.json
  BUNDLE_SYMBOLIC_NAME TestBundleDSTOI5
  OTHER_LIBRARIES usTestInterfaces usServiceComponent usServiceComponent)

