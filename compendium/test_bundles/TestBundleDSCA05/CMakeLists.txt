usFunctionCreateDSTestBundle(TestBundleDSCA05)

usFunctionCreateTestBundleWithResources(TestBundleDSCA05
  SOURCES src/ServiceImpl.cpp ${_glue_file}
  RESOURCES manifest.json
  BUNDLE_SYMBOLIC_NAME TestBundleDSCA05
  OTHER_LIBRARIES usTestInterfaces usServiceComponent usServiceComponent)

