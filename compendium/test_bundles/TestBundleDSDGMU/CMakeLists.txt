usFunctionCreateDSTestBundle(TestBundleDSDGMU)

usFunctionCreateTestBundleWithResources(TestBundleDSDGMU USES_DS
  SOURCES src/ServiceComponentDynamicGreedyMandatoryUnary.cpp ${_glue_file}
  RESOURCES manifest.json
  BUNDLE_SYMBOLIC_NAME TestBundleDSDGMU
  OTHER_LIBRARIES usTestInterfaces usServiceComponent usServiceComponent)

