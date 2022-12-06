usFunctionCreateDSTestBundle(TestBundleDSCA01)

usFunctionCreateTestBundleWithResources(TestBundleDSCA01
  SOURCES src/ServiceImpl.cpp ${_glue_file}
  RESOURCES manifest.json
  BUNDLE_SYMBOLIC_NAME TestBundleDSCA01
  OTHER_LIBRARIES usTestInterfaces usServiceComponent)

