usFunctionCreateDSTestBundle(TestBundleDSTOI3)

usFunctionCreateTestBundleWithResources(TestBundleDSTOI3 USES_DS
  SOURCES src/ServiceImpl.cpp ${_glue_file}
  RESOURCES manifest.json
  BUNDLE_SYMBOLIC_NAME TestBundleDSTOI3
  OTHER_LIBRARIES usTestInterfaces usServiceComponent)

