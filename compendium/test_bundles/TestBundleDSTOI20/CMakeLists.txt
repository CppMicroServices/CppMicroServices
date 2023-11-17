usFunctionCreateDSTestBundle(TestBundleDSTOI20)

usFunctionCreateTestBundleWithResources(TestBundleDSTOI20
  SOURCES src/ServiceImpl.cpp ${_glue_file}
  RESOURCES manifest.json
  BUNDLE_SYMBOLIC_NAME TestBundleDSTOI20
  OTHER_LIBRARIES usTestInterfaces usServiceComponent usServiceComponent)

