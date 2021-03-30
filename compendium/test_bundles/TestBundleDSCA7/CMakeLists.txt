usFunctionCreateDSTestBundle(TestBundleDSCA7)

usFunctionCreateTestBundleWithResources(TestBundleDSCA7
  SOURCES src/ServiceImpl.cpp ${_glue_file}
  RESOURCES manifest.json
  BUNDLE_SYMBOLIC_NAME TestBundleDSCA7
  OTHER_LIBRARIES usTestInterfaces usServiceComponent usServiceComponent)

