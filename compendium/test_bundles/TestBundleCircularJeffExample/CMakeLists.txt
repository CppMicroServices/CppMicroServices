usFunctionCreateDSTestBundle(TestBundleCircularJeffExample)

usFunctionCreateTestBundleWithResources(TestBundleCircularJeffExample
  SOURCES src/ServiceImpl.cpp ${_glue_file}
  RESOURCES manifest.json
  BUNDLE_SYMBOLIC_NAME TestBundleCircularJeffExample
  OTHER_LIBRARIES usTestInterfaces usServiceComponent)

