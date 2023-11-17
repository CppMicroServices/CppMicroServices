usFunctionCreateDSTestBundle(TestBundleDSCA24)

usFunctionCreateTestBundleWithResources(TestBundleDSCA24
  SOURCES src/ServiceImpl.cpp ${_glue_file}
  RESOURCES manifest.json
  BUNDLE_SYMBOLIC_NAME TestBundleDSCA24
  OTHER_LIBRARIES usTestInterfaces usServiceComponent)

