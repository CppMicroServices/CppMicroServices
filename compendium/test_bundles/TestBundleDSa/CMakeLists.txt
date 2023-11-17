usFunctionCreateDSTestBundle(TestBundleDSa)

usFunctionCreateTestBundleWithResources(TestBundleDSa
  SOURCES src/ServiceImpl.cpp ${_glue_file}
  RESOURCES manifest.json
  BUNDLE_SYMBOLIC_NAME TestBundleDSa
  OTHER_LIBRARIES usTestInterfaces usServiceComponent)
