usFunctionCreateDSTestBundle(TestBundleDSCA07a)

usFunctionCreateTestBundleWithResources(TestBundleDSCA07a
  SOURCES src/ServiceImpl.cpp ${_glue_file}
  RESOURCES manifest.json
  BUNDLE_SYMBOLIC_NAME TestBundleDSCA07a
  OTHER_LIBRARIES usTestInterfaces usServiceComponent usServiceComponent)

