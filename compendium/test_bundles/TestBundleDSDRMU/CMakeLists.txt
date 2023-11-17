usFunctionCreateDSTestBundle(TestBundleDSDRMU)

usFunctionCreateTestBundleWithResources(TestBundleDSDRMU
  SOURCES src/ServiceComponentDynamicReluctantMandatoryUnary.cpp ${_glue_file}
  RESOURCES manifest.json
  BUNDLE_SYMBOLIC_NAME TestBundleDSDRMU
  OTHER_LIBRARIES usTestInterfaces usServiceComponent usServiceComponent)

