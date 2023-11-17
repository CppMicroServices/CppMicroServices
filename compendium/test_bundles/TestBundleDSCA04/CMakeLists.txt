usFunctionCreateDSTestBundle(TestBundleDSCA04)

usFunctionCreateTestBundleWithResources(TestBundleDSCA04
  SOURCES src/ServiceImpl.cpp ${_glue_file}
  RESOURCES manifest.json
  BUNDLE_SYMBOLIC_NAME TestBundleDSCA04
  OTHER_LIBRARIES usTestInterfaces usServiceComponent)

