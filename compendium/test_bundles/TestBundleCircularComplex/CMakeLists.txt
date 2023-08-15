usFunctionCreateDSTestBundle(TestBundleCircularComplex)

usFunctionCreateTestBundleWithResources(TestBundleCircularComplex
  SOURCES src/ServiceImpl.cpp ${_glue_file}
  RESOURCES manifest.json
  BUNDLE_SYMBOLIC_NAME TestBundleCircularComplex
  OTHER_LIBRARIES usTestInterfaces usServiceComponent)

