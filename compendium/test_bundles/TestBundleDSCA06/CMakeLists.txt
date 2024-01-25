usFunctionCreateDSTestBundle(TestBundleDSCA06)

usFunctionCreateTestBundleWithResources(TestBundleDSCA06
  SOURCES src/ServiceImpl.cpp ${_glue_file}
  RESOURCES manifest.json
  BUNDLE_SYMBOLIC_NAME TestBundleDSCA06
  OTHER_LIBRARIES usTestInterfaces usServiceComponent cm)

