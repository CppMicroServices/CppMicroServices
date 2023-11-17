usFunctionCreateDSTestBundle(TestBundleDSCA03)

usFunctionCreateTestBundleWithResources(TestBundleDSCA03
  SOURCES src/ServiceImpl.cpp ${_glue_file}
  RESOURCES manifest.json
  BUNDLE_SYMBOLIC_NAME TestBundleDSCA03
  OTHER_LIBRARIES usTestInterfaces usServiceComponent)

