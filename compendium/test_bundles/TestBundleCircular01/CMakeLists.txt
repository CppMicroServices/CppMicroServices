usFunctionCreateDSTestBundle(TestBundleCircular01)

usFunctionCreateTestBundleWithResources(TestBundleCircular01
  SOURCES src/ServiceImpl.cpp ${_glue_file}
  RESOURCES manifest.json
  BUNDLE_SYMBOLIC_NAME TestBundleCircular01
  OTHER_LIBRARIES usTestInterfaces usServiceComponent)

