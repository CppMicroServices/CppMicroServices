usFunctionCreateDSTestBundle(TestBundleDSCA27)

usFunctionCreateTestBundleWithResources(TestBundleDSCA27 USES_DS
  SOURCES src/ServiceImpl.cpp ${_glue_file}
  RESOURCES manifest.json
  BUNDLE_SYMBOLIC_NAME TestBundleDSCA27
  OTHER_LIBRARIES usTestInterfaces usServiceComponent)

