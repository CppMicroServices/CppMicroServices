usFunctionCreateDSTestBundle(TestBundleDSTOI12)

usFunctionCreateTestBundleWithResources(TestBundleDSTOI12
  SOURCES src/ServiceImpl.cpp ${_glue_file}
  RESOURCES manifest.json
  BUNDLE_SYMBOLIC_NAME TestBundleDSTOI12
  OTHER_LIBRARIES usTestInterfaces usServiceComponent)

