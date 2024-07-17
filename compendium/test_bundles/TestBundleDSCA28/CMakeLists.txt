usFunctionCreateDSTestBundle(TestBundleDSCA28)

usFunctionCreateTestBundleWithResources(TestBundleDSCA28
  SOURCES src/ServiceImpl.cpp ${_glue_file}
  RESOURCES manifest.json
  BUNDLE_SYMBOLIC_NAME TestBundleDSCA28
  OTHER_LIBRARIES usTestInterfaces usServiceComponent)

