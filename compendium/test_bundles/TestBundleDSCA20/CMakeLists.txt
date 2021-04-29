usFunctionCreateDSTestBundle(TestBundleDSCA20)

usFunctionCreateTestBundleWithResources(TestBundleDSCA20
  SOURCES src/ServiceImpl.cpp ${_glue_file}
  RESOURCES manifest.json
  BUNDLE_SYMBOLIC_NAME TestBundleDSCA20
  OTHER_LIBRARIES usTestInterfaces usServiceComponent)

