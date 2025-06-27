usFunctionCreateDSTestBundle(TestBundleDSCA20_5)

usFunctionCreateTestBundleWithResources(TestBundleDSCA20_5
  SOURCES src/ServiceImpl.cpp ${_glue_file}
  RESOURCES manifest.json
  BUNDLE_SYMBOLIC_NAME TestBundleDSCA20_5
  OTHER_LIBRARIES usTestInterfaces usServiceComponent)

