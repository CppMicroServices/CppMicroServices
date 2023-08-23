usFunctionCreateDSTestBundle(TestBundleCircularTwoComp)

usFunctionCreateTestBundleWithResources(TestBundleCircularTwoComp
  SOURCES src/ServiceImpl.cpp ${_glue_file}
  RESOURCES manifest.json
  BUNDLE_SYMBOLIC_NAME TestBundleCircularTwoComp
  OTHER_LIBRARIES usTestInterfaces usServiceComponent)

