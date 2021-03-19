usFunctionCreateDSTestBundle(TestBundleDSTOICA20)

usFunctionCreateTestBundleWithResources(TestBundleDSTOICA20
  SOURCES src/ServiceImpl.cpp ${_glue_file}
  RESOURCES manifest.json
  BUNDLE_SYMBOLIC_NAME TestBundleDSTOICA20
  OTHER_LIBRARIES usTestInterfaces usServiceComponent)

