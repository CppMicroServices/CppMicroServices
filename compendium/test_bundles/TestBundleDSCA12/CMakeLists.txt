usFunctionCreateDSTestBundle(TestBundleDSCA12)

usFunctionCreateTestBundleWithResources(TestBundleDSCA12
  SOURCES src/ServiceImpl.cpp ${_glue_file}
  RESOURCES manifest.json
  BUNDLE_SYMBOLIC_NAME TestBundleDSCA12
  OTHER_LIBRARIES usTestInterfaces usServiceComponent usServiceComponent)

