usFunctionCreateDSTestBundle(TestBundleCircular03)

usFunctionCreateTestBundleWithResources(TestBundleCircular03
  SOURCES src/ServiceImpl.cpp ${_glue_file}
  RESOURCES manifest.json
  BUNDLE_SYMBOLIC_NAME TestBundleCircular03
  OTHER_LIBRARIES usTestInterfaces usServiceComponent)

