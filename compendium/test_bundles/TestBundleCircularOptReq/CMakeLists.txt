usFunctionCreateDSTestBundle(TestBundleCircularOptReq)

usFunctionCreateTestBundleWithResources(TestBundleCircularOptReq
  SOURCES src/ServiceImpl.cpp ${_glue_file}
  RESOURCES manifest.json
  BUNDLE_SYMBOLIC_NAME TestBundleCircularOptReq
  OTHER_LIBRARIES usTestInterfaces usServiceComponent)

