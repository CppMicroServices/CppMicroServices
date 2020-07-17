usFunctionCreateDSTestBundle(TestBundleDSDROU)

usFunctionCreateTestBundleWithResources(TestBundleDSDROU
  SOURCES src/ServiceComponentDynamicReluctantOptionalUnary.cpp ${_glue_file}
  RESOURCES manifest.json
  BUNDLE_SYMBOLIC_NAME TestBundleDSDROU
  OTHER_LIBRARIES usTestInterfaces usServiceComponent usServiceComponent)

