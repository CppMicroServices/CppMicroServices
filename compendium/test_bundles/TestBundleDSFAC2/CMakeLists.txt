usFunctionCreateDSTestBundle(TestBundleDSFAC2)

usFunctionCreateTestBundleWithResources(TestBundleDSFAC2
  SOURCES src/ServiceImpl.cpp ${_glue_file}
  RESOURCES manifest.json
  BUNDLE_SYMBOLIC_NAME TestBundleDSFAC2
  OTHER_LIBRARIES usTestInterfaces usServiceComponent)

