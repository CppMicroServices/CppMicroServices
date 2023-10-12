usFunctionCreateDSTestBundle(TestBundleDSa)

usFunctionCreateTestBundleWithResources(TestBundleDSa USES_DS
  SOURCES src/ServiceImpl.cpp ${_glue_file}
  RESOURCES manifest.json
  BUNDLE_SYMBOLIC_NAME TestBundleDSa
  OTHER_LIBRARIES usTestInterfaces usServiceComponent)
