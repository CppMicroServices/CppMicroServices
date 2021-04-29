usFunctionCreateDSTestBundle(TestBundleDSCA15)

usFunctionCreateTestBundleWithResources(TestBundleDSCA15
  SOURCES src/ServiceImpl.cpp ${_glue_file}
  RESOURCES manifest.json
  BUNDLE_SYMBOLIC_NAME TestBundleDSCA15
  OTHER_LIBRARIES usTestInterfaces usServiceComponent usServiceComponent)

