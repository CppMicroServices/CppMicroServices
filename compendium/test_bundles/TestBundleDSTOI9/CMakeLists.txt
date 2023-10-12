usFunctionCreateDSTestBundle(TestBundleDSTOI9)

usFunctionCreateTestBundleWithResources(TestBundleDSTOI9 USES_DS
  SOURCES src/ServiceImpl.cpp ${_glue_file}
  RESOURCES manifest.json
  BUNDLE_SYMBOLIC_NAME TestBundleDSTOI9
  OTHER_LIBRARIES usTestInterfaces usServiceComponent)

