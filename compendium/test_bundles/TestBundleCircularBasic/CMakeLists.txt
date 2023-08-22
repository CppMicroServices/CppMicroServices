usFunctionCreateDSTestBundle(TestBundleCircularBasic)

usFunctionCreateTestBundleWithResources(TestBundleCircularBasic
  SOURCES src/ServiceImpl.cpp ${_glue_file}
  RESOURCES manifest.json
  BUNDLE_SYMBOLIC_NAME TestBundleCircularBasic
  OTHER_LIBRARIES usTestInterfaces usServiceComponent)

