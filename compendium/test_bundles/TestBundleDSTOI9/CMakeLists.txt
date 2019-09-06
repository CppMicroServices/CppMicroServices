usFunctionCreateDSTestBundle(TestBundleDSTOI9)

usFunctionCreateTestBundleWithResources(TestBundleDSTOI9
  SOURCES src/ServiceImpl.cpp ${_glue_file}
  RESOURCES manifest.json
  BUNDLE_SYMBOLIC_NAME TestBundleDSTOI9
  OTHER_LIBRARIES usTestInterfaces usServiceComponent)

