usFunctionCreateDSTestBundle(TestBundleDSDGMU)

usFunctionCreateTestBundleWithResources(TestBundleDSDGMU
  SOURCES src/ServiceComponentDynamicGreedyMandatoryUnary.cpp ${_glue_file}
  RESOURCES manifest.json
  BUNDLE_SYMBOLIC_NAME TestBundleDSDGMU
  OTHER_LIBRARIES usTestInterfaces usServiceComponent usServiceComponent)

