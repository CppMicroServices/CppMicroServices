usFunctionCreateDSTestBundle(TestBundleDSCA20)

usFunctionCreateTestBundleWithResources(TestBundleDSCA20 USES_DS
  SOURCES src/ServiceImpl.cpp ${_glue_file}
  RESOURCES manifest.json
  BUNDLE_SYMBOLIC_NAME TestBundleDSCA20
  OTHER_LIBRARIES usTestInterfaces usServiceComponent)

