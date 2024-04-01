usFunctionCreateDSTestBundle(TestBundleDSFAC1)

usFunctionCreateTestBundleWithResources(TestBundleDSFAC1
  SOURCES src/ServiceImpl.cpp ${_glue_file}
  RESOURCES manifest.json
  BUNDLE_SYMBOLIC_NAME TestBundleDSFAC1
  OTHER_LIBRARIES usTestInterfaces usServiceComponent)

