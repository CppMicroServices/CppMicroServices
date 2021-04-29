usFunctionCreateDSTestBundle(TestBundleDSCA14)

usFunctionCreateTestBundleWithResources(TestBundleDSCA14
  SOURCES src/ServiceImpl.cpp ${_glue_file}
  RESOURCES manifest.json
  BUNDLE_SYMBOLIC_NAME TestBundleDSCA14
  OTHER_LIBRARIES usTestInterfaces usServiceComponent usServiceComponent)

