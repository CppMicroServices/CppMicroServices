usFunctionCreateDSTestBundle(TestBundleDSCA27)

usFunctionCreateTestBundleWithResources(TestBundleDSCA27
  SOURCES src/ServiceImpl.cpp ${_glue_file}
  RESOURCES manifest.json
  BUNDLE_SYMBOLIC_NAME TestBundleDSCA27
  OTHER_LIBRARIES usTestInterfaces usServiceComponent)

