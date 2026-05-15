usFunctionCreateDSTestBundle(TestBundleDSCA29)

usFunctionCreateTestBundleWithResources(TestBundleDSCA29
  SOURCES src/ServiceImpl.cpp ${_glue_file}
  RESOURCES manifest.json
  BUNDLE_SYMBOLIC_NAME TestBundleDSCA29
  OTHER_LIBRARIES usTestInterfaces usServiceComponent)
