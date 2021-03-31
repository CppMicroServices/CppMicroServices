usFunctionCreateDSTestBundle(TestBundleDSCA1)

usFunctionCreateTestBundleWithResources(TestBundleDSCA1
  SOURCES src/ServiceImpl.cpp ${_glue_file}
  RESOURCES manifest.json
  BUNDLE_SYMBOLIC_NAME TestBundleDSCA1
  OTHER_LIBRARIES usTestInterfaces usServiceComponent)

