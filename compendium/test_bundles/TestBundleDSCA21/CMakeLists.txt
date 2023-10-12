usFunctionCreateDSTestBundle(TestBundleDSCA21)

usFunctionCreateTestBundleWithResources(TestBundleDSCA21 USES_DS
  SOURCES src/ServiceImpl.cpp ${_glue_file}
  RESOURCES manifest.json
  BUNDLE_SYMBOLIC_NAME TestBundleDSCA21
  OTHER_LIBRARIES usTestInterfaces usServiceComponent)

