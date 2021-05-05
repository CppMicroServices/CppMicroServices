usFunctionCreateDSTestBundle(TestBundleDSCA13)

usFunctionCreateTestBundleWithResources(TestBundleDSCA13
  SOURCES src/ServiceImpl.cpp ${_glue_file}
  RESOURCES manifest.json
  BUNDLE_SYMBOLIC_NAME TestBundleDSCA13
  OTHER_LIBRARIES usTestInterfaces usServiceComponent usServiceComponent)

