usFunctionCreateDSTestBundle(TestBundleDSCA30)

usFunctionCreateTestBundleWithResources(TestBundleDSCA30
  SOURCES src/ServiceImpl.cpp ${_glue_file}
  RESOURCES manifest.json
  BUNDLE_SYMBOLIC_NAME TestBundleDSCA30
  OTHER_LIBRARIES usTestInterfaces usServiceComponent)
