usFunctionCreateDSTestBundle(TestBundleDSCA10)

usFunctionCreateTestBundleWithResources(TestBundleDSCA10
  SOURCES src/ServiceImpl.cpp ${_glue_file}
  RESOURCES manifest.json
  BUNDLE_SYMBOLIC_NAME TestBundleDSCA10
  OTHER_LIBRARIES usTestInterfaces usServiceComponent cm)

