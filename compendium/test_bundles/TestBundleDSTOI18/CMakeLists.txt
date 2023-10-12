usFunctionCreateDSTestBundle(TestBundleDSTOI18)

usFunctionCreateTestBundleWithResources(TestBundleDSTOI18 USES_DS
  SOURCES src/ServiceImpl.cpp ${_glue_file}
  RESOURCES manifest.json
  BUNDLE_SYMBOLIC_NAME TestBundleDSTOI18
  OTHER_LIBRARIES usTestInterfaces usServiceComponent usServiceComponent)

