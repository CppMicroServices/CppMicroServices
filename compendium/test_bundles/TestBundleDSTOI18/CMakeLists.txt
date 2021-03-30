usFunctionCreateDSTestBundle(TestBundleDSTOI18)

usFunctionCreateTestBundleWithResources(TestBundleDSTOI18
  SOURCES src/ServiceImpl.cpp ${_glue_file}
  RESOURCES manifest.json
  BUNDLE_SYMBOLIC_NAME TestBundleDSTOI18
  OTHER_LIBRARIES usTestInterfaces usServiceComponent usServiceComponent)

