usFunctionCreateDSTestBundle(TestBundleCircularDouble)

usFunctionCreateTestBundleWithResources(TestBundleCircularDouble
  SOURCES src/ServiceImpl.cpp ${_glue_file}
  RESOURCES manifest.json
  BUNDLE_SYMBOLIC_NAME TestBundleCircularDouble
  OTHER_LIBRARIES usTestInterfaces usServiceComponent)

