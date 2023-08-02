usFunctionCreateDSTestBundle(TestBundleCircular02)

usFunctionCreateTestBundleWithResources(TestBundleCircular02
  SOURCES src/ServiceImpl.cpp ${_glue_file}
  RESOURCES manifest.json
  BUNDLE_SYMBOLIC_NAME TestBundleCircular02
  OTHER_LIBRARIES usTestInterfaces usServiceComponent)

