usFunctionCreateDSTestBundle(TestBundleDSTOI19)

usFunctionCreateTestBundleWithResources(TestBundleDSTOI19
  SOURCES src/ServiceImpl.cpp ${_glue_file}
  RESOURCES manifest.json
  BUNDLE_SYMBOLIC_NAME TestBundleDSTOI19
  OTHER_LIBRARIES usTestInterfaces usServiceComponent usServiceComponent)

