usFunctionCreateDSTestBundle(TestBundleDSTOI1201)

usFunctionCreateTestBundleWithResources(TestBundleDSTOI1201
  SOURCES src/ServiceImpl.cpp ${_glue_file}
  RESOURCES manifest.json
  BUNDLE_SYMBOLIC_NAME TestBundleDSTOI1201
  OTHER_LIBRARIES usTestInterfaces usServiceComponent)

