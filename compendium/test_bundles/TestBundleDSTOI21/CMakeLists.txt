usFunctionCreateDSTestBundle(TestBundleDSTOI21)

usFunctionCreateTestBundleWithResources(TestBundleDSTOI21 USES_DS
  SOURCES src/ServiceImpl.cpp ${_glue_file}
  RESOURCES manifest.json
  BUNDLE_SYMBOLIC_NAME TestBundleDSTOI21
  OTHER_LIBRARIES usTestInterfaces usServiceComponent usServiceComponent)

