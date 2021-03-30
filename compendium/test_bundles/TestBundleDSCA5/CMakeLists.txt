usFunctionCreateDSTestBundle(TestBundleDSCA5)

usFunctionCreateTestBundleWithResources(TestBundleDSCA5
  SOURCES src/ServiceImpl.cpp ${_glue_file}
  RESOURCES manifest.json
  BUNDLE_SYMBOLIC_NAME TestBundleDSCA5
  OTHER_LIBRARIES usTestInterfaces usServiceComponent usServiceComponent)

