usFunctionCreateDSTestBundle(TestBundleDSTOI8)

usFunctionCreateTestBundleWithResources(TestBundleDSTOI8 USES_DS
  SOURCES src/ServiceImpl.cpp ${_glue_file}
  RESOURCES manifest.json
  BUNDLE_SYMBOLIC_NAME TestBundleDSTOI8
  OTHER_LIBRARIES usTestInterfaces usServiceComponent usServiceComponent)

