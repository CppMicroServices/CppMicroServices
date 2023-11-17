usFunctionCreateDSTestBundle(TestBundleDSCA05a)

usFunctionCreateTestBundleWithResources(TestBundleDSCA05a
  SOURCES src/ServiceImpl.cpp ${_glue_file}
  RESOURCES manifest.json
  BUNDLE_SYMBOLIC_NAME TestBundleDSCA05a
  OTHER_LIBRARIES usTestInterfaces usServiceComponent usServiceComponent)

