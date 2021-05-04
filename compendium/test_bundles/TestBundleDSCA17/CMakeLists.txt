usFunctionCreateDSTestBundle(TestBundleDSCA17)

usFunctionCreateTestBundleWithResources(TestBundleDSCA17
  SOURCES src/ServiceImpl.cpp ${_glue_file}
  RESOURCES manifest.json
  BUNDLE_SYMBOLIC_NAME TestBundleDSCA17
  OTHER_LIBRARIES usTestInterfaces usServiceComponent)

