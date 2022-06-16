usFunctionCreateDSTestBundle(TestBundleDSCA02)

usFunctionCreateTestBundleWithResources(TestBundleDSCA02
  SOURCES src/ServiceImpl.cpp ${_glue_file}
  RESOURCES manifest.json
  BUNDLE_SYMBOLIC_NAME TestBundleDSCA02
  OTHER_LIBRARIES usTestInterfaces usServiceComponent)

