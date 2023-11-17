usFunctionCreateDSTestBundle(TestBundleDSCA26)

usFunctionCreateTestBundleWithResources(TestBundleDSCA26
  SOURCES src/ServiceImpl.cpp ${_glue_file}
  RESOURCES manifest.json
  BUNDLE_SYMBOLIC_NAME TestBundleDSCA26
  OTHER_LIBRARIES usTestInterfaces usServiceComponent)

