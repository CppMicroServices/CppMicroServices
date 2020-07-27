usFunctionCreateDSTestBundle(TestBundleDSDGOU)

usFunctionCreateTestBundleWithResources(TestBundleDSDGOU
  SOURCES src/ServiceComponentDynamicGreedyOptionalUnary.cpp ${_glue_file}
  RESOURCES manifest.json
  BUNDLE_SYMBOLIC_NAME TestBundleDSDGOU
  OTHER_LIBRARIES usTestInterfaces usServiceComponent usServiceComponent)

