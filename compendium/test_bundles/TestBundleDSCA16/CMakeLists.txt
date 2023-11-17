usFunctionCreateDSTestBundle(TestBundleDSCA16)

usFunctionCreateTestBundleWithResources(TestBundleDSCA16
  SOURCES src/ServiceImpl.cpp ${_glue_file}
  RESOURCES manifest.json
  BUNDLE_SYMBOLIC_NAME TestBundleDSCA16
  OTHER_LIBRARIES usTestInterfaces usServiceComponent)

