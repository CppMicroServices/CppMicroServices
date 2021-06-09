usFunctionCreateDSTestBundle(TestBundleDSDependent)

usFunctionCreateTestBundleWithResources(TestBundleDSDependent
  SOURCES src/ServiceImpl.cpp ${_glue_file}
  RESOURCES manifest.json
  BUNDLE_SYMBOLIC_NAME TestBundleDSDependent
  OTHER_LIBRARIES usTestInterfaces usServiceComponent)
