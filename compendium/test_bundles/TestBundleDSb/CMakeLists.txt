usFunctionCreateDSTestBundle(TestBundleDSb)

usFunctionCreateTestBundleWithResources(TestBundleDSb
  SOURCES src/ServiceImpl.cpp ${_glue_file}
  RESOURCES manifest.json
  BUNDLE_SYMBOLIC_NAME TestBundleDSb
  OTHER_LIBRARIES usTestInterfaces usServiceComponent)
